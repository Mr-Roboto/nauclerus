#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Production reference cache rejects stale source, tools and binary receipts."""
import json
from pathlib import Path
import shutil
import sys
import tempfile
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import security_reference as r
from security_baseline import SecurityError, ROOT


def main():
    count=0
    with tempfile.TemporaryDirectory(prefix='nauclerus-reference-fixture-') as tmp:
        p=Path(tmp)/'ref';shutil.copytree(ROOT/'build/upstream',p)
        r.validate(p);count+=1
        def reject(label,run):
            nonlocal count
            try:run()
            except (SecurityError,OSError,ValueError):count+=1
            else:raise AssertionError(label)
        file=p/'firmware/src/ota.c';b=file.read_bytes();file.write_bytes(b+b'!')
        reject('source corruption',lambda:r.validate(p));file.write_bytes(b)
        file=p/'firmware/src/unexpected.c';file.write_bytes(b'')
        reject('source injection',lambda:r.validate(p));file.unlink()
        file=p/'build/felucca.dis';b=file.read_bytes();file.write_bytes(b+b'!')
        reject('artifact corruption',lambda:r.validate(p));file.write_bytes(b)
        file=p/'build/security-receipt.json';b=file.read_bytes();file.unlink()
        reject('missing receipt',lambda:r.validate(p));file.write_bytes(b)
        listing=p/'build/felucca.dis';saved=listing.read_bytes();listing.write_bytes(saved+b'!')
        d=json.loads(b);d['outputs']['felucca.dis']=r.sha(listing.read_bytes());file.write_text(json.dumps(d))
        reject('artifact and receipt jointly corrupted',lambda:r.validate(p));listing.write_bytes(saved);file.write_bytes(b)
        d=json.loads(b);d['inputs']['cflags']=['-O0'];file.write_text(json.dumps(d))
        reject('stale input receipt',lambda:r.validate(p));file.write_bytes(b)
        d=json.loads(b);d['commit']='0'*40;file.write_text(json.dumps(d))
        reject('wrong source receipt',lambda:r.validate(p));file.write_bytes(b)
    print(f'security reference: {count} production cache fixtures passed')


if __name__ == '__main__':main()
