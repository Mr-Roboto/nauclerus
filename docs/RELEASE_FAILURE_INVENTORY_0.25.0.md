# Nauclerus 0.25.0 consolidated release failure inventory

The entire host test inventory was collected independently, including every executable after the first failure. UI functions were also isolated into230 independent processes so crashes or loops could not hide later assertions. Original logs and numeric probes are retained in build/release-inventory and build/release-checks; final execution receipts accompany the release.

| Failure group | Expected | Observed | Cause and correction |
|---|---|---|---|
| LCD cached redraw | 0 differing pixels | 64 pixels, x=6..13, y=3..12; cached PLAY triangle versus STOP square | Firmware defect: draw_menu() returned on its body signature before refreshing draw_head(). Refresh the independently cached header on the body-cache fast path. The 64 coordinates/RGB565 pairs are preserved in the validation evidence; no pixel tolerance was added. |
| MIDI Learn menu entry | MENU=1, LEARN=0, ml_arm=0; MIDI-held effects=0 | Old HOME-hold fixture entered Settings; with corrected GLO tap, Learn and MIDI holds persisted one additional UI frame | Old input fixture plus genuine one-frame modal-entry defect. Use GLO tap in tests; immediately invoke existing ml_guard() and set existing perf_kill on menu entry. Existing audio-block cleanup releases MIDI-held effects. |
| REMIX held-step accent | OCT+ plus step accents that step; bare OCT+ confirms | OCT+ was consumed by confirmation before step-accent dispatch | Firmware dispatch defect. Reuse acc_oct for step contexts; release alone calls the same remix_confirm(), while a step press consumes the accent without confirming a pending draft. K4 and non-step contexts retain their commit handler. |
| Orange theme fixture | Approved theme0 RGB565 EB64/3081 | Fixture still expected old MD RED RGB(224,71,43)/RGB(56,16,10) | Stale palette assertion. Check exact accepted Orange values. Firmware palettes and IDs unchanged. |
| Factory bank count | 152 total; MD55, MODERN41, CLASSIC56; CLASSIC starts96 | Old expectations 151, MODERN40, CLASSIC95 | FASTSLOW UID152 was intentionally added before SQUELCH. Correct exact bank counts, ACID count5 (others <=4), skip boundary +5, and old-fixture cardinality151+1. Every original UID/name/model mapping still checked; no preset data changed. |
| Legacy FDRE KEYF | Real pre-KEYF FDRE, all original reserved step bytes zero; loaded R.KEYF0; identical audio hashes | Test wrote current FDRF, whose reserved step bytes encode ACID MORPH; audio hashes already both918404854 | Stale fixture construction, not a playback defect. Serialize the actual older FDRE structure with zero extension bytes, use real conversion, then keep original KEYF and audio-equivalence assertions. |
| FDRF reserved metadata | Unowned bytes zero; valid TB-3PO header/checksum; unchanged3840B and lock bank | Old assertion required all510 reserved bytes zero; actual NAUTB3PO extension occupies48..159; bank equality1 and lock count5 | Stale pre-generator assertion. Require zero outside existing TB-3PO range, exact header and checksum, plus original bank, size and project checks. |
| UI MUTE blink sampling | Opposite250ms phases, LED and screen agree | LED samples0/0 and boxes0/0 with queue1/play1; host getter advances1ms on each read | Harness phase error: 250ms jump plus getter advances can cross two blink edges. Align monotonically to the next actual250ms phase before each sample. No firmware timing or threshold changed. |
| Theme screenshot exporter | 12 theme directories | Only10 directories were created; fopen for appended RED failed, exporter crashed | Harness setup defect. Create red/red-inv directories and retain full two-color pixel assertions in all12 schemes. |
| ABOUT pixel search | Glyph pixels exact in credit region y>=78 | Search also required blank pixels at y76..77, which belong to the independently drawn VU logo | Stale test ownership/margin assertion. Clip the comparison to credit ownership at y78 while retaining exact glyphs, clear margins within the credit region, all original credits, scroll and logo independence. No ABOUT layout changed. |
| Remix scope tests | One Drum scope plus eight retained Bass scopes; runtime uses current model | Older tests assumed eight independent Drum selections and v1-only metadata | Update fixtures/oracles to RC2 scope ownership and v2 18-byte encoding. Preserve all256 selections, legacy-v1 tests, invalid/checksum tests, independent Bass, OFF-equivalence and real production paths. |
| Song prepared steps | Compare lookahead with chain.src[slot].step[track] and correct effective scope | Tests required pointers into proj_slot directly and per-track Drum scope | TB-3PO intentionally prepares independent mutable Song step buffers. Verify prepared bytes match saved data when auto-mutation OFF, use prepared-buffer pointers, global Drum scope and retained live configuration. No Song implementation changed. |
| Grids quick-assignment fixture | Select a Drum track to access the actual GRIDS page | Fixture selected ACID, so ARP correctly routed to TB-3PO | Stale pre-TB-3PO page setup. Select another physical Drum track, retain ACID among assignment targets, verify only T1-3 SRC fields change. |
| RC9 WAVE knob fixture | From canonical PULSE1, next visible choice is MORPH3 | Old assertion expected2 (RC9 fixed25% PULSE identity) | Stale pre-RC10 visible-order assertion. Keep legacy numeric identities unchanged and check3. RC10/RC13 waveform/storage tests remain independent. |
| Other HOME/GLO UI fixtures | GLO tap MENU; HOME hold Settings; Settings HOME short cycles/hold exits | Older tests entered MENU via HOME hold or GLOBAL via GLO tap | Only obsolete test input sequences changed. HOME escape correctly ends Learn; re-enter Learn explicitly when testing TRACKS knob assignment. Non-ACID layouts retain old order; ACID checks exact approved15-parameter order. |
| Analytics test | Nauclerus default GC_CODE empty; no upstream analytics account | Test required deadactive account and script injection by default | Stale website expectation. Verify disabled default and test opt-in counter behavior using explicit synthetic code. No website analytics service enabled. |
| Package scope-size integrity | Target seq_remix_config exactly18B/9 scopes | Checker expected16B from Phase1 | Stale structural guard; set exact expected18, retain package/CRC/product/loader/memory checks and corruption rejection. |
| Security fixtures in worktree | Use actual Git object directory | Test assumed .git/objects although worktree .git is a file | Harness path fix via git rev-parse --git-path objects; all156 real negative guard fixtures retained. |
| Earlier LFO DEST maximum | 19; all20 destination mappings/names | Obsolete maximum18 | Test-only correction for existing A.MOR destination, with explicit all-ID/name verification. |
| Earlier ACID golden and self-oscillation | Accepted RC11/12/13 output5d5973a5; autonomous magnitude>1000 | Older golden8145d3ce and driven-band625.746936 | See ACID_GOLDEN_0.25.0.md/JSON. Three retained accepted snapshots produce identical PCM and autonomous1660.082153 versus zero non-resonant AC band; original >1000 criterion retained, stability/boundedness checks added. No DSP/default/preset changes. |

## Every collected failed assertion

The following are the unfiltered assertion descriptions from the independent inventory, including cascades caused by obsolete navigation fixtures. A nonzero status is never treated as PASS.

- themes: MD RED #e0472b / #38100a
- random use (seed 23757): the screen = a full repaint's (64 px differ, rows 3..12)
- the menu opens: LEARN off
- REC held on SEQ while learning: REC picked (no CLEAR TRACK?)
- another confirm (MOTION's CLEAR MOTION T1?): LEARN off
- HOME's TRACKS mode: LEVEL
- TRACKS: PAN
- TRACKS: LEN
- the menu opens: the MIDI-held effect ends
- the menu closed: it does not come back (the controller still down)
- banks: MACHINEDRUM 55, MODERN 40, CLASSIC 56, then USER 128; a bad id: none
- every preset: a name of 1..8 characters, unique; its knobs in its model's ranges; in its voice's bank; <= 4 per voice
- ACID on 2 other tracks: its presets skipped
- every 0.22.0 factory preset's uid: the same preset (name, voice) in the new banks
- project: an FDRE record from before KEYF loads with KEYF off, playing as before
- FDRF: motion, locks and PLAY bits through a save and a load (the project record 3840 B, 510 B reserved 0)

## Scope and release criteria

The only production changes after accepted RC13 are modal UI/header fixes in ui_menu.c and ui_input.c. No ACID DSP/filter, presets, defaults, IDs, project/Song records, loader, flash map, USB identity or musical timing is changed. Full release checks, simulator/browser tests, negative security tests, package checks, H2/H3, target/stack/memory and Gate5 must pass before publication. Linux host retired-instruction measurement remains explicitly SKIPPED; static target checks and physical acceptance are distinct evidence.

### Earlier complete-run assertion descriptions

- PERFORM layer: GLO release opens MENU
- every page of PAGES[] is reached with its button
- HOME held: the menu draws
- menu > ABOUT draws
- ABOUT: the VU mark on top (half size), moving
- CLK USB: SELECT on HOME's TRACKS mode keeps the tempo and says CLK USB
- the menu opened with an effect key held: effects off, the key still the layer's
- PAGE: another page button closes it, the macros stay on
- PAGE: an FX tap closes it and opens the FX pages
- HOLD: FX held into the menu: the CRUSH macro stays on there
- HOLD: FX held out of the menu: dead (no map, keys not the layer's)
- PAGE: FX held into the menu: the CRUSH macro stays on there
- PAGE: FX held out of the menu: dead (no PERFORM page, keys not the layer's)
- FX dead (held through the menu), its layer key let go: no PERFORM screen over the page the knobs edit
- REMIX: OCT+ alone is the bank; OCT+ held + a key accents the step
- step record ends: the menu opened (REC dark under it)
- PRIO: every model's SOUND list ends CHOKE, PRIO
- BACKLIGHT ON, every page and the HOME screen, KEYS off / on, armed or not: where the keys are steps (every SEQ page, TRACKS unarmed) on steps full, the unlit beats 1 5 9 13 at 25 % in bank 1 and bank 2, the rest 5 %
- buttons open their page families (EDIT SOUND, FX, SEQ, GLO, SAVE, ARP GRIDS, LFO LFO)
- ENV (STEPS, 2026-10-24) then SCL: KEYS on (STEPS off), the page stays
- EDIT again: the second SOUND page
- HOME held on TRACKS: the menu
- the menu closed: HOME in the mode last shown (TRACKS)
- GLO: a REC tap: REC ALL, no play, the screen stays, REC lit
- REC in the menu: nothing
- GLO tapped: GLOBAL 1/3 on the release, no MUTE screen
- GLO taps go on: SYSTEM 2/3, GLOBAL 3/3
- GLO held 0.1 s alone: a tap, GLOBAL
- PERFORM layer: GLO a plain button there (its release opens GLOBAL)
- all let go: neither, no page change (not a tap)
- GLO reaches GLOBAL 3/3 with MUTE
- GLOBAL 3/3: KNOB 1 -> BAR (the setting)
- GLOBAL 3/3: KNOB 2 is METRO, OFF
- METRO values: OFF LOW MID HIGH
- GLOBAL 3/3: KNOB 2 -> MID (the setting; the knob shows it)
- KNOB 2 up: HIGH at the end

## Final aggregate-run target-reference finding

The complete aggregate runner continued after the drum suite and exposed two over-budget results: chain_apply cost202 versus expected174, and acid_render540 versus326. The independent RC13 application reproduced byte-exact; all53 target metrics match accepted RC13 and stable. Only those two obsolete expected values are rebased; +10% and all other entries remain unchanged. [Full independent evidence](RELEASE_TARGET_COST_0.25.0.md).
