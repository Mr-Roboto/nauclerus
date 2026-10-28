#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
# Drum machine fork: 2026 DEADACTIVE
"""Stage the Nauclerus static portal without releasing or installing firmware.

Call portal(build/sim, destination) after tools/build_sim.sh. The legacy
main(package, version, destination, optional_ble) signature is retained for the
firmware builder, but it only stages the independently built portal. No package
or installer is copied. Missing or mismatched simulator inputs fail closed.
"""
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
LIB = ("fm1pkg.js", "fm1ota.js", "sim/counter.js")   # inlined into the installer (sim/counter.js: the install count)
BLOCKS, BLK, KEEP = 20, 0x30, 0x2F


def strip_module(src):
    src = re.sub(r"^export\s+", "", src, flags=re.M)
    return re.sub(r"^import .*?;\n", "", src, flags=re.M)


def editor_html():
    """editor.html with its visits counted: sim/counter.js inlined before </body>, count.js added only on the site"""
    html = (HERE / "editor.html").read_text(encoding="utf-8")
    if html.count("</body>") != 1:
        raise SystemExit("editor.html must contain </body> once; update make_site.py")
    counter = strip_module((HERE / "sim" / "counter.js").read_text(encoding="utf-8"))
    return html.replace("</body>", f'<script type="module">\n{counter}\nloadPageCounter();\n</script>\n</body>')


def product_of(raw):
    """the package identity: one marker byte after each of the first 20 blocks (fm1pkg.js productOf)"""
    return "".join(chr((m - i - 1) & 0xFF) for i in range(BLOCKS) if (m := raw[i * BLK + KEEP]) != 0x7D)


def checked(pkg):
    """the package's bytes and identity, or a stop: only fm1pkg_make.py packages of a Felucca identity"""
    raw = Path(pkg).read_bytes()
    product = product_of(raw)
    sys.path.insert(0, str(HERE.parent/"tools"))
    import version
    if product != version.PRODUCT:
        raise SystemExit(f"{pkg}: identity {product!r} is not the approved Nauclerus package identity")
    if b"FELUCCA-LOADER-1" not in raw:              # Felucca loader marker: never publish a package with vendor files
        raise SystemExit(f"{pkg}: no Felucca loader in it; the site ships only fm1pkg_make.py packages "
                         "(a package patched from an official one carries vendor files)")
    return raw, product


def portal(built, dest):
    """Stage only the source-matched Nauclerus portal. No installer/device package."""
    import hashlib
    built, dest = Path(built), Path(dest)
    meta=json.loads((built/'version.json').read_text())
    if meta.get('project')!='Nauclerus' or meta.get('installer_enabled') is not False:
        raise ValueError('not an installation-disabled Nauclerus portal')
    for key,name in [('wasm_sha256','fm1sim.wasm'),('source_archive_sha256','source.tar.gz')]:
        if hashlib.sha256((built/name).read_bytes()).hexdigest()!=meta[key]:
            raise ValueError('stale/mismatched '+name)
    # Refuse stale staging rather than leaving an old firmware/installer behind.
    dest.mkdir(parents=True,exist_ok=True)
    forbidden=[dest/'firmware',dest/'webapp',*dest.glob('*.fwsc')]
    if any(p.exists() for p in forbidden):raise ValueError('destination contains an old installer/package; choose an empty staging directory')
    files=[]
    for p in built.iterdir():
        if p.is_file() and (p.suffix in {'.html','.css','.js','.json','.wasm','.woff','.txt','.md','.gz'} or p.name in {'LICENSE','.nojekyll'}):
            shutil.copy2(p,dest/p.name);files.append(p.name)
    for name in ['LICENSES','docs']:
        shutil.copytree(built/name,dest/name,dirs_exist_ok=True)
    (dest/'deployment.json').write_text(json.dumps({'target':'https://mr-roboto.github.io/nauclerus/','branch':'gh-pages','published':False,'installer_enabled':False,'source_digest':meta['source_digest'],'files':files},indent=2)+'\n')
    return dest

def install_site(pkg, version, out):
    """Stage one verified Nauclerus package. Browser rechecks hash before enabling Install."""
    import hashlib
    pkg,out=Path(pkg),Path(out)
    from importlib.util import spec_from_file_location,module_from_spec
    spec=spec_from_file_location('candidate_integrity',HERE.parent/'tools/check_package_integrity.py')
    ci=module_from_spec(spec);spec.loader.exec_module(ci)
    logical,product=ci.unpack(pkg.read_bytes())
    ci.validate(logical,(pkg.parent/'felucca.bin').read_bytes(),(pkg.parent/'loader/ota.bin').read_bytes())
    name='nauclerus-'+(HERE.parent/'VERSION.txt').read_text().strip()+'.fwsc'
    if pkg.name!=name:raise ValueError('not the exact current Nauclerus artifact')
    raw=pkg.read_bytes()
    info={'version':version,'product':product,'pkg':'../../firmware/'+name,'bytes':len(raw),'sha256':hashlib.sha256(raw).hexdigest()}
    target=out/'webapp/installer';target.mkdir(parents=True,exist_ok=True)
    fw=out/'firmware';fw.mkdir(exist_ok=True)
    if any(p.name!=name for p in fw.glob('*.fwsc')):raise ValueError('stale or unrelated package in staging')
    shutil.copy2(pkg,fw/name)
    html=(HERE/'index_nauclerus.html').read_text()
    libs='\n'.join(strip_module((HERE/f).read_text()) for f in LIB)
    html=html.replace('/*LIB*/',libs).replace('/*META*/',json.dumps(info))
    (target/'index.html').write_text(html)
    (out/'firmware.json').write_text(json.dumps(info,indent=2)+'\n')
    vpath=out/'version.json'
    if vpath.exists():
        v=json.loads(vpath.read_text());v['installer_enabled']=True;v['publication_status']='stable-release-prepared';v['firmware']=info
        vpath.write_text(json.dumps(v,indent=2)+'\n')
    return info


def main(pkg, version, out, ble_pkg=None):
    if ble_pkg is not None:raise ValueError('No unverified Bluetooth artifact may accompany this release')
    dest=portal(HERE.parent/'build/sim',Path(out))
    return install_site(pkg,version,dest)


if __name__ == "__main__":
    if len(sys.argv) not in (4,5):sys.exit(__doc__)
    main(*sys.argv[1:5])
