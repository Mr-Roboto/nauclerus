# Nauclerus 0.25.0 release validation

The owner explicitly authorized stable publication of the accepted RC13 implementation.
Original development checkout, staged intent-to-add entries and local main are preserved;
release preparation uses an isolated release/0.25.0 checkout. No new musical feature,
DSP, filter coefficient, preset, default or storage change is made. The separately
approved NAU-0250-015 identity changes are included.
The approved application/package identity is FM-1_25000 and the USB product is Nauclerus. The shared loader identity ota-FM-1_900 and its checksum remain unchanged.

The consolidated [failure inventory](RELEASE_FAILURE_INVENTORY_0.25.0.md) records every
collected assertion and its cause, including the exact64 header pixels in the adjacent
JSON. Production fixes are confined to menu-header refresh, same-frame modal
Learn/effect cancellation, and preserving held-step accents on REMIX. Audio sources
and presets are unchanged. The independent [target reference](RELEASE_TARGET_COST_0.25.0.md)
reproduces the accepted RC13 application byte-exact and all53 target metrics match.

Earlier audio/DEST test corrections are recorded in
[ACID_GOLDEN_0.25.0.md](ACID_GOLDEN_0.25.0.md): DEST maximum19 and all names/IDs are
checked; the sole ACID golden changes to5d5973a5 after byte-identical RC11/12/13 renders.
No other audio golden changes; the two independently proven stale target-cost
references are documented above. Test tolerances and security restrictions are unchanged. Source/PCM/package comparison evidence is
in the adjacent JSON. RC13 BIP/UNI focused tests remain required.

Required checks: complete host runner, Remix production tests, DSP tests, RC13
routing/UI/storage, browser simulator/manual, installer mock protocol and corrupt
package rejection, immutable security baseline/negative fixtures, reference receipt,
H2/H3, target budgets, stack/memory, and Gate5. No skipped required security/fidelity
gate is accepted as PASS. Linux retired-instruction counters are unavailable and
are reported as SKIPPED; target static costs and the prior physical acceptance are
separate evidence, not a claim of hardware cycle certification.

The release source archive must match tag v0.25.0, contain applicable licenses and
exclude the five unverified optional documentation images. Originals remain local.
The baseline annotation nauclerus-security-base-1 and its exact pin must accompany
the release for source checks to work in a new clone. No frozen-base-10 is invented.

The website preserves the approved Graphite/Champagne/Amber theme and real LCD
palettes. Browser installation uses the unchanged fm1ota.js protocol; its wrapper
verifies exact stable package size/SHA-256/product before enabling Install, requires
backup/recovery acknowledgement and explicit confirmation, and requests WebMIDI only
from the user action. No hardware is contacted during automated validation.

Publication receipts, source/app/package/loader hashes, test logs and public URL
verification accompany the GitHub release. The accepted subjective resonance/filter
character limitation remains. Release checks do not substitute for the user's
recovery prerequisites: reliable power/data cable, project backup, browser MIDI SysEx
support and a tested recovery path with separately obtained stock artifacts.

## Final local execution — PASS

All mandatory executable gates pass: complete host release runner, all Remix
production tests, ASan/UBSan DSP checks, 230 isolated UI functions, RC9 UI,
RC10 configuration/PWM/LFO/storage/audio/dense mix, RC13 arithmetic/routing/UI/
storage and bit-identical BIP fingerprints, TB-3PO phrase/scale/parity, all53
target estimates, H2/H3, stack/memory, 163 production security fixtures (including the narrow USB exception negatives),
loader/tamper/recovery mocks, firmware/package corruption checks and unchanged
Gate5. The current source builds to WASM; native simulator checks, 35 glue tests,
30 site tests, 29 desktop/mobile browser checks and8 fail-closed installer browser
checks pass. All39 retained asset/notice/SDK hash records match. The banner is the owner's
original and the screenshot is a genuine RC13 LCD framebuffer. The five unverified
historic images are absent from release staging.

Six Linux-only retired-instruction measurements are explicitly SKIPPED: LFO CPU,
kit CPU, boot CPU, TRACKS frame CPU, piano-roll frame CPU and rendered-hit CPU.
Comparable macOS counters are unavailable here. Functional/audio assertions still
run; static target budgets, dense host measurements and prior owner hardware
acceptance remain separate evidence. The optional stock V15 restore-artifact check is also SKIPPED: the separately
licensed vendor image is unavailable locally. Recovery protocol mocks run normally.
No actual FM-1 hardware was accessed.

The firmware source differs from preserved RC13 in ui_input.c and ui_menu.c for
the documented release UI corrections, and in felucca.c and usb_app.c for the
approved application fallback ID and USB product descriptor. Build definitions
now supply FM-1_25000. ACID/DSP, preset, timing, project/Song and loader sources
remain byte-identical.
Final artifact: 358292-byte application; RAM footprint74456B; pool317556B with
8192B required reserve unchanged; .noinit15576B. Package609649B, FM-1_25000.
SHA-256: b496e081fd78c05eaa3cd368caf384285ec7b708477c5ff638f373e6a3c2a870.
Public deployment/source-archive receipts accompany the release after publication.

Identity migration and compatibility: [NAU-0250-015](IDENTITY_MIGRATION_0.25.0.md).
