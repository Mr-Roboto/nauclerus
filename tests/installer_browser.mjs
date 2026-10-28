// SPDX-License-Identifier: GPL-3.0-only
// Verified-package UI tests; MIDI is replaced before any page script executes.
import {createRequire} from 'node:module';
import {readFileSync} from 'node:fs';
import {createHash} from 'node:crypto';
import assert from 'node:assert/strict';
const require=createRequire(import.meta.url);
const {chromium}=require(process.env.NAUCLERUS_PLAYWRIGHT||'playwright');
const base=process.env.NAUCLERUS_PORTAL_URL||'http://127.0.0.1:8766/nauclerus/';
const version=readFileSync('VERSION.txt','utf8').trim();
const bytes=readFileSync(`build/nauclerus-${version}.fwsc`),sha=createHash('sha256').update(bytes).digest('hex');
const browser=await chromium.launch({headless:true});let checks=0;
const check=(name,ok)=>{assert.ok(ok,name);checks++;console.log('PASS',name);};
try {
 const context=await browser.newContext();
 await context.addInitScript(()=>{
  window.__midiCalls=0;
  Object.defineProperty(navigator,'requestMIDIAccess',{configurable:true,value:async()=>{window.__midiCalls++;throw Error('Test deliberately denies all hardware access');}});
 });
 const page=await context.newPage();
 const meta=await(await page.request.get(base+'firmware.json')).json();
 check('Metadata binds exact stable package/product/size/SHA-256',meta.product==='FM-1_25000'&&meta.bytes===bytes.length&&meta.sha256===sha&&meta.pkg.endsWith(`/nauclerus-${version}.fwsc`));
 const delivered=await(await page.request.get(new URL(meta.pkg,new URL('webapp/installer/',base)).href)).body();
 check('Installer serves byte-exact validated firmware',delivered.equals(bytes));
 await page.goto(base+'webapp/installer/');
 await page.waitForFunction(()=>document.querySelector('#status').textContent.includes('Nauclerus'));
 check('Loading verifies artifact without MIDI or enabling Install',await page.evaluate(()=>window.__midiCalls===0&&document.querySelector('#go').disabled));
 await page.locator('#ack').check();
 check('Explicit recovery/backup acknowledgement enables verified Install',await page.locator('#go').isEnabled());
 page.once('dialog',d=>d.dismiss());await page.locator('#go').click();
 check('Cancelled confirmation never requests MIDI',await page.evaluate(()=>window.__midiCalls===0));
 await page.locator('#ack').uncheck();
 check('Removing acknowledgement disables Install',await page.locator('#go').isDisabled());
 const bad=await context.newPage(),corrupt=Buffer.from(bytes);corrupt[2048]^=1;
 await bad.route(`**/firmware/nauclerus-${version}.fwsc`,r=>r.fulfill({status:200,contentType:'application/octet-stream',body:corrupt}));
 await bad.goto(base+'webapp/installer/');
 await bad.waitForFunction(()=>document.querySelector('#status').textContent.includes('SHA-256/size mismatch'));
 await bad.locator('#ack').check();
 check('Tampered package fails closed even with acknowledgement',await bad.evaluate(()=>document.querySelector('#go').disabled&&window.__midiCalls===0));
 await page.setViewportSize({width:390,height:844});
 check('Installer fits a mobile viewport',await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1));
 await context.close();console.log(`installer browser: ${checks} PASS, 0 FAIL; no hardware access`);
} finally {await browser.close();}
