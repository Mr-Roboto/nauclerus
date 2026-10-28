// SPDX-License-Identifier: GPL-3.0-only
// Drum machine fork: 2026 DEADACTIVE
// The site around the simulator: the landing page (web/sim/index.html) and the restyled English installer
// (web/index_pkg.html). Run from the repo root: node tests/sim_site.mjs
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { execFileSync } from 'node:child_process';

// ---- the installer: restyled and rebranded, its install logic untouched
const ORIG_REV = '12a1ba0';                         // the installer's last change on main (upstream 1.0 installer)
const NOW = readFileSync('web/index_pkg.html', 'utf8');
let ORIG = null;
try { ORIG = execFileSync('git', ['show', `${ORIG_REV}:web/index_pkg.html`], { encoding: 'utf8', stdio: ['ignore', 'pipe', 'ignore'] }); }
catch { /* a source archive without git history: the guard is skipped */ }

const script = html => html.match(/<script type="module">([\s\S]*?)<\/script>/)[1];
const TEXT_RE = /const TEXT = (\{[\s\S]*?\n\});\n/;
const masked = s => s.replace(TEXT_RE, 'const TEXT = /*TEXT*/;\n');
const table = html => Function(`return ${script(html).match(TEXT_RE)[1]}`)();
// the only script changes allowed (spec): English only, and the status label's name
const CHANGES = [
  ['let lang = (navigator.language || "en").toLowerCase().startsWith("ja") ? "ja" : "en";', 'const lang = "en";'],
  ['  $("lang").textContent = t("other");\n', ''],
  ['$("lang").addEventListener("click", () => { lang = lang === "ja" ? "en" : "ja"; applyLang(); });\n', ''],
  ['say(`Felucca ${meta.version}`);', 'say(`Hortator ${meta.version}`);'],
  // the install count (2026-10-22): one GoatCounter event after Done, never awaited (web/sim/counter.js)
  ['if (k === "done") { $("bar").value = 100; sayK("done"); }',
    'if (k === "done") { $("bar").value = 100; sayK("done"); countInstall(meta.version); }'],
  // two versions in one release (2026-10-09): the package loader runs again for the chosen version
  ['(async () => {', 'const load = async () => {'],
  ['})();\n', `};
load();
loadPageCounter();
if (counterOn()) $("counted").hidden = false;
if ((meta.variants || []).length > 1) $("variants").hidden = false;
document.querySelectorAll('input[name="variant"]').forEach((r) => r.addEventListener("change", () => {
  const v = meta.variants.find((x) => x.id === r.value);
  if (!v || writing) return;
  meta.pkg = v.pkg;
  meta.version = v.version;
  image = null;
  $("go").disabled = true;
  const radios = document.querySelectorAll('input[name="variant"]');   // one download at a time: no stale image
  radios.forEach((x) => { x.disabled = true; });
  load().finally(() => radios.forEach((x) => { x.disabled = false; }));
}));
`],
];

test('installer: the script is main\'s, changed only in its text table, the language lines and the label', { skip: !ORIG && 'no git history' }, () => {
  let want = masked(script(ORIG));
  for (const [from, to] of CHANGES) {
    assert.ok(want.includes(from), `main's installer no longer has: ${from.trim()} (update CHANGES)`);
    want = want.replace(from, to);
  }
  assert.equal(masked(script(NOW)), want);
});

test('installer: English only, every message the logic can show is still there', { skip: !ORIG && 'no git history' }, () => {
  const old = table(ORIG), now = table(NOW);
  assert.deepEqual(Object.keys(now), ['en'], 'one language');
  for (const k of Object.keys(old.en)) if (k !== 'other' && k !== 'editor') assert.equal(typeof now.en[k], 'string', k);
  for (const [, k] of NOW.matchAll(/data-t="(\w+)"/g)) assert.equal(typeof now.en[k], 'string', `data-t ${k}`);
});

test('installer: named Hortator, no language switch, /*LIB*/ and /*META*/ kept for make_site.py', () => {
  assert.match(NOW, /<html lang="en">/);
  assert.match(NOW.match(/<title>(.*)<\/title>/)[1], /Hortator/);
  assert.match(NOW.match(/<h1[^>]*>([\s\S]*?)<\/h1>/)[1], /Hortator/);
  assert.doesNotMatch(NOW, /FM-1 Drums|FM-1 DRUMS/, 'the old name is gone');
  assert.doesNotMatch(NOW, /id="lang"/);
  assert.equal(NOW.split('/*LIB*/').length, 2);
  assert.equal(NOW.split('/*META*/').length, 2);
  for (const id of ['go', 'bar', 'status', 'log']) assert.match(NOW, new RegExp(`id="${id}"`), id);
});

test('installer: a version choice (with or without Bluetooth), hidden unless the site has two packages', () => {
  const fs = NOW.match(/<fieldset id="variants" hidden>([\s\S]*?)<\/fieldset>/);
  assert.ok(fs, 'a hidden fieldset id="variants"');
  const values = [...fs[1].matchAll(/<input type="radio" name="variant" value="(\w+)"( checked)?>/g)];
  assert.deepEqual(values.map((m) => m[1]), ['std', 'ble']);
  assert.equal(values[0][2], ' checked', 'the version without Bluetooth is the default');
});

// ---- the landing page: the simulator as the hero, what it does, install
const LANDING = readFileSync('web/sim/index.html', 'utf8');

test('landing: Nauclerus hero, simulator, manual, release status and matching source', () => {
  for(const id of ['hero','features','manual','release-status','play','device','tilt'])assert.match(LANDING,new RegExp('id="'+id+'"'));
  assert.match(LANDING, /<title>Nauclerus/);
  assert.match(LANDING, /href="source\.tar\.gz"/);
  assert.match(LANDING, /https:\/\/github.com\/Mr-Roboto\/nauclerus\/releases/);
  assert.doesNotMatch(LANDING, /deadactive.github.io/);
});

// ---- layout.js: the panel's two layouts, the morph between them, the hero's choreography (pure, no DOM)
import { LAYOUTS, blend, heroPose, outline } from '../web/sim/layout.js';

test('layouts: both have every control, a 240 px LCD, the same knobs', () => {
  const { landscape: L, portrait: P } = LAYOUTS;
  assert.deepEqual(Object.keys(L.ctl).sort(), Object.keys(P.ctl).sort());
  assert.equal(Object.keys(L.ctl).length, 14 + 27);
  assert.deepEqual(Object.keys(L.enc).sort(), Object.keys(P.enc).sort());
  assert.equal(L.lcd[2], 240);
  assert.equal(P.lcd[2], 240);
});

test('blend: exactly the landscape at 0 and the portrait at 1; the OCT tray, which the phone has not, fades out', () => {
  const { landscape: L, portrait: P } = LAYOUTS;
  const a = blend(L, P, 0), b = blend(L, P, 1), h = blend(L, P, 0.5);
  assert.deepEqual([a.W, a.H, a.lcd, a.ctl, a.enc], [L.W, L.H, L.lcd, L.ctl, L.enc]);
  assert.deepEqual([b.W, b.H, b.lcd, b.ctl, b.enc], [P.W, P.H, P.lcd, P.ctl, P.enc]);
  for (const name of Object.keys(P.deco)) assert.deepEqual(b.deco[name].box, P.deco[name], name);
  assert.equal(a.deco.oct.opacity, 1);
  assert.equal(b.deco.oct.opacity, 0);
  assert.ok(h.W < L.W && h.W > P.W, 'halfway is between the two');
  assert.deepEqual(blend(L, L, 0.7).ctl, L.ctl, 'desktop: nothing moves');
});

test('hero pose: far, tilted and blurred at the top; flat, sharp and arrived at the end, desktop and phone', () => {
  for (const phone of [false, true]) {
    const s = heroPose(0, phone), e = heroPose(1, phone);
    assert.ok(s.blur >= 8 && s.transform !== 'none' && !s.arrived && s.copy === 1, `start (${phone})`);
    assert.ok(e.blur === 0 && e.transform === 'none' && e.arrived && e.copy === 0, `end (${phone})`);
    assert.equal(e.morph, phone ? 1 : 0);
  }
});

test('hero pose: the shine never plays while the scene is blurred (a blur filter re-renders the whole scene)', () => {
  for (const phone of [false, true])
    for (let i = 0; i <= 1000; i++) {
      const q = heroPose(i / 1000, phone);
      assert.ok(!(q.blur > 0 && q.glare > 0), `p=${i / 1000} phone=${phone}: blur ${q.blur}, glare ${q.glare}`);
      if (phone && q.morph > 0) assert.equal(q.transform, 'none', `p=${i / 1000}: the phone morphs only once flat`);
    }
});

// ---- the features: eight chapters, each a piece on the firmware's own DSP (features.js), each telling where it is on
// the panel (Bluetooth, a separate test build, has no panel control yet)
import { BTN, ENC } from '../web/sim/controls.js';

test('features: current firmware chapters and real panel controls', () => {
  const sec=LANDING.slice(LANDING.indexOf('id="features"'),LANDING.indexOf('id="release-status"'));
  const ids=[...sec.matchAll(/<article class="chapter" id="f-(\w+)"/g)].map(m=>m[1]);
  assert.deepEqual(ids,['voices','reson','comp','lfo','seq','grids','fx','acid','melody','remix','events']);
  const labels=new Set([...BTN,...ENC].map(c=>c.label));
  for(const ch of sec.split('<article').slice(1)) {
    const id=ch.match(/id="f-(\w+)"/)[1];
    assert.match(ch,new RegExp('data-piece="'+id+'"'));
    const where=ch.match(/<p class="where">([\s\S]*?)<\/p>/)[1];
    const keys=[...where.matchAll(/<b>([^<]+)<\/b>/g)].map(m=>m[1]);
    assert.ok(keys.length,id);for(const key of keys)assert.ok(labels.has(key),key);
  }
});

test('release status: no installation control, current identity and explicit local status', () => {
  const sec=LANDING.slice(LANDING.indexOf('id="release-status"'),LANDING.indexOf('<footer'));
  for(const id of ['fw-version','whats-new'])assert.match(sec,new RegExp('id="'+id+'"'));
  assert.match(sec,/preview has no installation controls enabled/i);assert.match(sec,/id="release-install-link"[^>]*hidden/);
  assert.doesNotMatch(sec,/requestMIDIAccess|<button/);
});

test('Controls opens its help over the hero stage, next to the button (not below the whole scroll track)', () => {
  const stage = LANDING.slice(LANDING.indexOf('id="stage"'), LANDING.indexOf('id="play"'));
  assert.match(stage, /id="help-toggle"/);
  assert.match(stage, /id="help"/, 'the help panel is inside the sticky stage');
});

test('footer: the name and its origin, the ways around the page, the credits and the licence', () => {
  const foot = LANDING.slice(LANDING.indexOf('<footer'), LANDING.indexOf('</footer>'));
  assert.match(foot, /NAUCLERUS/);
  for (const href of ['#play', '#manual', '#release-status', 'source.tar.gz']) assert.ok(foot.includes(`href="${href}"`), href);
  for (const credit of ['Felucca', 'Leo Kuroshita', 'DEADACTIVE', 'Terminus', 'CC0', 'GPL-3.0', 'not affiliated',
                        'Mutable Instruments', 'Emilie Gillet', 'DaisySP', 'Electrosmith', 'Joe Landers', 'Gearmulator',
                        'Elektron', 'jl-uboot-tool', 'Fukiai'])
    assert.ok(foot.includes(credit), credit);
});

test('hero pose: the device lands early and holds: the last 40 % of the scroll track is the simulator, flat and usable', () => {
  for (const phone of [false, true])
    for (let i = 600; i <= 1000; i++) {
      const q = heroPose(i / 1000, phone);
      assert.ok(q.arrived && q.transform === 'none' && q.blur === 0 && q.glare === 0, `p=${i / 1000} phone=${phone}`);
    }
});

test('the simulator bar: the power caption, Controls and Reset in one row; the reset confirm is a pop-over in it', () => {
  const bar = LANDING.slice(LANDING.indexOf('id="play-bar"'), LANDING.indexOf('id="help"'));
  for (const id of ['power-note', 'help-toggle', 'reset', 'reset-confirm']) assert.match(bar, new RegExp(`id="${id}"`), id);
  assert.doesNotMatch(bar, /id="power-on"/, 'the power switch is on the device now');
  assert.match(LANDING, /class="hero-end"[^>]*id="play"|id="play"[^>]*class="hero-end"/);
});

// ---- web/make_site.py: the site's front page is the landing page with the simulator (when build/sim is built)
import { existsSync, rmSync } from 'node:fs';

test('make_site: source-matched simulator and licenses, no device package/installer', () => {
  const out='build/portal-site-test';
  const py=process.env.PYTHON||'python3';
  execFileSync(py,['-c','import sys;sys.path.insert(0,"web");import make_site;make_site.portal("build/sim",sys.argv[1])',out]);
  for(const name of ['index.html','fm1sim.wasm','app.js','manual.js','manual.json','manual.html','version.json','source.tar.gz','source-manifest.json','LICENSE','LICENSES'])assert.ok(existsSync(out+'/'+name),name);
  for(const name of ['webapp','firmware','package.fwsc','source','host','emcache'])assert.ok(!existsSync(out+'/'+name),name);
  const meta=JSON.parse(readFileSync(out+'/version.json'));assert.equal(meta.installer_enabled,false);assert.equal(meta.project,'Nauclerus');
});

test('outline: the body\'s edge as wall strips, 4 sides and N per corner, following the rounded outline', () => {
  const W = 1000, H = 620, R = 44, N = 6;
  const strips = outline(W, H, R, N);
  assert.equal(strips.length, 4 + 4 * N);
  const len = strips.reduce((a, s) => a + Math.hypot(s.x1 - s.x0, s.y1 - s.y0), 0);
  const perimeter = 2 * (W + H) - 8 * R + 2 * Math.PI * R;
  assert.ok(Math.abs(len - perimeter) / perimeter < 0.01, `${len} vs ${perimeter}`);
  for (let i = 0; i < strips.length; i++) {          // a closed loop: each strip starts where the last one ended
    const a = strips[i], b = strips[(i + 1) % strips.length];
    assert.ok(Math.hypot(a.x1 - b.x0, a.y1 - b.y0) < 1e-6, `gap after strip ${i}`);
  }
  for (const s of strips) assert.ok(s.x0 >= -1e-9 && s.x0 <= W + 1e-9 && s.y0 >= -1e-9 && s.y0 <= H + 1e-9);
});

test('sim.css parses: no comment left open, braces balanced (an open comment once swallowed half the page\'s styles)', () => {
  const css = readFileSync('web/sim/sim.css', 'utf8');
  const bare = css.replace(/\/\*[\s\S]*?\*\//g, '');
  assert.ok(!bare.includes('/*'), 'a comment is never closed');
  assert.equal((bare.match(/{/g) || []).length, (bare.match(/}/g) || []).length, 'braces');
});

test('the power switch is page UI over the stage, a leader line to where the FM-1 has it (the top edge, above the screen\'s right corner)', () => {
  const stage = LANDING.slice(LANDING.indexOf('id="stage"'), LANDING.indexOf('id="play"'));
  assert.match(stage, /<button[^>]*id="power-on"[^>]*role="switch"|<button[^>]*role="switch"[^>]*id="power-on"/, 'a switch in the page');
  assert.match(stage, /id="power-lead"/, 'the leader line');
  const app = readFileSync('web/sim/app.js', 'utf8');
  assert.doesNotMatch(app, /sw\.id = 'power-on'/, 'no longer built into the device');
  assert.doesNotMatch(app, /power-mark|powerMark/, 'nothing on the device: no mark either');
  assert.doesNotMatch(readFileSync('web/sim/sim.css', 'utf8'), /power-mark/);
  for (const [name, L] of Object.entries(LAYOUTS)) {                 // the mark on the device: where the real switch is
    const [x, y, w, h] = L.power;
    assert.equal(y + h / 2, 0, `${name}: centred on the top edge`);
    const bezelRight = L.deco.bezel[0] + L.deco.bezel[2];
    assert.ok(x + w <= bezelRight + 4 && x + w >= bezelRight - 60, `${name}: over the screen's right corner`);
  }
  assert.deepEqual(blend(LAYOUTS.landscape, LAYOUTS.portrait, 1).power, LAYOUTS.portrait.power);
  const css = readFileSync('web/sim/sim.css', 'utf8');
  assert.match(css, /@keyframes nudge/, 'the knob nudges toward on to draw the eye');
});

test('while the simulator is on screen, a cue says the page goes on', () => {
  const stage = LANDING.slice(LANDING.indexOf('id="stage"'), LANDING.indexOf('id="play"'));
  assert.match(stage, /<a [^>]*class="more-cue"[^>]*href="#features"|<a [^>]*href="#features"[^>]*class="more-cue"/);
});

test('Controls opens beside the device on wide screens: the stage gives the panel its width and the device moves left', () => {
  const css = readFileSync('web/sim/sim.css', 'utf8'), app = readFileSync('web/sim/app.js', 'utf8');
  assert.match(css, /\.hero-stage\.help-open \.help\s*{[^}]*right:/, 'the panel on the right');
  assert.match(css, /\.hero-stage\.help-open \.scene\s*{[^}]*padding-right:/, 'the device keeps out of its way');
  assert.match(app, /HELP_W/, 'the device is fitted to the width left beside the panel');
});

test('every chapter shows the FM-1\'s own screen (a 240 x 240 LCD the firmware draws)', () => {
  const sec = LANDING.slice(LANDING.indexOf('id="features"'), LANDING.indexOf('id="release-status"'));
  for (const ch of sec.split('<article').slice(1))
    assert.match(ch, /<figure class="device-screen">\s*<canvas width="240" height="240"/, ch.match(/id="f-(\w+)"/)[1]);
});

test('the lab\'s styles stay in its plates: the power switch\'s knob is styled by the switch alone', () => {
  const css = readFileSync('web/sim/sim.css', 'utf8').replace(/\/\*[\s\S]*?\*\//g, '');
  for (const [, sel] of css.matchAll(/([^{}]+)\{/g))
    for (const part of sel.split(','))
      if (/\.knob\b/.test(part)) assert.match(part.trim(), /^(\.plate|\.power-toggle|@)/, part.trim());
});

test('website palette is centralized and independent of the LCD', () => {
  const css=readFileSync('web/sim/sim.css','utf8');
  for(const [name,color] of Object.entries({bg:'#111318',surface:'#262A30',accent:'#D49349',secondary:'#A6A8AC',text:'#ECE8E0',border:'#41454D'}))assert.ok(css.includes('--'+name+': '+color));
  assert.match(css,/prefers-reduced-motion: reduce/);
  assert.doesNotMatch(css,/#(?:0D191E|173C43|E96F25|6BC4B2|E4DED2|4D777D|9DC2BD|B7C8C2)\b/i);
});

import { parseBdf, buildFont, woff, WANT } from './sim_font.mjs';
test('the type is the firmware\'s: Terminus from its BDF as a web font, every character the page uses', () => {
  const bdf = parseBdf(readFileSync('assets/fonts/ter-u16n.bdf', 'utf8'));
  const tables = buildFont(bdf), font = woff(tables);
  assert.equal(font.subarray(0, 4).toString(), 'wOFF');
  const cps = new Set(bdf.glyphs.map(g => g.cp).filter(WANT));   // what the web font carries
  const page = LANDING.replace(/<[^>]+>/g, ' ').replace(/&[a-z]+;/g, ' ');
  const missing = [...new Set(page)].filter(ch => ch.trim() && !cps.has(ch.codePointAt(0)));
  assert.deepEqual(missing, [], 'characters Terminus lacks');
  assert.doesNotMatch(LANDING, /Archivo|IBM\+Plex/, 'no other typeface');
  assert.match(readFileSync('web/sim/sim.css', 'utf8'), /@font-face\s*{[^}]*terminus\.woff/);
});

// ---- the engines: the page's numbers are the firmware's (dmodels.c N_MODEL, the presets' banks), MD first
const FW = (() => {
  const src = f => readFileSync(f, 'utf8');
  const names = [...src('firmware/src/dmodels.c').match(/N_MODEL\[NMODELS\] = \{([^}]*)\}/)[1].matchAll(/"(\w+)"/g)]
    .map(m => m[1]);
  const classic = names.slice(0, names.indexOf('CLAVE') + 1);   // the enum's first models, K808 .. CLAVE (ui.c MODEL_LIST)
  const trx = names.filter(n => n.endsWith('TRX')), efm = names.filter(n => n.endsWith('EFM'));
  const modern = names.filter(n => !classic.includes(n) && !trx.includes(n) && !efm.includes(n));
  const banks = JSON.parse(src('tools/presets.json'));
  return {
    names, classic, trx, efm, modern, md: [...trx, ...efm],
    factory: banks.reduce((a, b) => a + b.presets.length, 0),
    mdPresets: banks.find(b => b.bank === 'MACHINEDRUM').presets.length,
    user: Number(src('firmware/src/presets.c').match(/#define PRE_NUSER (\d+)u/)[1]),
  };
})();
const chapter = id => {
  const at = LANDING.indexOf(`<article class="chapter" id="f-${id}"`);
  assert.ok(at >= 0, `chapter f-${id}`);
  return LANDING.slice(at, LANDING.indexOf('</article>', at));
};
const words = html => html.replace(/<[^>]+>/g, ' ').replace(/&nbsp;|&#8209;/g, ' ').replace(/\s+/g, ' ');

test('engine/preset counts match current firmware and are generated for the browser', () => {
  assert.deepEqual([FW.names.length,FW.md.length,FW.trx.length,FW.efm.length,FW.modern.length,FW.classic.length],[45,21,13,8,10,14]);
  assert.deepEqual([FW.factory,FW.user,FW.mdPresets],[152,128,55]);
  const v=words(chapter('voices'));assert.match(v,/152 factory presets/);assert.match(v,/45 engines/);
  const ids=JSON.parse(readFileSync('build/sim/lab_ids.json'));assert.equal(ids.presets.factory,FW.factory);assert.equal(ids.models.length,FW.names.length);
  assert.match(words(LANDING),/Machinedrum is a trademark of Elektron; Nauclerus is not affiliated/);
});

test('the examples lead with MD and MODERN engines: no CLASSIC model in a plate (the voices plate lists it last)', () => {
  const classic = new Set(FW.classic);
  for (const f of ['lab_reson', 'lab_comp', 'lab_lfo', 'lab_seq', 'lab_grids', 'lab_fx', 'lab_voices']) {
    const js = readFileSync(`web/sim/${f}.js`, 'utf8').replace(/\/\/.*$/gm, '');
    for (const [, n] of js.matchAll(/DM\.(\w+)/g)) assert.ok(!classic.has(n), `${f}: DM.${n} is CLASSIC`);
    for (const [, n] of js.matchAll(/'(\w+)'/g)) assert.ok(!classic.has(n), `${f}: '${n}' is CLASSIC`);
  }
  assert.match(readFileSync('web/sim/lab_voices.js', 'utf8'), /model: DM\.BDTRX/, 'the voices plate opens on BDTRX');
});

test('the compressor pumps ACID, the 303-style bass line, under an MD kick (BDTRX the key)', () => {
  const js = readFileSync('web/sim/lab_comp.js', 'utf8');
  const kit = js.slice(js.indexOf('const kit'), js.indexOf('const settings'));
  assert.match(kit, /'model', 0, DM\.BDTRX/, 'track 1, the key: BDTRX');
  assert.match(kit, /'model', \w+, DM\.ACID/, 'the ducked voice: ACID');
  assert.match(kit, /'note',/, 'ACID plays a line of notes');
  assert.doesNotMatch(kit, /RIM|CLAVE|CHORD/, 'no rimshot chord any more');
  assert.match(words(chapter('comp')), /ACID/);
});

test('the first-visit demos (tests/sim_demo.c) lead with the MD bank; none plays a CLASSIC voice', () => {
  const src = readFileSync('tests/sim_demo.c', 'utf8');
  const kits = [...src.slice(src.indexOf('SIM_KITS[4]')).matchAll(/\{\{(DM_\w+(?:, DM_\w+)*)\}/g)].slice(0, 4)
    .map(m => m[1].split(', '));
  assert.equal(kits.length, 4);
  const order = readFileSync('firmware/src/dmodels.c', 'utf8')   // the enum's names.match(/enum \{ (DM_K808[^}]*) \}/)[1].split(', ');
  const isClassic = m => order.indexOf(m) <= order.indexOf('DM_CLAVE');
  const isMd = m => /TRX$|EFM$/.test(m);
  assert.ok(kits[0].every(isMd) && kits[1].every(isMd), `demos 1 and 2 are MD kits: ${kits[0]} / ${kits[1]}`);
  for (const k of kits) assert.ok(!k.some(isClassic), `no CLASSIC in a demo: ${k}`);
  assert.ok(kits[3].some(m => m.endsWith('TRX')) && kits[3].some(m => m.endsWith('EFM')), 'demo 4 mixes TRX and EFM');
  assert.ok(kits.some(k => k.includes('DM_ACID')), 'a demo plays ACID');
});

test('no telemetry is enabled and no installer is advertised by the hero', () => {
  const counter=readFileSync('web/sim/counter.js','utf8');
  assert.match(counter,/GC_CODE = ''/);assert.doesNotMatch(LANDING,/gc\.zgo\.at/);
});
