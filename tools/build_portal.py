#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Build the existing firmware simulator from an exact dirty-source snapshot.
Native emcc; no installation, device packaging or publication.
"""
import version as project_identity
import argparse, gzip, hashlib, io, json, os, re, shutil, subprocess, tarfile
from pathlib import Path
from datetime import datetime, timezone
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'build/sim'
EXCLUDE={'docs/panel.jpg', 'docs/superpowers/specs/2026-10-19-md-ui/'+ 'home.png', 'docs/superpowers/specs/2026-10-19-md-ui/layout-a-vs-b.png','docs/superpowers/specs/2026-10-19-md-ui/screens.png','docs/superpowers/specs/2026-10-19-md-ui/themes.png'}
def run(args, cwd=ROOT, **kw):
    print('+', ' '.join(map(str,args)),flush=True)
    subprocess.run(list(map(str,args)),cwd=cwd,check=True,**kw)
def snapshot():
    files=set()
    for args in [('ls-files','-z'),('ls-files','--others','--exclude-standard','-z')]:
        files.update(subprocess.check_output(['git',*args],cwd=ROOT).decode().split('\0'))
    files.discard('');files-=EXCLUDE
    src=OUT/'source';src.mkdir(parents=True,exist_ok=True)
    manifest=[]
    for name in sorted(files):
        p=ROOT/name
        if not p.is_file() or p.is_symlink():raise SystemExit('unsupported source input: '+name)
        if name.startswith(('build/','.git/','.aws/','.codex/','.agents/')):raise SystemExit('private/build input: '+name)
        q=src/name;q.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,q)
        manifest.append({'path':name,'sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'bytes':p.stat().st_size})
    digest=hashlib.sha256(json.dumps(manifest,sort_keys=True).encode()).hexdigest()
    with (OUT/'source.tar.gz').open('wb') as f,gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0) as z,tarfile.open(fileobj=z,mode='w') as tar:
        for row in manifest:
            p=src/row['path'];info=tarfile.TarInfo('nauclerus-source/'+row['path']);data=p.read_bytes();info.size=len(data);info.mode=p.stat().st_mode&0o777;info.mtime=0;tar.addfile(info,io.BytesIO(data))
    (OUT/'source-manifest.json').write_text(json.dumps({'source_digest':digest,'files':manifest,'excluded_optional_images':sorted(EXCLUDE)},indent=2)+'\n')
    return src,digest

def manual(src):
    from markdown_it import MarkdownIt
    md=MarkdownIt('commonmark',{'html':False}).enable('table')
    text=(src/'docs/USER_GUIDE.md').read_text()
    # A source reference becomes a real local downloadable document, never a stale relative URL.
    text=re.sub(r'\]\((?:\.\./)?([^):]+\.md)(#[^)]*)?\)',lambda m:'](docs/'+Path(m[1]).name+(m[2] or '')+')',text)
    topics=[]
    for section in re.split(r'(?m)^## ',text)[1:]:
        title,_,body=section.partition('\n');slug=re.sub(r'[^\w -]','',title.lower()).replace(' ','-')
        topics.append({'id':slug,'title':title,'html':md.render(body)})
    readme=(src/'README.md').read_text()
    for title in ['Mutes','MIDI LEARN','PERFORM','CPU and PRIO','Knob acceleration','Key backlight']:
        m=re.search(r'(?ms)^### '+re.escape(title)+r'\n(.*?)(?=^#{2,3} |\Z)',readme)
        if m:topics.append({'id':re.sub(r'[^\w -]','',title.lower()).replace(' ','-'),'title':title,'html':md.render(m[1])})
    topics.append({'id':'engines-presets','title':'Sound engines, presets and FX','html':md.render('''EDIT selects the model. PRESETS browses/auditions; hold EDIT for the PRESETS list: LOAD, USER SAVE and DELETE with existing safeguards. HOME cancels audition. Model-specific labels/ranges come directly from the firmware; the initial drum kit is preserved.\n\nFX cycles FILTER (type/cutoff/resonance/envelope/decay), DIST/CHOR/DLY/REV, SLICER, RESON, then global delay, reverb and chorus controls. Hold FX for PERFORM. These track effects are distinct from the ACID ladder CUTOF/RESO.\n\nUSB/TRS MIDI hardware, physical calibration, real-device CPU shedding/recovery and firmware updates are not simulated. USB audio here is browser Web Audio, not a USB device. BLE is an inherited source option, not part of this simulator. Browser saves are IndexedDB storage on this origin; clearing site data removes them. The simulator has no hardware installation actions; the separate verified-package installer requires explicit user action.\n''')})
    (OUT/'manual.json').write_text(json.dumps(topics,ensure_ascii=False))
    docs=OUT/'docs';docs.mkdir(exist_ok=True)
    for p in (src/'docs').glob('*.md'):shutil.copy2(p,docs/p.name)
    for f in ['README.md','LICENSING.md','CHANGELOG.md']:shutil.copy2(src/f,docs/f)
    # Generated static manual view remains available without client-side scripting.
    (OUT/'manual.html').write_text('<!doctype html><meta charset="utf-8"><title>Nauclerus manual</title><link rel="stylesheet" href="sim.css"><main class="manual-static">'+''.join('<section id="'+t['id']+'"><h2>'+t['title']+'</h2>'+t['html']+'</section>' for t in topics)+'</main>')

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--host-only',action='store_true');ap.add_argument('--preview',action='store_true');a=ap.parse_args()
    OUT.mkdir(parents=True,exist_ok=True);src,digest=snapshot();gen=src/'build/gen';gen.mkdir(parents=True,exist_ok=True);host=OUT/'host';host.mkdir(exist_ok=True)
    env=os.environ.copy();env['PYTHONDONTWRITEBYTECODE']='1'
    run(['python3','-c','import sys;sys.path.insert(0,"tools");import build as b;b.generate()'],cwd=src,env=env)
    text=(src/'firmware/hal/fm1_input.h').read_text();match=re.search(r'static const int8_t FM1_KEYMAP.*?^};',text,re.S|re.M);assert match;(gen/'sim_keymap.h').write_text(match[0]+'\n')
    version=(src/'VERSION.txt').read_text().strip();defs=['-DFELUCCA_VERSION="Nauclerus '+version+'"'];flags=['-O2','-Wno-int-to-pointer-cast','-I'+str(gen),'-I'+str(src/'firmware/src'),'-I'+str(src/'tests'),*defs]
    inputs=json.loads((OUT/'source-manifest.json').read_text())['files']
    codekey=hashlib.sha256(json.dumps([(r['path'],r['sha256']) for r in inputs if r['path'].endswith(('.c','.h')) or r['path']=='VERSION.txt']).encode()).hexdigest()
    cached=(host/'verified-code-key').exists() and (host/'verified-code-key').read_text()==codekey
    if cached: print('Reusing previously passed native checks for identical C/header/version content',flush=True)
    else:
        for name in ['sim_test','sim_lab_test','sim_ids','sim_record','sim_feature_reel']:
            binary=host/name;run(['cc',*flags,src/'tests'/f'{name}.c','-lm','-o',binary])
            if name=='sim_ids':
                with (OUT/'lab_ids.json').open('w') as f:run([binary],cwd=src,stdout=f)
            elif name=='sim_record':run([binary,OUT/'reel.bin',OUT/'reel.hash',OUT/'reel.json'],cwd=src)
            elif name=='sim_feature_reel':run([binary,OUT/'features.bin',OUT/'features.json'],cwd=src)
            else:run([binary],cwd=src)
        (host/'verified-code-key').write_text(codekey)
    for name in ['reel.bin','features.bin']:
        with (OUT/(name+'.gz')).open('wb') as f,gzip.GzipFile(fileobj=f,filename='',mode='wb',mtime=0) as z:z.write((OUT/name).read_bytes())
    run(['node',src/'tests/sim_font.mjs',src/'assets/fonts/ter-u16n.bdf',OUT/'terminus.woff'])
    shutil.copyfile(src/'assets/fonts/Terminus-LICENSE.txt',OUT/'terminus-license.txt')
    if a.host_only:return
    if not shutil.which('emcc'):raise SystemExit('native emcc unavailable; no old wasm is substituted')
    # The installed compiler/cache are read-only; compiler-generated cache stays in build.
    cache=OUT/'emcache'
    if not cache.exists():shutil.copytree('/usr/share/emscripten/cache',cache)
    env['EM_CACHE']=str(cache)
    exports=re.findall(r'^(?:const )?(?:void|int|int32_t|uint32_t|float|uint8_t|uint16_t|lab_probe_t)\s*\*?\s*((?:sim_|lab_)\w+)\(', (src/'tests/sim_core.c').read_text()+'\n'+(src/'tests/sim_lab.c').read_text(),re.M)
    exports+=['remix_lookup','lab_probes','lab_buf','lab_probe_count','lab_probe_clear','lab_probe_size']
    run(['emcc',*flags,'-sSTANDALONE_WASM','--no-entry','-sINITIAL_MEMORY=16777216','-sSTACK_SIZE=262144','-sEXPORTED_FUNCTIONS='+json.dumps(['_'+s for s in dict.fromkeys(exports)]),src/'tests/sim_core.c','-o',OUT/'fm1sim.wasm'],env=env)
    for p in (src/'web/sim').iterdir():
        if p.is_file():shutil.copy2(p,OUT/p.name)
    manual(src)
    meta={'creator':'Mr. Roboto','product':project_identity.PRODUCT,'usb_product':project_identity.USB_PRODUCT,'project':'Nauclerus','version':version,'identity':'Nauclerus '+version,'source_digest':digest,'source_archive_sha256':hashlib.sha256((OUT/'source.tar.gz').read_bytes()).hexdigest(),'wasm_sha256':hashlib.sha256((OUT/'fm1sim.wasm').read_bytes()).hexdigest(),'commit':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),'dirty_snapshot':bool(subprocess.check_output(['git','status','--porcelain'],cwd=ROOT).strip()),'date':datetime.now(timezone.utc).strftime('%Y-%m-%d'),'installer_enabled':False,'publication_status':'local-unpublished','canonical':'https://mr-roboto.github.io/nauclerus/','since':[],'release':{'version':version,'notes':[]},'compiler':subprocess.check_output(['emcc','--version'],text=True).splitlines()[0]}
    for line in (src/'CHANGELOG.md').read_text().split('## 0.25.0-rc.1')[0].splitlines():
        if line.startswith('- '):
            text=line[2:].replace('**','').replace('`','');meta['since'].append({'title':' '.join(text.split()[:6])[:56],'text':text})
    (OUT/'version.json').write_text(json.dumps(meta,indent=2)+'\n');(OUT/'.nojekyll').touch()
    for f in ['LICENSE','LICENSING.md','CHANGELOG.md']:shutil.copy2(src/f,OUT/f)
    shutil.copytree(src/'LICENSES',OUT/'LICENSES',dirs_exist_ok=True)
    # One source fingerprint versions every runtime import/fetch, including worklets.
    names=[p.name for p in OUT.iterdir() if p.is_file() and p.suffix in {'.html','.js','.css','.json','.wasm','.gz','.woff','.txt'}]
    for p in OUT.iterdir():
        if p.suffix not in {'.html','.js','.css'} or not p.is_file():continue
        text=p.read_text()
        for name in names:
            for quote in ["'",'"']:
                text=text.replace(quote+name+quote,quote+name+'?v='+digest[:12]+quote).replace(quote+'./'+name+quote,quote+'./'+name+'?v='+digest[:12]+quote)
        p.write_text(text)
    run(['node','--test-reporter=spec' ,src/'tests/sim_glue.mjs',OUT],cwd=src)
    # Explicit allowlist: source/cache/host files never enter the deployment tree.
    from importlib.util import spec_from_file_location,module_from_spec
    spec=spec_from_file_location('make_site',src/'web/make_site.py');site=module_from_spec(spec);spec.loader.exec_module(site)
    dest=ROOT/'build/pages';site.portal(OUT,dest)
    run(['node','--test-reporter=spec',ROOT/'tests/sim_site.mjs'],cwd=ROOT)
    print('Local site:',dest,'; preview at http://127.0.0.1:8765/nauclerus/ ; NO publication or device package created')
if __name__=='__main__':main()
