#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Actual production guard against isolated filesystem/Git tampering fixtures.
No source, index, references or objects in the real repository are modified.
"""
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import security_baseline as s


def main():
    d = s.load(); count = 0
    def check(label, ok):
        nonlocal count
        if not ok:
            raise AssertionError(label)
        count += 1
    with tempfile.TemporaryDirectory(prefix='nauclerus-security-fixture-') as tmp:
        root = Path(tmp)
        shutil.copytree(s.ROOT/'firmware', root/'firmware')
        check('actual clean tree', not s.source_errors(root))
        for name,row in d['files'].items():
            p = root/name; body = p.read_bytes(); mode = p.stat().st_mode
            if row.get('contract'):
                p.write_bytes(body.replace(b'0x42475244u',b'0x42475245u'))
            else:
                p.write_bytes(body+b'!')
            check('modified '+name, bool(s.source_errors(root)))
            p.unlink()
            check('deleted '+name, bool(s.source_errors(root)))
            p.symlink_to(s.ROOT/name)
            check('symlink '+name, bool(s.source_errors(root)))
            p.unlink(); p.write_bytes(body); p.chmod(mode ^ 0o111)
            check('mode '+name, bool(s.source_errors(root)))
            p.chmod(mode)
        p = root/'firmware/hal/untracked.h';p.write_bytes(b'')
        check('unexpected untracked protected file', bool(s.source_errors(root)));p.unlink()
        p = root/'firmware/src/main.c';b = p.read_bytes()
        p.write_bytes(b.replace(b'fm1_enter_uboot();',b'fm1_reboot();'))
        check('boot/recovery tamper', bool(s.source_errors(root)));p.write_bytes(b)
        p = root/'firmware/src/storage.c';b = p.read_bytes()
        p.write_bytes(b.replace(b'0xD4000u',b'0xD5000u'))
        check('unauthorized storage addresses', bool(s.source_errors(root)));p.write_bytes(b)
        p = root/'firmware/src/seq.c';p.write_bytes(p.read_bytes()+b'\n/* ordinary application */\n')
        check('ordinary application change allowed', not s.source_errors(root))
        p = root/'firmware/src/core.h';b = p.read_bytes()
        for label, altered in [('tail moved', b+b'\n'),('duplicate', b'#define BOOTGUARD_MAGIC 1\n'+b),
                               ('spaced duplicate',b'# /**/ define BOOTGUARD_MAGIC 1\n'+b),
                               ('continued duplicate',b'#def\\\nine BOOTGUARD_MAGIC 1\n'+b),
                               ('undef',b'#undef BOOTGUARD_MAGIC\n'+b)]:
            p.write_bytes(altered);check(label,bool(s.source_errors(root)))
        p.write_bytes(b'/* ordinary types comment */\n'+b)
        check('ordinary core body allowed', not s.source_errors(root));p.write_bytes(b)
        # The approved product name is the only USB source exception.
        p=root/'firmware/src/usb_app.c';body=p.read_bytes()
        check('approved Nauclerus product exactly once',body.count(s.USB_APP_PRODUCT_NEW)==1 and not s.source_errors(root))
        for label,altered in [
            ('wrong descriptor length',body.replace(b'{20, 3,',b'{22, 3,',1)),
            ('different app product',body.replace(b"'N', 0, 'a', 0",b"'X', 0, 'a', 0",1)),
            ('USB PID',body.replace(b'#define FELUCCA_USB_PID 0x0001',b'#define FELUCCA_USB_PID 0x0003')),
            ('loader USB name',body.replace(b"{30, 3, 'F', 0",b"{30, 3, 'X', 0",1)),
            ('extra USB change',body+b'\n/* extra */\n'),
            ('duplicate app descriptor',body+s.USB_APP_PRODUCT_NEW),
        ]:
            p.write_bytes(altered);check(label,bool(s.source_errors(root)))
        p.write_bytes(body)
        # Real isolated Git repository; read-only object alternates, only dedicated fixture tag.
        repo = root/'git-fixture'
        subprocess.run(['git','init','-q','--initial-branch=fixture',str(repo)],check=True)
        (repo/'.git/objects/info/alternates').write_text(subprocess.check_output(['git','rev-parse','--path-format=absolute','--git-path','objects'],cwd=s.ROOT,text=True).strip()+'\n')
        refs = repo/'.git/refs/tags';refs.mkdir(exist_ok=True)
        ref = refs/s.NAME;ref.write_text(s.TAG+'\n')
        tools = repo/'tools';tools.mkdir()
        pin = tools/'frozen_base.txt';pin.write_bytes((s.ROOT/'tools/frozen_base.txt').read_bytes())
        desc = tools/'security_baseline.json';desc.write_bytes((s.ROOT/'tools/security_baseline.json').read_bytes())
        check('isolated approved reference', bool(s.load(repo)))
        def rejected(label, run):
            try:
                run()
            except s.SecurityError:
                check(label,True)
            else:
                check(label,False)
        ref.unlink();rejected('missing reference',lambda:s.load(repo))
        ref.write_text(s.COMMIT+'\n');rejected('moved to lightweight reference',lambda:s.load(repo))
        ref.write_text(s.git(s.ROOT,'rev-parse',s.COMMIT+'^').decode())
        rejected('moved commit',lambda:s.load(repo))
        # A different annotated tag on the same commit must also fail.
        annotation=s.git(s.ROOT,'cat-file','tag',s.TAG)+b'changed annotation\n'
        obj=subprocess.check_output(['git','-C',str(repo),'hash-object','-w','-t','tag','--stdin'],input=annotation).decode().strip()
        ref.write_text(obj+'\n');rejected('same commit different tag object',lambda:s.load(repo))
        ref.write_text(s.TAG+'\n')
        original=pin.read_text()
        for label,text in [('wrong pin',original.replace(s.COMMIT,'0'*40)),
                           ('duplicate pin',original+'LOADER_SHA256 '+s.LOADER+'\n'),
                           ('missing pin','\n'.join(l for l in original.splitlines() if not l.startswith('BASELINE_TREE ')))]:
            pin.write_text(text);rejected(label,lambda:s.load(repo))
        pin.write_text(original)
        original=desc.read_text()
        for label,alter in [('schema',lambda x:x.update(schema=2)),('hash',lambda x:x['files']['firmware/app.ld'].update(sha256='0'*64)),
                            ('loader',lambda x:x.update(loader_sha256='0'*64))]:
            x=json.loads(original);alter(x);desc.write_text(json.dumps(x));rejected(label,lambda:s.load(repo))
        desc.write_text(original.replace('"schema": 1','"schema": 1, "schema": 1'))
        rejected('duplicate JSON key',lambda:s.load(repo));desc.write_text(original)
        (repo/'.git/objects/info/alternates').unlink()
        rejected('missing object',lambda:s.load(repo))
    print(f'security baseline: {count} production guard fixtures passed (including missing/moved real Git refs)')


if __name__ == '__main__':
    main()
