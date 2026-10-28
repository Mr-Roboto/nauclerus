// SPDX-License-Identifier: GPL-3.0-only
// Nauclerus demonstrations: configure the existing isolated production-firmware lab.
// All oscillation, routing, generation, recording, locks and mapping run in WASM.
import { el } from './lab_ui.js';
export default function nauclerus(plate,{lab,ids,screen}) {
  const kind=plate.dataset.piece,{P,LF,DM,fs}=ids;
  const box=el('div','nau-lab'), controls=el('div','lab-controls'), canvas=el('canvas'), readout=el('p','readout');canvas.width=720;canvas.height=150;box.append(controls,canvas,readout);plate.append(box);
  let player=null,wave=1,morph=64,polarity=0,depth=0,selection=1,condition=0,ratchets=0,useMotion=true;
  function select(label,choices,change){const l=el('label','',label),s=el('select');s.setAttribute('aria-label',label);for(const [value,text] of choices){const o=el('option','',text);o.value=value;s.append(o);}s.addEventListener('change',()=>change(Number(s.value)));l.append(s);controls.append(l);return s;}
  function slider(label,value,min,max,change){const l=el('label','',label),input=el('input');input.type='range';input.min=min;input.max=max;input.value=value;input.setAttribute('aria-label',label);input.addEventListener('input',()=>change(Number(input.value)));l.append(input);controls.append(l);}
  const local={call:(fn,...args)=>lab[fn](...args)};
  function setup(t){
    t.call('model',0,kind==='remix'?DM.BDTRX:DM.ACID);
    t.call('param',0,P.SRC,0);t.call('param',0,P.SLEN,16);t.call('param',0,P.LEVEL,104);
    for(let i=0;i<16;i++){t.call('step',0,i,kind==='remix'?i%3===0:true,i%4===0,kind==='events'?condition:0,kind==='events'?ratchets:0);t.call('note',0,i,-27+(i%4)*2,false,false);}
    if(kind==='acid'){
      t.call('param',0,P.E6,wave);t.call('param',0,P.AMORPH,morph);t.call('param',0,P.E7,100);
      t.call('param',0,P.LSRC1,polarity);t.call('param',0,P.LFO1+LF.DEST,19);t.call('param',0,P.LFO1+LF.DEPTH,depth);
      t.call('param',0,P.LFO1+LF.WAVE,ids.names.lwave.indexOf('SINE'));t.call('param',0,P.LFO1+LF.RATE,46);
    }else if(kind==='melody')t.call('generate',0,true);
    else if(kind==='remix')t.call('remix',0,selection,true);
    else if(kind==='events'){
      if(useMotion)t.call('recordDemo',0);
      t.call('lock',0,3,P.E2,120);t.call('lock',0,11,P.E2,15);
    }
  }
  function draw(){
    lab.reset();setup(local);lab.play(true);const samples=lab.renderMono(fs*2);
    const g=canvas.getContext('2d'),style=getComputedStyle(document.documentElement);g.clearRect(0,0,canvas.width,canvas.height);g.strokeStyle=style.getPropertyValue('--secondary');g.beginPath();
    for(let x=0;x<canvas.width;x++){let peak=0;const from=Math.floor(x*samples.length/canvas.width),to=Math.floor((x+1)*samples.length/canvas.width);for(let i=from;i<to;i++)if(Math.abs(samples[i])>Math.abs(peak))peak=samples[i];g[x?'lineTo':'moveTo'](x,75-peak*70);}g.stroke();
    if(kind==='remix')readout.textContent='Source positions: '+Array.from({length:16},(_,i)=>lab.x.remix_lookup(selection>>4,selection&15,i)+1).join(' ');
    else readout.textContent=kind==='acid'?`Production waveform · MORPH/PWM ${morph} · ${['BIP','','UNI+','','UNI-'][polarity]} · DEPTH ${depth}`:kind==='melody'?'Production TB-3PO · NEW uses fresh entropy; MUTATE preserves anchors.':'Actual Motion capture plus per-step CUTOF locks; no JS automation engine.';
    if(player){player.call('reset');setup(player);player.call('play',true);}screen();
  }
  if(kind==='acid'){
    const s=select('WAVE',[[0,'SAW'],[1,'PULSE'],[3,'MORPH']],v=>{wave=v;draw();});s.value='1';
    slider('MORPH / PWM',64,0,127,v=>{morph=v;draw();});select('Polarity',[[0,'L1BI / BIP'],[2,'L1U+ / UNI+'],[4,'L1U- / UNI-']],v=>{polarity=v;draw();});slider('DEPTH',0,-64,64,v=>{depth=v;draw();});
  }else if(kind==='melody'){
    for(const [label,fresh] of [['NEW LINE',true],['MUTATE',false]]){const b=el('button','bar-btn',label);b.type='button';b.addEventListener('click',()=>{if(fresh)lab.reset(),setup(local);else lab.generate(0,false);lab.play(true);const a=lab.renderMono(fs);readout.textContent=`${label}: actual generator, seed ${lab.x.sim_tb_seed(0)}; ${a.length} rendered frames.`;if(player)player.call('generate',0,fresh);screen();});controls.append(b);}
  }else if(kind==='remix'){
    select('BANK',Array.from({length:16},(_,i)=>[i,String.fromCharCode(65+i)]),v=>{selection=v*16+(selection&15);draw();});const s=select('PATTERN',Array.from({length:16},(_,i)=>[i,String(i+1)]),v=>{selection=(selection&240)+v;draw();});s.value='1';
  }else{
    select('PROB',[[0,'100%'],[11,'50%'],[21,'1-SHOT'],[22,'1/2']],v=>{condition=v;draw();});select('RATCH',[[0,'1'],[1,'2'],[2,'3'],[3,'4']],v=>{ratchets=v;draw();});select('Motion',[[1,'Recorded control moves'],[0,'Locks only']],v=>{useMotion=!!v;draw();});
  }
  draw();return {start(p){player=p;setup(p);p.call('play',true);},stop(){player=null;}};
}
