// SPDX-License-Identifier: GPL-3.0-only
// Drum machine fork: 2026 DEADACTIVE
// The manual's panel map: a small FM-1 (mini.js) beside the list of its controls. Pointing at a control, or at its
// line in the list (hover, tap, focus), lights both and shows the line under the FM-1. The list is the content; the
// drawing is a picture of it (aria-hidden).
import { KEYS, BTN, ENC } from './controls.js?v=319fa1756664';
import { buildMini } from './mini.js?v=319fa1756664';

const GROUPS = {                                   // data-ctl names for several parts at once
  white: KEYS.filter(k => !k.black).map(k => `key:${k.n}`),
  black: KEYS.filter(k => k.black).map(k => `key:${k.n}`),
};

const contextual = {
  tb: {'OCT+':'NEW LINE, with KEYS or ENV/STEPS enabled; explicit held editing combinations keep priority.', 'OCT-':'MUTATE; same production generator while stopped or playing.', KNOB1:'DENS on GENERATE / ROOT on SCALE.', KNOB2:'ACCENT / SCALE.', KNOB3:'SLIDE / OCTAVE.', KNOB4:'OCTS / automatic MUTATE interval (pattern loops).'},
  remix:{'OCT+':'Confirm draft BANK/PATTERN at the next safe step when playing.', 'OCT-':'No Remix bank-navigation action.', KNOB1:'Draft BANK A–P.',KNOB2:'Draft PATTERN 1–16.',KNOB3:'REMIX ON/OFF immediately queues the confirmed selection; never confirms a draft.',KNOB4:'SET: clockwise confirms, exactly like OCT+.'},
  grids:{'OCT+':'WRITE: stopped only; overwrites eligible drum Grids tracks into 64 normal steps. Save first.',KNOB4:'Page 2: ASSIGN / 3 TRACKS; clockwise sets T1–T3 sources, not their models.'},
  project:{'OCT-':'LOAD through existing two-activation safeguard.', 'OCT+':'SAVE through existing two-activation safeguard.', KNOB1:'SLOT A–D.',KNOB2:'NAME.',KNOB3:'LOAD.',KNOB4:'SAVE.'},
  menu:{SEL:'ENTER on ABOUT/calibration dedicated screens only. Does not toggle KEYS or scalar settings.', SELECT:'Move the menu cursor; scroll ABOUT.',HOME:'Short cancels to real HOME.', 'OCT+':'OK / ENTER.', 'OCT-':'BACK.'},
  acid:{KNOB1:'Pages 1–4: MODEL / TUNE / GATE / CHOKE.',KNOB2:'WAVE / DECAY / ACCNT / PRIO.',KNOB3:'CUTOF / ENVMD / PAN / NOTE.',KNOB4:'RESO / LEVEL / MORPH / empty. MORPH is disabled on SAW.'},
};
const basic = Object.fromEntries([...BTN,...ENC].map(c=>[c.label,c.tip||'Context-dependent control.']));
basic['OCT+']='KEYS octave or step-bank/action in the current page. TB-3PO NEW, REMIX SET, GRIDS WRITE and PROJECT SAVE take context priority.';
basic['OCT-']='KEYS octave, previous bank, TB-3PO MUTATE, PROJECT LOAD or modal BACK; held-step lock clearing retains priority.';
basic.MASTER='Master output level, independent from track LEVEL.';
basic.white='Track triggers, KEYS notes or chronological steps according to mode; held keys can form combinations.';
basic.black='Keyboard notes, PERFORM effects, MUTE options or naming functions according to context.';
function content(context){return {...basic,...contextual[context]};}

export async function initManual() {
  const host = document.getElementById('manual-mini'), list = document.getElementById('map-list');
  if(!host||!list)return;
  list.innerHTML='<h3>Buttons, encoders and keys</h3><ul>'+Object.keys(basic).map(k=>`<li data-ctl="${k}"><b>${k}</b> <span></span></li>`).join('')+'</ul>';
  let topics,ids;
  try{[topics,ids]=await Promise.all([fetch('manual.json?v=319fa1756664').then(r=>{if(!r.ok)throw Error('Manual unavailable');return r.json();}),fetch('lab_ids.json?v=319fa1756664').then(r=>r.json())]);}
  catch(error){document.getElementById('manual-topics').textContent=error.message;return;}
  const nav=document.getElementById('manual-nav'), body=document.getElementById('manual-topics');body.replaceChildren();
  const page={ 'acid-sound-engine':['EDIT',0], 'tb-3po-melody-generator':['ARP',0], 'orb-remix':['SEQ',2], 'steps-probability-and-ratchets':['SEQ',0], 'motion-and-parameter-locks':['SEQ',3], 'two-lfos-and-four-routing-slots':['LFO',3], 'grids-and-write':['ARP',0], 'projects-and-song':['SAVE',0], 'color-themes':['GLO',0], 'engines-presets':['EDIT',0], 'perform':['FX',0] };
  for(const t of topics){
    const item=document.createElement('a');item.href='#manual-'+t.id;item.textContent=t.title;nav.append(item);
    const section=document.createElement('section');section.id='manual-'+t.id;section.dataset.search=(t.title+' '+t.html).toLowerCase();
    const heading=document.createElement('h2');heading.textContent=t.title;section.append(heading);
    const text=document.createElement('div');text.innerHTML=t.html;section.append(text);
    const open=document.createElement('a');open.href='#play';open.textContent='Try this on the live panel →';open.className='manual-try';
    open.addEventListener('click',()=>{const [button,offset]=page[t.id]||['HOME',0];window.dispatchEvent(new CustomEvent('nauclerus:navigate',{detail:{button,offset}}));});section.append(open);body.append(section);
  }
  document.getElementById('manual-search').addEventListener('input',e=>{
    const q=e.target.value.toLowerCase().trim();for(const s of body.children)s.hidden=!!q&&!s.dataset.search.includes(q);
    for(const a of nav.children)a.hidden=document.querySelector(a.hash)?.hidden||false;
  });
  const context=document.getElementById('manual-context');
  const fill=c=>{const tips=content(c);for(const li of list.querySelectorAll('li'))li.querySelector('span').textContent=tips[li.dataset.ctl];};
  let active='general';
  window.addEventListener('nauclerus:context',e=>{const m=e.detail;active=m.context>=100&&m.context<200?'menu':m.context===ids.GR.TB3PO?'tb':m.context===ids.GR.REMIX?'remix':m.context===ids.GR.GRIDS?'grids':m.context===ids.GR.SLOTS?'project':m.context===ids.GR.MODEL&&m.model===ids.DM.ACID?'acid':'general';if(context.value==='auto')fill(active);});
  fill('general');context.addEventListener('change',()=>{fill(context.value==='auto'?active:context.value);document.getElementById('map-caption').textContent='Panel context: '+context.selectedOptions[0].textContent;});

  const caption = document.getElementById('map-caption');
  if (!host || !list || !caption) return;
  const parts = buildMini(host);
  const items = [...list.querySelectorAll('li[data-ctl]')];
  const partsOf = li => li.dataset.ctl.split(' ').flatMap(c => GROUPS[c] || [c]).map(k => parts[k]).filter(Boolean);
  const idle = [...caption.childNodes];
  let shown = null;
  const show = li => {
    if (li === shown) { caption.replaceChildren(...[...li.childNodes].map(n=>n.cloneNode(true))); return; }
    if (shown) { shown.classList.remove('on'); for (const p of partsOf(shown)) p.classList.remove('on'); }
    shown = li;
    li.classList.add('on');
    for (const p of partsOf(li)) p.classList.add('on');
    caption.replaceChildren(...[...li.childNodes].map(n => n.cloneNode(true)));
  };
  const clear = () => {
    if (!shown) return;
    shown.classList.remove('on');
    for (const p of partsOf(shown)) p.classList.remove('on');
    shown = null;
    caption.replaceChildren(...idle);
  };
  for (const li of items) {
    li.tabIndex = 0;
    for (const ev of ['pointerenter', 'click', 'focus']) li.addEventListener(ev, () => show(li));
    for (const p of partsOf(li)) {
      p.classList.add('hit');
      for (const ev of ['pointerenter', 'click']) p.addEventListener(ev, () => show(li));
    }
  }
  list.addEventListener('focusout', e => { if (!list.contains(e.relatedTarget)) clear(); });
}
