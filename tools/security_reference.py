#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Build/verify the exact independent reference without Git worktrees/ref changes.
No destructive cache replacement. Every reuse verifies source, tools and outputs.
"""
import argparse
import io
import json
import os
from pathlib import Path
import subprocess
import sys
import tarfile
import hashlib
from security_baseline import ROOT, NAME, COMMIT, TREE, TAG, LOADER, SecurityError, git, sha, load, compiled_boot_guard, unique

INPUTS = 'c9926adb5470fd653f0212596c4f2bbc28540cb60a3bafc73c0a9b5a94bc7160'
REFERENCE_APP = '31f6c94ed3e1645e8ec321e79460f679d76ac511ae5b1a15b42b4474449ca4b2'
# Seven outputs match two fresh, separate exact-commit builds. A rewritten
# receipt cannot authorize a different ELF/listing while retaining the app hash.
OUTPUT_DIGEST = 'b01c312d9906257842205fc13a20c82ba5a662207c86b3c826fd0f17317edf65'
OUTPUTS = ['felucca.bin','felucca.elf','felucca.dis','text.bin','data.bin','ramtext.bin','loader/ota.bin']


def inputs():
    d = json.loads((ROOT/'tools/security_build_inputs.json').read_text(),object_pairs_hook=unique)
    if sha(json.dumps(d,sort_keys=True,separators=(',',':')).encode()) != INPUTS:
        raise SecurityError('reference build input pins changed')
    tc = Path(os.environ.get('JIELI_TOOLCHAIN',str(Path.home()/'.jieli/toolchain')))
    sdk = Path(os.environ.get('AC79_SDK',str(Path.home()/'fw-AC79_AIoT_SDK')))
    for rel,want in d['tools'].items():
        if sha((tc/rel).read_bytes()) != want:
            raise SecurityError('toolchain changed: '+rel)
    for rel,want in d['sdk'].items():
        if sha((sdk/'cpu/wl82/tools'/rel).read_bytes()) != want:
            raise SecurityError('SDK input changed: '+rel)
    return d


def source(path):
    expected = {}
    for line in git(ROOT,'ls-tree','-rz',COMMIT).split(b'\0'):
        if not line: continue
        fields,name = line.split(b'\t');mode,typ,blob = fields.decode().split()
        name = name.decode();p = path/name
        if typ != 'blob' or not p.is_file() or p.is_symlink():
            raise SecurityError('reference source missing/type changed: '+name)
        data=p.read_bytes();actual=hashlib.sha1(b'blob '+str(len(data)).encode()+b'\0'+data).hexdigest()
        if actual != blob or ('100755' if p.stat().st_mode & 0o111 else '100644') != mode:
            raise SecurityError('reference source changed: '+name)
        expected[name]=sha(data)
    for p in path.rglob('*'):
        name=p.relative_to(path).as_posix()
        if name.startswith('build/') or p.is_dir(): continue
        if name not in expected: raise SecurityError('unexpected reference source: '+name)
    return expected


def validate(path):
    load();inp=inputs();src=source(path)
    r=json.loads((path/'build/security-receipt.json').read_text(),object_pairs_hook=unique)
    if (r.get('name'),r.get('commit'),r.get('tree'),r.get('tag')) != (NAME,COMMIT,TREE,TAG) or r.get('inputs') != inp or r.get('sources') != src:
        raise SecurityError('reference receipt mismatch')
    outputs={n:sha((path/'build'/n).read_bytes()) for n in OUTPUTS}
    if (r.get('outputs') != outputs or outputs['loader/ota.bin'] != LOADER or outputs['felucca.bin'] != REFERENCE_APP
            or sha(json.dumps(outputs,sort_keys=True,separators=(',',':')).encode()) != OUTPUT_DIGEST):
        raise SecurityError('reference artifact hash mismatch')
    compiled_boot_guard(path/'build')
    print('security reference: exact source/tools/output receipt verified: '+str(path))


def prepare(path):
    load();inp=inputs()
    if path.exists():
        validate(path);return
    path.mkdir(parents=True)
    with tarfile.open(fileobj=io.BytesIO(git(ROOT,'archive',COMMIT))) as t:
        t.extractall(path,filter='data')
    src=source(path)
    env={k:v for k,v in os.environ.items() if not k.startswith(('FELUCCA_','DRUM_','BLE_')) and k not in ('FROZEN_BASE_FILE','BUDGET_UPDATE','GOLDEN_UPDATE')}
    env.update(PYTHONDONTWRITEBYTECODE='1',GIT_OPTIONAL_LOCKS='0')
    with (ROOT/'build/security-reference-build.log').open('wb') as log:
        subprocess.run(['./build.sh'],cwd=path,env=env,stdout=log,stderr=subprocess.STDOUT,check=True)
    r=dict(name=NAME,commit=COMMIT,tree=TREE,tag=TAG,inputs=inp,sources=src,
           outputs={n:sha((path/'build'/n).read_bytes()) for n in OUTPUTS})
    (path/'build/security-receipt.json').write_text(json.dumps(r,indent=2)+'\n')
    validate(path)


def main():
    ap=argparse.ArgumentParser();ap.add_argument('--verify',action='store_true');ap.add_argument('path',nargs='?',type=Path,default=ROOT/'build/upstream');a=ap.parse_args()
    try:
        (validate if a.verify else prepare)(a.path.resolve())
    except (SecurityError,OSError,ValueError,subprocess.CalledProcessError) as e:
        print('security reference: FAIL '+str(e),file=sys.stderr);return 1
    return 0


if __name__ == '__main__': sys.exit(main())
