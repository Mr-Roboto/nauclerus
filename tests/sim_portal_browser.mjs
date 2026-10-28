// SPDX-License-Identifier: GPL-3.0-only
// Run against tools/serve_portal.py. Uses an existing, explicitly supplied Playwright.
import {createRequire} from 'node:module';
import {mkdirSync,writeFileSync,readFileSync} from 'node:fs';
import assert from 'node:assert/strict';
const require=createRequire(import.meta.url);
const {chromium}=require(process.env.NAUCLERUS_PLAYWRIGHT||'playwright');
const expectInstaller=process.env.NAUCLERUS_EXPECT_INSTALLER==='1';
const base=process.env.NAUCLERUS_PORTAL_URL||'http://127.0.0.1:8765/nauclerus/';
const out='build/portal-checks';mkdirSync(out,{recursive:true});
const browser=await chromium.launch({headless:true,args:['--autoplay-policy=no-user-gesture-required']});
const report={url:base,checks:[],errors:[],screenshots:[]};
const check=(name,value)=>{assert.ok(value,name);report.checks.push(name);console.log('PASS',name);};
try {
browser.on('disconnected',()=>{});
const context=await browser.newContext({viewport:{width:1440,height:1000},reducedMotion:'reduce'});
await context.addInitScript(()=>{
  const Native=window.AudioContext;window.__audio=[];
  window.AudioContext=class extends Native {constructor(...args){super(...args);window.__audio.push(this);}};
  const Node=window.AudioWorkletNode;window.__nodes=[];
  window.AudioWorkletNode=class extends Node {constructor(ctx,...args){super(ctx,...args);const analyser=ctx.createAnalyser();this.connect(analyser);this.__analyser=analyser;window.__nodes.push(this);}};
});
const page=await context.newPage();page.setDefaultTimeout(15000);
page.on('pageerror',e=>report.errors.push(e.message));
page.on('console',m=>{if(m.type()==='error')report.errors.push(m.text());});
page.on('response',r=>{if(r.status()>=400)report.errors.push(`${r.status()} ${r.url()}`);});
await page.goto(base+'#play');await page.waitForSelector('#manual-topics section');
await page.locator('#power-on').click({timeout:15000});
await page.waitForFunction(()=>document.querySelector('#device').dataset.playing==='false');
check('Nauclerus identity and current source metadata',await page.evaluate(async({expectedVersion,expectInstaller})=>{const v=await(await fetch('version.json')).json();return document.title.startsWith('Nauclerus')&&v.project==='Nauclerus'&&v.version===expectedVersion&&v.installer_enabled===expectInstaller&&v.source_digest.length===64;},{expectedVersion:readFileSync('VERSION.txt','utf8').trim(),expectInstaller}));
await page.keyboard.press('Space',{delay:90});
await page.waitForFunction(()=>document.querySelector('#device').dataset.playing==='true');
await page.waitForTimeout(700);
const audio=await page.evaluate(()=>{let peak=0;for(const n of window.__nodes){const a=new Float32Array(n.__analyser.fftSize);n.__analyser.getFloatTimeDomainData(a);peak=Math.max(peak,...a.map(Math.abs));}return{peak,states:window.__audio.map(x=>x.state)};});
check('AudioWorklet transport and nonzero browser audio',audio.peak>0.001&&audio.states.includes('running'));report.audio=audio;
// A held modifier plus a piano key reaches the actual panel; release clears both.
await page.locator('[data-ctl="btn:5"]').click({button:'right'});
await page.keyboard.down('KeyZ');await page.waitForTimeout(140);
check('Simultaneous latched modifier and piano input',await page.locator('[data-ctl="btn:5"]').evaluate(e=>e.classList.contains('latched')));
await page.keyboard.up('KeyZ');await page.locator('[data-ctl="btn:5"]').click();
await page.keyboard.press('Space',{delay:90});
await page.screenshot({path:out+'/desktop-play.png'});report.screenshots.push('desktop-play.png');
await page.locator('a[href="#manual"]').first().click();
await page.locator('#manual-context').selectOption('tb');
await page.locator('#map-list li[data-ctl="OCT+"]').click();
check('Interactive panel highlights TB-3PO OCT+ context',await page.locator('#map-caption').textContent().then(t=>t.includes('NEW LINE'))&&await page.locator('#manual-mini .on').count()>0);
await page.locator('#manual-search').fill('PWM');
check('Manual search and anchor navigation',await page.locator('#manual-topics section:visible').count()>0&&await page.locator('#manual-topics section[hidden]').count()>0&&await page.locator('#manual-nav a').count()>=18);
await page.locator('#manual-search').fill('');await page.locator('#manual-context').selectOption('acid');
await page.screenshot({path:out+'/desktop-manual.png'});report.screenshots.push('desktop-manual.png');
await page.locator('#manual-orb-remix .manual-try').click();
await page.waitForFunction(()=>document.querySelector('#device').dataset.context==='16');
check('Manual deep link navigates to the actual REMIX page',true);
// Stop the main session writing while testing a separate simulator's persistent flash.
await page.evaluate(async()=>{for(const ctx of window.__audio)await ctx.close();});
// Production WASM integration on the browser engine, independent of the user's main session.
const production=await page.evaluate(async()=>{
  const {Fm1Sim}=await import('./engine.js'),{Lab}=await import('./lab.js');
  const ids=await(await fetch('lab_ids.json')).json(),bytes=await(await fetch('fm1sim.wasm')).arrayBuffer();
  const lab=await Lab.create(bytes,ids),sim=lab.sim,x=sim.x,{P,DM,LF}=ids;
  const run=n=>{for(;n>0;n-=128)sim.render(Math.min(n,128));};
  const tap=b=>{sim.btn(b,true);run(2048);sim.btn(b,false);run(2048);};
  lab.reset();for(const k of [3,6]){lab.model(k,DM.ACID);lab.generate(k,true);}
  const independent=x.sim_tb_seed(3)!==0&&x.sim_tb_seed(6)!==0;
  let waves=[];for(const w of [0,1,3]){lab.param(3,P.E6,w);lab.param(3,P.AMORPH,64);lab.hit(3);waves.push(Math.max(...lab.renderMono(4096).map(Math.abs)));}
  lab.reset();lab.model(0,DM.ACID);lab.param(0,P.E6,1);lab.param(0,P.E7,100);lab.step(0,0,true);lab.note(0,0,-12);lab.param(0,P.SLEN,1);
  let hashes=[];for(const width of [0,64,127]){lab.param(0,P.AMORPH,width);lab.hit(0);const s=lab.renderMono(8192);hashes.push(s.reduce((h,v)=>Math.imul(h^Math.round(v*1e7),16777619),2166136261));}
  let modes=[];for(let mode=0;mode<6;mode++){lab.param(0,P.LSRC1,mode);lab.param(0,P.LFO1+LF.DEST,19);lab.param(0,P.LFO1+LF.DEPTH,40);lab.param(0,P.LSRC2,mode);modes.push(x.sim_get_param(0,P.LSRC1)===mode&&x.sim_get_param(0,P.LSRC2)===mode);lab.renderMono(2048);}
  lab.remix(0,17,true);const remix=x.sim_remix_state(0)!==0;
  lab.recordDemo(0);lab.lock(0,3,P.E2,100);lab.play(true);const eventAudio=Math.max(...lab.renderMono(44100).map(Math.abs));lab.play(false);
  // Actual TB-3PO UI with KEYS and ENV flags, stopped and running.
  let generator=true;for(const play of [false,true])for(const keys of [false,true])for(const env of [false,true]){
    lab.reset();lab.model(0,DM.ACID);x.lab_select(0);run(4096);tap(8);if(keys)tap(1);if(env)tap(2);if(play)tap(10);
    const before=x.sim_tb_seed(0);tap(13);generator&&=before!==x.sim_tb_seed(0)&&x.sim_context()===ids.GR.TB3PO;tap(12);
  }
  lab.play(false);
  // Save real edited project via the normal UI, then persist its flash sectors.
  lab.reset();run(4096);lab.param(0,P.LEVEL,81);lab.step(0,0,true);lab.remix(0,17,true);tap(7);const saveContext=x.sim_context();sim.enc(6,1);run(2048);sim.enc(6,1);run(4096);
  const {FlashStore,openFlashDb}=await import('./store.js');const store=new FlashStore(openFlashDb,()=>{});await store.load();await store.clear();const clean=new FlashStore(openFlashDb,()=>{});await clean.load();const dirty=sim.takeDirty();clean.put(dirty);
  while(clean.busy||clean.pending.size)await new Promise(r=>setTimeout(r,20));clean.db.close();

  // Song uses the saved slot while keeping live configuration on STOP.
  tap(9);for(let n=0;n<4;n++){sim.enc(0,1);run(2048);}const songPage=x.sim_context();
  sim.enc(4,1);run(2048);sim.enc(6,1);run(2048);tap(10);const songPlaying=sim.playing();
  let songPeak=0;for(let n=0;n<500;n++){const b=sim.render(128);for(const v of b.left)songPeak=Math.max(songPeak,Math.abs(v));}tap(10);
  const table=new Uint8Array(4096);for(let n=0;n<4096;n++)table[n]=x.remix_lookup(n>>8,(n>>4)&15,n&15);
  const tableSHA=[...new Uint8Array(await crypto.subtle.digest('SHA-256',table))].map(b=>b.toString(16).padStart(2,'0')).join('');
  return{songPage,songPlaying,songPeak,tableSHA,independent,waves,pwmDifferent:new Set(hashes).size===3,modes,remix,eventAudio,generator,models:ids.models.length,presets:ids.presets.factory,saveContext,savedLevel:x.sim_get_param(0,P.LEVEL)};
});
check('Current models, ACID waves and independent generator tracks',production.models===45&&production.presets===152&&production.independent&&production.waves.every(v=>v>0.001));
check('PWM, both LFO source/mode sets, Remix, Motion and locks',production.pwmDifferent&&production.modes.every(Boolean)&&production.remix&&production.eventAudio>0.001);
check('Canonical 256-pattern ORB table in browser WASM',production.tableSHA==='88034625d620ce4a1a87f954edf81850adf261155a7891fabfea68b01b6d60cb');
check('Saved-slot Song playback starts and produces audio',production.songPlaying&&production.songPeak>0.001);
check('TB-3PO OCT actions with all SEL/ENV and transport combinations',production.generator);report.production=production;
await page.reload();await page.waitForSelector('#manual-topics section');
const saved=await page.evaluate(async()=>{const {FlashStore,openFlashDb}=await import('./store.js');const store=new FlashStore(openFlashDb,()=>{});const sectors=await store.load();const {Fm1Sim}=await import('./engine.js');const ids=await(await fetch('lab_ids.json')).json();const sim=await Fm1Sim.create(await(await fetch('fm1sim.wasm')).arrayBuffer());sim.boot(new Map(sectors));sim.render(1024);store.db.close();return{sectors:sectors.length,level:sim.x.sim_get_param(0,ids.P.LEVEL)};});
report.saved=saved;console.log('SAVED',JSON.stringify(saved));check('Real PROJECT SAVE, IndexedDB reload and power-on restore',saved.sectors>0&&saved.level===81);report.saved=saved;
// Mount every new live lab and exercise its UI/play button.
for(const kind of ['acid','melody','remix','events']){const plate=page.locator(`[data-piece="${kind}"]`);await plate.scrollIntoViewIfNeeded();await plate.locator('.lab-play').waitFor();await plate.locator('.lab-play').click();await page.waitForTimeout(180);check(`${kind} production demonstration plays`,await plate.locator('.lab-play').getAttribute('aria-pressed')==='true');await plate.locator('.lab-play').click();}
await page.locator('[data-piece="acid"]').scrollIntoViewIfNeeded();await page.screenshot({path:out+'/desktop-labs.png'});report.screenshots.push('desktop-labs.png');
const links=await page.evaluate(()=>[...document.querySelectorAll('a[href]')].map(a=>a.getAttribute('href')));
for(const href of new Set(links.filter(h=>(expectInstaller||h!=='webapp/installer/')&&!h.startsWith('http')&&!h.startsWith('#')&&!h.startsWith('mailto:')))){const r=await page.request.get(new URL(href,base).href);check('Local link '+href,r.ok());}
check('Canonical releases and no obsolete installer',links.includes('https://github.com/Mr-Roboto/nauclerus/releases')&&!links.some(h=>/deadactive.github.io/.test(h)));
await context.close();
const mobile=await browser.newContext({viewport:{width:390,height:844},isMobile:true,hasTouch:true,reducedMotion:'reduce'});const mp=await mobile.newPage();mp.setDefaultTimeout(15000);mp.on('pageerror',e=>report.errors.push(e.message));await mp.goto(base+'#manual');await mp.waitForSelector('#manual-topics section');
check('Mobile manual fits viewport',await mp.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1));await mp.screenshot({path:out+'/mobile-manual.png'});report.screenshots.push('mobile-manual.png');await mp.goto(base+'#play');await mp.locator('#power-on').click();await mp.waitForFunction(()=>document.querySelector('#device').dataset.playing==='false');await mp.screenshot({path:out+'/mobile-play.png'});report.screenshots.push('mobile-play.png');check('Mobile panel boots and fits viewport',await mp.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1));await mobile.close();
check('No browser exceptions or missing resources',report.errors.length===0);
report.status='PASS';
}catch(error){report.status='FAIL';report.failure=error.stack;console.error(error);process.exitCode=1;}
finally{writeFileSync(out+'/browser-report.json',JSON.stringify(report,null,2)+'\n');await browser.close();}
