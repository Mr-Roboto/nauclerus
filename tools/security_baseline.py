# SPDX-License-Identifier: GPL-3.0-only
"""Owner-approved, fail-closed local baseline. No publisher-signature claim.

Expectations are bound to independently reviewed committed blobs, not generated
from current source. The annotated tag and canonical manifest are pinned too.
"""
import hashlib
import json
import os
from pathlib import Path
import re
import stat
import subprocess

ROOT = Path(__file__).resolve().parents[1]
NAME = 'nauclerus-security-base-1'
COMMIT = '2690e74aba6fb99eeb547010e65e52cc2da395e9'
TREE = '3b0583347cc1e91e7870b0051fe4256e0066055d'
TAG = '07462232c82f044bb5a50cf46d16a9ed2a333260'
MANIFEST = 'b55fc9404167b9098314a2af508bbf40f2d36fc8fc5a7f82ad020f6bccfe8738'
LOADER = 'cc98eed224299fa42ca22a546073e0f8b98d8362e26b5f325a9aaeab287bdf5d'


class SecurityError(Exception):
    pass


def sha(data):
    return hashlib.sha256(data).hexdigest()


def git(root, *args):
    p = subprocess.run(['git','-C',str(root),*args],capture_output=True,
                       env=dict(os.environ,GIT_OPTIONAL_LOCKS='0',GIT_NO_LAZY_FETCH='1'))
    if p.returncode:
        raise SecurityError('missing/invalid Git reference or object: '+' '.join(args))
    return p.stdout


def unique(pairs):
    out = {}
    for k,v in pairs:
        if k in out:
            raise SecurityError('duplicate descriptor key: '+k)
        out[k] = v
    return out


def load(root=ROOT, pinfile=None, descriptor=None):
    root = Path(root)
    expected = {'FROZEN_BASE':[NAME,COMMIT], 'BASELINE_TAG_OBJECT':[TAG],
                'BASELINE_TREE':[TREE], 'BASELINE_MANIFEST_SHA256':[MANIFEST], 'LOADER_SHA256':[LOADER]}
    pins = {}
    try:
        for line in Path(pinfile or os.environ.get('FROZEN_BASE_FILE') or root/'tools/frozen_base.txt').read_text().splitlines():
            f = line.split()
            if not f or f[0].startswith('#'):
                continue
            if f[0] in pins or f[0] not in expected:
                raise SecurityError('duplicate/unknown baseline pin: '+f[0])
            pins[f[0]] = f[1:]
        if pins != expected:
            raise SecurityError('baseline pins missing, moved or changed')
        obj = git(root,'rev-parse','--verify','refs/tags/'+NAME).decode().strip()
        if obj != TAG or git(root,'cat-file','-t',obj).strip() != b'tag':
            raise SecurityError('baseline annotation moved or changed')
        if git(root,'rev-parse','refs/tags/'+NAME+'^{commit}').decode().strip() != COMMIT:
            raise SecurityError('baseline commit moved')
        if git(root,'rev-parse',COMMIT+'^{tree}').decode().strip() != TREE:
            raise SecurityError('baseline tree changed')
        annotation = git(root,'cat-file','tag',TAG).decode()
        if 'Protected manifest SHA-256: '+MANIFEST not in annotation or 'Loader SHA-256: '+LOADER not in annotation:
            raise SecurityError('annotation does not attest manifest/loader')
        d = json.loads(Path(descriptor or root/'tools/security_baseline.json').read_text(),object_pairs_hook=unique)
        keys = {'schema','status','name','commit','tree','loader_sha256','tag_object','approval','files','historical_missing','manifest_sha256'}
        if set(d) != keys or d['schema'] != 1 or d['status'] != 'ADOPTED_LOCAL_USER_AUTHORIZATION':
            raise SecurityError('unsupported/malformed baseline descriptor')
        if (d['name'],d['commit'],d['tree'],d['tag_object'],d['loader_sha256'],d['manifest_sha256']) != (NAME,COMMIT,TREE,TAG,LOADER,MANIFEST):
            raise SecurityError('descriptor identities changed')
        if not isinstance(d['approval'],str) or not d['approval']:
            raise SecurityError('missing approval record')
        if sha(json.dumps(d['files'],sort_keys=True,separators=(',',':')).encode()) != MANIFEST:
            raise SecurityError('protected manifest changed')
        for name,row in d['files'].items():
            mode,typ,blob = git(root,'ls-tree',COMMIT,'--',name).decode().split()[:3]
            data = git(root,'show',COMMIT+':'+name)
            if typ != 'blob' or (mode,blob,sha(data)) != (row['mode'],row['blob'],row['sha256']):
                raise SecurityError('baseline blob mismatch: '+name)
            if row.get('contract') == 'boot-tail' and not data.endswith(row['tail'].encode()):
                raise SecurityError('baseline boot contract mismatch')
        return d
    except (OSError,ValueError,KeyError,TypeError) as e:
        raise SecurityError('malformed/unavailable baseline evidence: '+str(e)) from e


# NAU-0250-015: exact owner-approved application product descriptor only.
# The independently pinned original blob, manifest and reference remain unchanged.
USB_APP_PRODUCT_OLD = b"#else\nstatic const uint8_t STR2[] = {18, 3, 'H', 0, 'o', 0, 'r', 0, 't', 0, 'a', 0, 't', 0, 'o', 0, 'r', 0};\n#endif\n"
USB_APP_PRODUCT_NEW = b"#else\nstatic const uint8_t STR2[] = {20, 3, 'N', 0, 'a', 0, 'u', 0, 'c', 0, 'l', 0, 'e', 0, 'r', 0, 'u', 0, 's', 0};\n#endif\n"

def approved_source_bytes(name, data):
    if name == 'firmware/src/usb_app.c' and data.count(USB_APP_PRODUCT_NEW) == 1 and USB_APP_PRODUCT_OLD not in data:
        return data.replace(USB_APP_PRODUCT_NEW, USB_APP_PRODUCT_OLD, 1)
    return data


def source_errors(tree=ROOT, root=ROOT, pinfile=None, descriptor=None):
    d = load(root,pinfile,descriptor); tree = Path(tree); bad = []
    for directory in ('firmware/hal','firmware/loader'):
        base = tree/directory
        if base.is_symlink():
            bad.append('symlink directory: '+directory)
        if base.exists():
            for p in base.rglob('*'):
                n = p.relative_to(tree).as_posix()
                if p.is_symlink() or (not p.is_dir() and n not in d['files']):
                    bad.append('unexpected protected entry: '+n)
    for name,row in d['files'].items():
        p = tree/name
        try:
            st = p.lstat()
            parts = Path(name).parts
            if not stat.S_ISREG(st.st_mode) or any((tree/Path(*parts[:i])).is_symlink() for i in range(1,len(parts))):
                bad.append('not a regular protected file: '+name); continue
            if ('100755' if st.st_mode & 0o111 else '100644') != row['mode']:
                bad.append('protected mode changed: '+name)
            b = p.read_bytes()
            if row.get('contract') == 'boot-tail':
                if not b.endswith(row['tail'].encode()) or b.count(b'#define BOOTGUARD_MAGIC') != 1 or b.count(b'} bootguard ') != 1:
                    bad.append('boot contract changed: '+name)
                pp = re.sub(rb'/\*.*?\*/',b' ',b.replace(b'\\\n',b''),flags=re.S)
                directives = re.findall(rb'^\s*#\s*(define|undef)\s+(BOOTGUARD_MAGIC|bootguard)\b',pp,re.M)
                if directives != [(b'define',b'BOOTGUARD_MAGIC')]:
                    bad.append('boot redefinition: '+name)
            elif sha(approved_source_bytes(name,b)) != row['sha256']:
                bad.append('protected bytes changed: '+name)
        except OSError:
            bad.append('protected file missing/unreadable: '+name)
    return bad


def compiled_boot_guard(build):
    from elf_syms import Elf, _sections
    import struct
    p = Path(build)/'felucca.elf'; e = Elf(p)
    symbols = [(a,n) for a,n,s in e.syms if s == 'bootguard']
    if len(symbols) != 1 or symbols[0][1] != 12:
        raise SecurityError('compiled bootguard must be one 12-byte object')
    b = p.read_bytes(); secs = _sections(b); st = secs[struct.unpack_from('<H',b,0x32)[0]]
    names = b[st[4]:st[4]+st[5]]; a,n = symbols[0]
    if not any(names[s[0]:].split(b'\0')[0] == b'.noinit' and s[3] <= a and a+n <= s[3]+s[5] for s in secs):
        raise SecurityError('compiled bootguard outside .noinit')
