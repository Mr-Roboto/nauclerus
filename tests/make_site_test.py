#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Current Nauclerus staging: exact package and fail-closed corrupt/unknown artifacts."""
import json,sys,tempfile,shutil
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path[:0]=[str(ROOT/'web'),str(ROOT/'tools')]
import make_site,version
pkg=ROOT/'build'/('nauclerus-'+version.read()+'.fwsc')
with tempfile.TemporaryDirectory() as td:
 out=Path(td)/'site';out.mkdir()
 info=make_site.install_site(pkg,'nauclerus-'+version.read()+'+test',out)
 assert (out/'firmware'/pkg.name).read_bytes()==pkg.read_bytes()
 assert info['product']=='FM-1_25000' and len(info['sha256'])==64
 html=(out/'webapp/installer/index.html').read_text()
 assert 'SHA-256/size mismatch' in html and 'meta.sha256' in html and 'requestMIDIAccess' in html and 'id="ack"' in html
 shutil.copy2(ROOT/'build/felucca.bin',Path(td)/'felucca.bin')
 (Path(td)/'loader').mkdir();shutil.copy2(ROOT/'build/loader/ota.bin',Path(td)/'loader/ota.bin')
 # Mutate a byte inside the verified flash payload, retaining all adjacent inputs.
 raw=bytearray(pkg.read_bytes());raw[2048]^=1
 bad=Path(td)/pkg.name;bad.write_bytes(raw)
 try:make_site.install_site(bad,'bad',Path(td)/'bad')
 except ValueError:pass
 else:raise AssertionError('corrupted/unbound package accepted')
print('make_site: exact Nauclerus identity, package bytes, integrity/hash gating and corrupt artifact rejection PASS')
