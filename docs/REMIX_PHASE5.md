# ORB Remix Phase 5: SEQ > REMIX and controlled APPLY

## Page and controls

SEQ order: STEP, PATTERN, REMIX, MOTION, SONG. Existing SELECT navigation,
SEQ-button cycling and ALGORITHM track selection remain in use. The page always
edits the selected track T1..T8; no MIDI control or new UI framework is added.

| Encoder | Function |
| --- | --- |
| 1 BANK | A..P, clamped, internal 0..15 |
| 2 PATTERN | 1..16, clamped, internal 0..15 |
| 3 REMIX | Right ON, left OFF |
| 4 APPLY | Right submits the draft; left reverts unapplied edits |

Each track has its own draft selection and ON bit. Intentional dirty drafts
survive page/track changes. A clean draft follows committed/queued settings.
Older OFF projects start with A1 OFF. An OFF draft may retain its bank/pattern
in temporary UI state, but the saved representation remains logical code 0.
Entering/leaving a page never submits APPLY. Project LOAD/initialization changes
a generation counter, clears old queues and invalidates all old UI drafts.

The four columns show draft BANK, PATTERN, REMIX and APPLY state (SET, EDIT,
WAIT or STOP). The graph explicitly separates `Tn SET <committed>`,
`EDIT/DRAFT <selection> ON/OFF`, and `QUEUED <selection>` or Song ownership.
Sixteen bars and staggered 2x numeric labels show the canonical source positions
for chronological positions 1..16 in one block, using production remix_lookup.
Repeated sources remain repeated. OFF, A1 IDENTITY and GRIDS BYPASS are labelled.
All colors come from the existing C_BG/C_INK primitives; the ten themes are
unchanged. Cached signatures avoid playhead-driven graph redraws. Popup damage
uses existing partial windows; idle graph traffic is zero in the focused test.

## Committed versus effective playback

The Phase 4A project configuration and Phase 4B effective ownership remain.
A 16-byte per-track pending array uses bit 9 as a valid flag plus logical code
0..256. An IRQ-guarded main-loop APPLY immediately commits while stopped.
While playing (including a pending PLAY request), it queues the latest code.
Applying the currently committed code cancels an older queued change. Reverting
a draft never cancels an already explicit queued APPLY.

The audio ISR latches a track's queue only after its chronological index advances
and before motion_step/step_fire. Thus current ratchets finish with the old
source, then notes, ACID lookahead, Motion and locks all see the same new source.
No allocation, flash write or metadata decoder runs there. Chronological swing,
divisions, probabilities, lengths and loop counts remain authoritative. This
uses Hortator's existing block scheduling; no new audio timing architecture is
introduced.

For a non-identity STEP mapping change, the old ACID gate and outgoing slide
state close at the boundary. A new TIE cannot borrow a note from the old mapping;
a fresh ON event retriggers normally and subsequent slides use new lookahead.
OFF <-> A1 changes no source and preserves held legato. Grids changes no source
and keeps its existing gates. Existing motion_step releases prior locks before
new source events, preserving Motion-before-Lock precedence and held-value
Motion. A source zero at nonzero chronology does not restore the base.

STOP commits all outstanding explicit APPLY intentions to the live configuration,
then performs the normal gate/lock/Motion restoration. PLAY rebuilds runtime
snapshots. SAVE still obeys STOP TO SAVE and stores committed configuration only;
unapplied drafts are never saved. A successful LOAD replaces old pending work
with the loaded project's settings. FDRF remains 3840 B and its extension stays
unchanged; no new persistence fields or .noinit allocations are introduced.

Armed/running songs block all four controls with STOP TO EDIT. Effective playback
continues to come from the row's prepared project slot; live drafts/configuration
never overwrite it. Song STOP returns to ordinary live-project semantics. Grids
tracks retain their SRC and bypass mapping. Recording/editing remain chronological;
REMIX knobs cannot create parameter locks even with a step held.

## Tests and visuals

`sh tests/run_remix_tests.sh` now includes `tests/remix_ui_test.c` with the real
panel, UI, sequencer, flash/project and boot paths under ASan/UBSan, with no test-only
Remix setter enabled in that test. All focused Phase 1..5 checks pass. The final
preview-label adjustment was also checked by rerunning the production UI test
under the same sanitizers.

Coverage: exact SEQ order and both navigation controls; all eight tracks and all
256 selections; OFF/A1/P16; untouched patterns/banks; independent drafts and queues;
no implicit APPLY; stopped/playing application; one sample before/at the boundary;
latest APPLY and cancel-to-current; STOP flush and real PLAY/STOP buttons; APPLY
before the first audio event; swing/ratchets and repeated-source 1/2 probability;
Motion/lock precedence and chronological restoration; ACID NOTE/TIE/SLIDE and gate
reset; OFF/A1 legato; Grids; armed/running Song restrictions and return to live;
SAVE/LOAD, power-on, legacy OFF and clearing stale LOAD queues/drafts.

Existing UI tests retain all brightness and musical assertions. Their obsolete
four-SEQ-page count is updated to five (12 cases instead of 10), and REMIX is
added to the every-SEQ-page step-key matrix. One existing project test copies the
now-volatile committed array with element reads; its assertion is unchanged.
No golden references, thresholds, palette values or integrity guards change.

80 actual 240x240 framebuffer captures cover OFF, identity, active non-identity,
draft, queued, Grids, Song and T8 in all ten original schemes. Pixel checks verify
palette-only colors, redraw suppression and equality with a forced repaint.
See `build/remix_ui/states.png`, `build/remix_ui/themes.png` and the individual
`theme-NN-STATE.png` / `.ppm` files. These are host screenshots, not mockups.

The same ordinary OFF fixture compiled against the pre-Phase-5 backup and this
tree has identical stereo PCM/state fingerprints (two ACID tracks, six drum
tracks including Grids, Motion/locks, probability, ratchets, swing and EQ):

| Length | EQ | Audio FNV64 | State FNV64 |
| ---: | ---: | --- | --- |
| 1 | 0 | 1d90cb5ef56ab9a1 | 67457523d4fad14b |
| 8 | 1 | 4ff9f7afff7246c9 | 9433fcdad8f84ab9 |
| 16 | 2 | 5310def28ef4dfab | 0c0708dd817ce749 |
| 24 | 0 | 474b02155a86c34f | 738666ab40db9ba2 |
| 40 | 1 | 19e17c7d9fd65f5b | b64ddb291973ef7c |
| 64 | 2 | 21c463e0bfeb3ebb | b7351242666e0f2c |

The pre/post Song OFF fixture also matches in all four drum/ACID x looping cases
(the fingerprints documented in Phase 4B). No reference files were regenerated.

## Files and preservation

Production files changed: firmware/src/pages.c, ui.c, ui_input.c, ui_draw.c,
seq.c and project.c. Tests changed: tests/ui_test.c, remix_project_test.c and
run_remix_tests.sh. New files: tests/remix_ui_test.c and this document.
The source backup is `/tmp/nauclerus-before-phase5-20261027-112850` (complete
tracked/untracked archive, patches, index copy, hashes and reference sources).
All 22 original intent-to-add entries remain; the two new files also use
intent-to-add for Gate 5 (24 total). No content is staged. Prior local work is
preserved; no reset, stash, branch switch, commit or push is performed.

## Build and remaining validation

The final documented default build succeeds without warnings. The full host
runner was repeated with code/test sources held fixed. Subsequent UI
refinements (preview spacing, reading the queue once and an IRQ-guarded Song
edit check) were checked by repeating the complete focused UI test and unchanged
Gate 5; playback logic stayed fixed. Gate 5 passes:
normal/no-Bluetooth default images are byte-identical, 344012 B.
An initial run compared two preview-label revisions because the visual edit
raced its source snapshot; inspecting its retained copy isolated exactly that
difference. The final stable-tree full run resolves it without changing Gate 5.

`sh tests/run_tests.sh` returns 1 only at the existing missing frozen-reference
guard. All executed functional, existing sanitizer, UI, Motion, Lock, ACID, Song,
boot, fidelity, protocol, target-budget and web checks pass. Fidelity covers all
six drum models, 316 Grids cases, 216 compressor cases with three signals and
144 DaisySP cases (36 informational). The full focused Remix runner and final
UI test pass with ASan/UBSan; no sanitizer category is suppressed. Additional
production-code checks for held-step nondestructive controls, popup partial
restoration and editing a new draft while an older APPLY commits also pass.

| Resource | Used B | Capacity B | Free B | Phase 4C delta B |
| --- | ---: | ---: | ---: | ---: |
| Application flash | 344012 | 581564 | 237552 | +2240 |
| Static RAM (.data + .bss) | 73332 | 98304 | 24972 | +40 |
| Pool | 307316 | 344064 | 36748 | 0 |
| .noinit | 15576 | 15696 | 120 | 0 |

The pending array (16 B), project epoch (4 B) and UI draft state/alignment
account for the 40 B ordinary RAM increase. The ELF keeps project slots at
15360 B in .noinit and preserves all project member offsets/3840 B assertions.
The 8192 B pool reserve remains enforced. Stack analysis passes the unchanged
75% limits: main 2264/24320 B, interrupts 1480/7936 B. Target render budgets pass;
the audio ISR model cost is 10413 versus 9769 before Phase 5 (+644), below the
unchanged 15188 budget. This is a static instruction-cost model, not a measured
hardware CPU percentage.

The missing frozen-base-10 reference, pinned to
731386008fae060ae885f44b4765d172ecfae4b3, remains an outstanding release-validation
blocker. No tag, pin or guard is substituted. Existing explicit skips remain:

- Thirteen host instruction-budget checks in six groups: comparable instruction
  counting is unavailable on Linux. Functional assertions still execute.
- H2/H3 upstream comparisons: no upstream build. Standalone stack analysis and
  the H3 self-test pass.
- Installer logical-image and web package comparisons: legacy
  build/felucca.fwsc filename absent; the documented local regression package is
  build/felucca-UNTESTED.fwsc. Loader/package protocol simulations execute.
- Official V15 restoration: FM-1_v15.fwsc is unavailable.

Before physical use: validate encoder feel, 240x240 LCD readability, real SPI
redraw latency/audio load and audible boundary transitions on hardware. No
hardware flashing or test is performed here. No UI for MIDI Remix, TB-3PO,
pattern baking, theme change, new recording engine or SEL Global-menu navigation
is added. No dependency installation, release or publication occurs.
