# Nauclerus 0.25.0 user guide

Canonical project: **Nauclerus** — [repository](https://github.com/Mr-Roboto/nauclerus),
[releases/downloads](https://github.com/Mr-Roboto/nauclerus/releases).
Release artifacts and validation are recorded in [release readiness](RELEASE_READINESS_0.25.0.md).

This guide describes **Nauclerus 0.25.0**, the accepted RC13 implementation. Earlier phase documents describe historical development contracts;
this guide takes precedence for current controls.

## Panel navigation

ALGORITHM selects the physical track T1–T8. SELECT turns pages in the current family
and moves the MENU cursor; it is a rotary encoder, distinct from the SEL/SCL button.
On TRACKS, SELECT adjusts tempo instead. Repeated page-button taps cycle that family.
K1–K4 edit the four columns shown on the screen. Modal dialogs take priority over
normal page shortcuts.

| Control | Current action |
| --- | --- |
| HOME short | On HOME/TRACKS, toggles those views. From ordinary parameter pages, returns to the last-used HOME/TRACKS view. From MENU and normal temporary overlays, closes/cancels them and goes directly to the real HOME scope screen. Settings pages are the exception below. |
| HOME long | Opens the Settings family. Already in Settings: exits directly to real HOME. Releasing a long press does not generate a short press. |
| HOME short in Settings | Cycles GLOBAL 1 → SYSTEM → GLOBAL 2 → GLOBAL 1. The screen's family numbering includes SYSTEM as page 2/3. |
| GLO short | Opens/closes the system MENU. |
| GLO held | MUTE overlay. GLO plus a track key reaches its mute controls; existing combinations remain active. |
| EDIT | SOUND/EDIT pages for the selected model. Hold for the PRESETS list. PRESETS encoder browses/auditions presets. |
| SEQ | STEP → PATTERN → REMIX → MOTION → SONG. |
| ARP | Drum/percussion selection: two GRIDS pages. ACID selection: GENERATE and SCALE, the two TB-3PO pages. |
| LFO | Three paired-LFO pages, then DEST. |
| SAVE | PROJECT, then TOOLS. |
| SEL/SCL | Toggles KEYS outside modal contexts. In MENU, ENTER for ABOUT/HARDWARE CALIBRATION (and BLUETOOTH in a BLE build) only. |
| ENV | Toggles STEPS on release: white keys edit steps away from SEQ too. ENV and KEYS toggles normally turn each other off. Hold ENV plus OCT−/+ for step banks. |
| PLAY | Starts/stops. On SONG, starts the prepared chain; STOP ends it. REC does not start transport. |

GLO MENU uses SELECT to navigate, K1 to edit scalar settings, OCT+ for OK/ENTER and
OCT− for BACK. SEL/SCL also enters dedicated screens; it does not toggle KEYS,
change COLOR, or toggle scalar settings there. ABOUT scrolls with SELECT. HOME
short cancels an overlay's pending action rather than executing its highlighted item.

Settings pages contain:

| Page | K1 | K2 | K3 | K4 |
| --- | --- | --- | --- | --- |
| GLOBAL 1 | BPM | SWING | CLOCK | drum channel |
| SYSTEM | MIDI | SYNC | ROUTE | INFO |
| GLOBAL 2 | MUTE timing | METRO | — | — |

The separate MENU contains COLOR, SPEAKER EQ, USB LEVEL, PERFORM, ZOOM, ACCEL,
BACKLIGHT, HARDWARE CALIBRATION and ABOUT. BLUETOOTH is conditional on a separate
BLE build; its presence in source does not mean the default package enables it.

## ACID sound engine

Up to **two** physical tracks can use ACID. There are no fixed bass-track positions.
Select MODEL on EDIT to assign it; a third assignment is rejected. ACID is a mono
voice with note gates, accents, ties, slides and a resonant Q24 four-pole ladder.
Its CUTOF/RESO controls are separate from the track FILTER effect under FX.

### EDIT layout

| Page | K1 | K2 | K3 | K4 |
| --- | --- | --- | --- | --- |
| 1 | MODEL | WAVE | CUTOF | RESO |
| 2 | TUNE | DECAY | ENVMD | LEVEL (LVL) |
| 3 | GATE | ACCNT | PAN | MORPH |
| 4 | CHOKE | PRIO | NOTE | empty/inactive |

FASTSLOW is the first ACID factory preset in MODERN, before SQUELCH, UID 152:
TUNE −12, DECAY 50, CUTOF 80, RESO 30, ENVMD 60, ACCNT 64, WAVE PULSE, GATE 40.
A newly initialized ACID sound starts with MORPH 64 and the untouched generic track
level 104 (−4 dB). User-adjusted and restored project levels are not reset; browsing
ACID presets does not reset LEVEL. TUNE transposes the sound without rewriting notes.
The eight-knob preset format does not store MORPH independently: Pulse presets
establish their width default, while projects retain the additional track value.
Cancelling preset audition restores the previous manual MORPH value.

### WAVE and MORPH

| Visible WAVE | MORPH behavior |
| --- | --- |
| SAW | Disabled/dimmed. Turning MORPH changes nothing and creates no edit mark, lock or Motion event. Its stored value is retained. A.MOR modulation is inaudible. |
| PULSE | Variable width: 0 = 5%, 64 = 50%, 127 = 95%, monotonic between them. Both discontinuities are band-limited; width changes are smoothed over approximately 5 ms. |
| MORPH | Continuous Sharkfin at 0 → Trapezoid at 64 → Notch-Trapezoid at 127, adapted from X0X-RMX v1.1.0. |

Only one ACID track may own the MORPH waveform at a time. A conflicting selection
says `MORPH IN USE` and retains the requesting track's previous waveform. Loading
conflicting projects resolves ownership by physical track order. Both tracks may
use SAW/PULSE independently. Extreme narrow PWM naturally changes harmonic balance;
its bass fundamental is not identical to a 50% pulse.

Stored WAVE IDs are deliberately stable: 0 SAW, 1 legacy SQUARE, 2 legacy fixed-width
PULSE, 3 MORPH. IDs 1/2 both display PULSE; encoder selection exposes only three choices.
Old SQUARE loads at MORPH 64, preserving the oscillator's 50% output. Old fixed-25%
PULSE loads at MORPH 28 (approximately 24.69%, the nearest available integer setting).
Newly saved PWM amounts are retained. Legacy MORPH never becomes PULSE.

ACCNT controls accent strength; accented notes also affect filter-envelope behavior.
SLIDE belongs to the outgoing step and carries into the next audible note; TIE
extends a held sequencer note and does not start a note after a rest. TIE ignores
its own PROB/RATCH/ACC fields. GATE controls normal note duration. STOP/restart and
mapping changes use the existing gate/slide cleanup; there is a roughly 5.5 s lifetime
backstop on a continuously held ACID note/chain.

### Accepted sound limitation

The owner reports RC11 hardware acceptance with **10 of 11 checks PASS** and accepts
the remaining subjective resonance/filter character concern for 0.25.0. The ACID
engine is not claimed to sound identical to X0X-RMX or an analogue TB-303.
RC10 introduced a 150 Hz high-pass in the resonance-feedback path to improve bass
retention, without increasing global ACID gain. The earlier RC9 report of very weak
high-MORPH output was not reproduced in controlled offline renders; those renders
are not proof of a hardware-specific root cause. No additional DSP change is part
of this release-documentation work.

## TB-3PO melody generator

Select an ACID track and press ARP. Repeated ARP taps alternate:

| Page | K1 | K2 | K3 | K4 |
| --- | --- | --- | --- | --- |
| GENERATE | DENS 0–100% | ACCENT 0–100% | SLIDE 0–100% | OCTS 1–3 |
| SCALE | ROOT C–B | SCALE | OCTAVE 0–4 | MUTATE OFF/1–16 |

Defaults: DENS 70, ACCENT 40, SLIDE 25, OCTS 2, ROOT A, SCALE MPEN, OCTAVE 1, MUTATE OFF.
Each physical track retains its settings and seed, including when switched to a
drum model and back. Two ACID tracks can generate independently anywhere in T1–T8.
Generator OCTAVE is the stored note register, independent of sound TUNE; the root
base note is `12 × (OCTAVE + 1) + ROOT` before sound transposition.

The 22 scales, in order, are MPEN, PMAJ, EGYP, HIRA, MIN, MAJ, DOR, PHRY, LYD, MIXO,
LOCR, HARM, MELM, HMAJ, ALTR, GYPS, ROMN, BHAI, MARV, PURV, TODI, ARAB.

- **OCT+ NEW LINE:** replaces the target's step line using current settings and a
  new seed; says `NEW LINE`.
- **OCT− MUTATE:** restrained changes to the existing phrase, retaining rhythmic
  and melodic anchors; says `MUTATED` or `NO CHANGE`.
- Parameter changes prepare the next operation, not immediate note edits. The
  `*` stale indicator marks changed generation settings until NEW/MUTATE.
- NEW/MUTATE work while stopped or playing. Playing edits are prepared separately
  and copied at the next chronological step boundary (`QUEUED - NEXT STEP`), not
  exposed half-written to playback. They retain LEN/DIV and set SRC to STEP.
- Ordinary OCT presses reach the generator with KEYS or ENV/STEPS enabled.
  Deliberate held-STEP edits and ENV-held bank navigation still have priority;
  menu, confirmation and MIDI LEARN contexts retain theirs.

The core comes from the pinned X0X-RMX v1.1.0 implementation: motifs, related phrases,
root/fifth preference, rests, accents, nearby-note slides and restrained mutation.
DENS 0 keeps one root note;100 fills every step. ACCENT 0/100 gives none/all generated
notes; SLIDE 0 disables slides, 100 slides eligible adjacent sounding notes. One-step
patterns do not slide into themselves. New plain events clear stale articulation,
probability and ratchet fields. Mutation changes only affected events and does not
requantize every hand-edited note. NEW also clears unused positions beyond LEN;
MUTATE operates within the active length. Save first if you need the previous line.
Motion/lock bank contents and Remix settings are not erased by these operations.

**Automatic MUTATE counts completed track-pattern loops**, despite the source
parameter's “bars” terminology. Thus 1 means every pattern wrap; for a 16-step 1/16
pattern this is one 4/4 bar, but other lengths/divisions differ. OFF leaves the
line unchanged. Runtime counters/random streams reset at transport start. During
Song playback, mutation uses slot-owned RAM step copies; it does not rewrite saved
project records. Generator controls are blocked while a Song is armed/running.

## ORB Remix

SEQ order is STEP → PATTERN → **REMIX** → MOTION → SONG. The 256 owner-created mappings
are banks A–P, patterns 1–16; A1 is identity, and repeated source selections are
intentional. The 16-position preview shows the draft's source positions.

**All drum/percussion tracks share GLOBAL DRUM.** Each physical ACID track instead
uses its independent BASS setting. Selecting any drum track edits the same global
configuration; selecting an ACID track edits only that track. Changing a model
switches the effective source automatically without erasing its retained Bass
setting. Grids-sourced tracks bypass Remix until SRC becomes STEP.

| Control on REMIX | Action |
| --- | --- |
| K1 BANK / K2 PATTERN | Edit the draft only. |
| K3 REMIX | Enable/disable the confirmed selection directly; does not confirm the draft. |
| K4 SET clockwise or OCT+ | Same confirmation action for draft Bank/Pattern. |

Stopped confirmation is immediate. Playing confirmation and ON/OFF changes take
effect at the next chronological step boundary of each affected track; different
drum divisions can reach that boundary at different times. `DRAFT`, `SELECTED`,
`QUEUED`, `READY`, `OFF - PREVIEW ONLY`, `A1 IDENTITY` and `GRIDS BYPASS` distinguish
states. OCT+ breathes for an unconfirmed draft. Leaving the page never confirms it.
STOP commits an explicitly queued setting. Project SAVE stores committed settings,
not unapplied drafts. Song mode displays its slot-owned selection and blocks editing
with the existing STOP TO EDIT policy.

Remix changes source events, not time: the source for chronological position `c`
is `(c & ~15) + mapping[c & 15]`. Each 16-step block in a 64-step pattern maps separately.
A short LEN can therefore read stored positions beyond LEN inside the same block.
Notes, drum accents/ratchets, ACID lookahead, Motion and eligible locks follow the
same source. Swing, division, loop conditions, Song duration and recording grids
remain chronological. Recording, manual editing, Step Record, Motion capture and
lock editing always write original chronological positions—there is no inverse
mapping or destructive “bake.” Turn Remix OFF to hear the stored line in order.

## Steps, probability and ratchets

PATTERN has LEN 1–64, DIV, SWG and SRC (`STEP`, `G-KCK`, `G-SNR`, `G-HAT`). The 16 white
keys show a bank of steps. On STEP, K1 changes bank; OCT+/− navigate banks when not
owned by another mode. A short step tap toggles on release. Holding about 0.3s turns
an off step on for editing and leaves an already-on step on.

On SEQ STEP, hold a step: K1 PROB, K2 RATCH, K3 ACCENT (clockwise on, anticlockwise
off). OCT+ held plus a step toggles its accent. PROB offers 0–100% in 5% increments,
1-SHOT (first loop after PLAY only), and A/B conditions from 1/2 through 8/8. A/B means
play on loop A of every B loops of that track, not a global bar count.100% always
plays; a percentage uses that track's probability stream.

RATCH 1–4 gives that many equally spaced triggers within the chronological step,
including its swung duration. Probability is evaluated for the step event, not
independently for each ratchet. ACID uses the corresponding note/gate logic for
these retriggers; a TIE does not ratchet. Remix supplies the source event's PROB
and RATCH but does not reorder the chronological condition cycle or hit timing.

## Recording and editing

REC on TRACKS arms/disarms the selected track. Elsewhere a tap toggles REC ALL,
restoring the previous arm mask when turned off. PLAY starts transport separately.
Armed live hits write to the nearest swing-aware chronological position. KEYS notes
record to the selected track; ACID held continuations write TIEs, overlapping legato
writes SLIDE, with chronological wrap protection. Other tracks are not rewritten.

Step Record: KEYS on, STEP page, transport stopped, SRC STEP; tap REC. K1 STEP,
K2 NOTE, K3 TIME (`NOTE/TIE/REST`), K4 FLAG (`-/ACC/SLD/A+S`); EDIT writes a rest.
A key enters a note and advances on release. REC, PLAY, leaving the page or changing
track ends Step Record. Remix does not redirect the recording cursor.

## Motion and parameter locks

Arm the track and play; turning a recordable knob captures its value at the nearest
chronological step. The same parameter/step replaces its existing event. On
SEQ > MOTION, K1 PLAY controls recorded Motion, K2/K3 show MOTION/LOCKS counts,
K4 clockwise asks to CLEAR the track's Motion (OCT+ YES, OCT− NO).

Motion is held-value automation: a value holds until the next event, with base
restoration at chronological step 0, STOP, or PLAY OFF. A repeated mapped source 0
at another chronological position does not reset the base. There is no interpolation
engine. Turning a knob when not recording establishes its new base.

For a parameter lock, enable ENV/STEPS, hold a step off the SEQ pages, and turn a
recordable knob. The step gets its own value without changing the manual base.
OCT− tap while holding clears locks on that page's knobs; hold OCT− about 0.6s to
clear all locks on that step. Locks on OFF steps remain stored but dormant. Locks
play independently of MOTION PLAY; Motion CLEAR does not erase them. SEQ STEP retains
its PROB/RATCH/ACC controls instead of lock entry.

Playback order is Motion → Lock → LFO. Previous locks release on the next chronological
event; STOP restores values. With Remix, events and lock ON eligibility follow the
audible source. Motion and locks share a 1024-event bank per project across all tracks.
`MOTION FULL`/`LOCKS FULL` indicate capacity; clear unneeded events before recording more.

Recordable parameters include sound E0–E7, LEVEL/PAN, supported FX/filter/resonator
and layer controls, LFO RATE/MORPH/DEPTH and routing-slot DEPTH. Timing, MODEL and
routing selectors are not recordable. **ACID's additional MORPH parameter is not
currently in the Motion/lock recordable set**; use manual control or A.MOR LFO routing.
Do not confuse that with an LFO's own recordable MORPH parameter.

## Two LFOs and four routing slots

Each track has two independent LFOs, paired on three pages:

| Page | K1 | K2 | K3 | K4 |
| --- | --- | --- | --- | --- |
| LFO1 | LFO1 WAVE | LFO1 RATE | LFO2 WAVE | LFO2 RATE |
| LFO2 | LFO1 MORPH | LFO1 PHASE | LFO2 MORPH | LFO2 PHASE |
| LFO3 | LFO1 TRIG | LFO1 MODE | LFO2 TRIG | LFO2 MODE |
| DEST | SLOT 1–4 | SRC source/mode | DEST | signed DEPTH |

### Per-slot BIP / UNI+ / UNI−

K2 SRC now selects source and mode together on the existing DEST page:

| Stored source | Display | Source | Offset range |
| --- | --- | --- | --- |
|0 | L1BI | LFO1 | existing bipolar |
|1 | L2BI | LFO2 | existing bipolar |
|2 | L1U+ | LFO1 | base to positive-depth offset |
|3 | L2U+ | LFO2 | base to positive-depth offset |
|4 | L1U- | LFO1 | negative-depth offset to base |
|5 | L2U- | LFO2 | negative-depth offset to base |

BIP keeps the exact RC11 signed DEPTH arithmetic and rounding. UNI+ offsets never
fall below zero; UNI− offsets never rise above zero. From RC13, UNI DEPTH is unsigned
0–100%; mode alone sets direction. Switching from BIP uses the absolute magnitude.
RC12 negative UNI depths load/play as their magnitude, without phase reversal.
BIP retains its signed −100% to +100% range and exact existing arithmetic.
DEPTH 0 produces no modulation. Destination limits still clamp the accumulated value.
Other slots targeting the same parameter can contribute offsets in a different
direction; a single UNI+ slot does not override a simultaneous negative slot.

Each of the four slots has its own mode. Summing/restoration, manual bases, Motion
and Lock precedence are unchanged. MIDI Learn retains parameter IDs and now maps
SRC's CC range across six choices; controller values formerly selecting only LFO1/2
may need adjustment. SRC/mode remains non-recordable by Motion or locks; DEPTH remains
recordable. The page's OCT+ rate-mode shortcut decodes the selected LFO, not the mode.

All old stored 0/1 sources and default/new initialized slots remain BIP. Presets do
not contain or reset routing modes. No record size, parameter ID or flash-layout
change is made. New UNI projects require RC12 to interpret their mode: older
firmware does not understand source 2–5; retain backups before downgrading.
Project SAVE/LOAD retains all four encoded fields. Song keeps its existing ownership:
routing settings remain live-track controls, while row Motion/locks can affect
recordable DEPTH. RC12 does not introduce Song-owned routing selectors.

Waves: SQUAR, SAW, RSAW, SINE, TRI, S&H, WANDR, EXP+, EXP−, RWALK. MODE is SYNC,
HZ or TIME; TRIG is FREE, HIT or PLAY. On LFO pages OCT+ cycles the last-touched
LFO's rate mode when KEYS does not own OCT. Routing slots retain independent source
and depth; several slots may target the same parameter.

Select `A.MOR` to modulate ACID PWM/MORPH. Either or both LFOs can target it on either
ACID track, around its manual base; sums clamp 0–127 and the base returns after each
processing block, without cumulative drift. On SAW it is inaudible; on non-ACID it
is inert and labelled `--`. Existing destination IDs remain unchanged; A.MOR is
appended at 19. The LFO's own MORPH shapes the LFO—it is not ACID MORPH.

## Grids and WRITE

On a selected drum track ARP opens GRIDS. MAP page 1 has MODE/X/Y/CHAOS; page 2 has
KICK/SNARE/HAT fill and K4 `ASSIGN / 3 TRACKS`. In EUCL mode, page 1 replaces X/Y/CHAOS
with each channel's length; page 2 retains fills and ASSIGN.

One clockwise K4 movement on page 2 assigns T1/T2/T3 SRC to G-KCK/G-SNR/G-HAT.
Anticlockwise does nothing. It does not change models, steps or sounds, so choose
appropriate drum models yourself. T4–T8 are untouched. Song editing restrictions apply.

On either GRIDS page, stopped, **OCT+ WRITE** overwrites all non-ACID tracks currently
sourced from a Grids channel with a deterministic 64-position 1/32 snapshot of that
channel. Trigger/accent fields are written with ordinary drum defaults; LEN 64,
DIV 1/32 and SRC STEP are set. Sounds, other track parameters, STEP-source tracks,
ACID tracks, Remix settings and live generator runtime are retained. This is an
explicit overwrite: save the project first. `NO GRIDS TRACKS` means no eligible
targets; while playing/chain busy, `STOP TO WRITE` leaves everything unchanged.
The normal WRITE-available OCT+ cue breathes when stopped with eligible tracks;
modal and other button-mode LED priorities still apply.

Written steps can now use GLOBAL DRUM Remix. The snapshot is not a perfect timing
clone of live Grids under all swing settings: live Grids halves swung 1/16 periods,
whereas STEP playback swings its own selected division. Track swing also affects
STEP playback. WRITE preserves those settings rather than silently changing groove.

## Projects and Song

SAVE > PROJECT: K1 SLOT A–D, K2 NAME, K3 LOAD, K4 SAVE. OCT−/OCT+ are alternative
LOAD/SAVE inputs through the same handlers. GO actions use a first activation to
arm (`AGAIN: LOAD/SAVE`) and a second within approximately 1.5s to act. Shortcuts do
not bypass this safeguard. Stop transport for SAVE/LOAD (`STOP TO SAVE/LOAD`).
NAME supports 12 characters; OCT+ writes the name, OCT− or HOME cancels.

A project stores sounds, levels, steps/timing, Grids settings, LFOs/routing, Motion
and locks, Song rows, global Drum/individual Bass Remix, per-track TB-3PO settings
and seeds, and ACID MORPH/PWM values. FDRF records remain 3840 bytes; Remix/TB-3PO
use validated reserved extensions, while ACID's extra parameter uses spare bytes.
Absent/invalid metadata gives Remix OFF and generator defaults without regenerating
old patterns. Legacy wave migration is described above. Settings such as COLOR
are device-wide and not changed by loading another project.

The last saved project restores at power-on. Failed restore shows `LOAD FAILED`
and asks whether to start new; the existing YES/NO choices remain. SEQ held at
power-on is the safe-start path that skips project loading. Older supported
formats convert forward; pre-FDRF firmware cannot be relied on to read new saves.
The separate lock bank is committed before its linked project record. A missing
or invalid bank yields `LOCKS LOST`, not reinterpretation as another bank.
USR1/USR2 hold 80 KiB each; USR3 holds 48 KiB because the final 32 KiB is used by lock
banks. The existing one-time shortening of a longer legacy USR3 can discard its tail.

SONG supports 16 rows, each referencing A–D with 1–16 repeats and optional LOOP.
K1 ROW, K2 SLOT, K3 REPS, K4 LOOP. Select the `+` row and turn SLOT/REPS to append.
Hold REC on a row for DELETE ROW, or on the append row for CLEAR SONG; confirm with
OCT+, cancel with OCT−. PLAY on SONG prepares/starts the chain; STOP restores the
live project's timing, wave/MORPH and record-arm state.

Each row uses its slot's steps, LEN/DIV/SWG/SRC, Motion/lock bank, Remix selections
and TB-3PO settings/seed. ACID wave/MORPH are also restored from prepared slot data.
**Song is not a complete sound-kit/preset switch**: the existing live models and
other sound settings remain the basis of playback. A row lasts as long as its
longest track pattern times REPS. Mapping does not change that duration. Slot-owned
Remix never overwrites live settings; editing/recording restrictions while armed
or running use STOP TO EDIT. Automatic Song mutations affect RAM copies only.

## Color themes

GLO short → MENU → COLOR. K1 steps in either direction; OCT+ advances, wrapping.
The twelve themes in menu order are ORANGE, ORANGE INV, BLUE, BLUE INV, WHITE,
WHITE INV, YELLOW, YELLOW INV, VIOLET, VIOLET INV, RED, RED INV.

| Stored index | Name | Background | Ink |
| --- | --- | --- | --- |
| 0 | ORANGE (default) | #E96F25 | #321308 |
| 1 | ORANGE INV | #160904 | #F48532 |
| 2 | BLUE | #2445C8 | #EAF1FF |
| 4 | BLUE INV | #F4F8FF | #2445C8 |
| 3 | WHITE | #FFFFFF | #000000 |
| 5 | WHITE INV | #000000 | #FFFFFF |
| 6 | YELLOW | #FFD000 | #171717 |
| 7 | YELLOW INV | #171717 | #FFD000 |
| 8 | VIOLET | #7524CF | #FFFFFF |
| 9 | VIOLET INV | #FFFFFF | #7524CF |
| 10 | RED | #D80000 | #FFFFFF |
| 11 | RED INV | #FFFFFF | #D80000 |

Values are quantized to RGB565 by the existing renderer. ORANGE's two polarities
use the distinct specified pairs, not an exact swap. Stored indices/default are
unchanged: earlier index 0/1 selections now show Orange. Historical GREEN slots 3/5
are WHITE/WHITE INV; obsolete GREEN entries are not restored. Device preferences
save when MENU closes. SEL ENTER does not change a theme.

## Attribution and release status

Nauclerus development/modifications/remix work: **Mr. Roboto**. Hortator drum-machine
fork: **DEADACTIVE**. Original Felucca firmware: **Leo Kuroshita (@kurogedelic),
Hügelton Instruments**. Original authors, contributors and license notices remain
in MENU > ABOUT and [LICENSING.md](../LICENSING.md); see the README Credits for the
complete third-party list.

See [release notes](RELEASE_NOTES_0.25.0.md) for the stable announcement and
[release readiness](RELEASE_READINESS_0.25.0.md) for release validation and known limitations.

The ORB record is resolved by Mr. Roboto's explicit independent-creation/redistribution declaration;
Future Retro Orb is conceptual inspiration only. Exact table values and their source-to-binary chain
are retained; see [current provenance](../assets/remix/README.md#attribution-record).

Identity migration and compatibility: [NAU-0250-015](IDENTITY_MIGRATION_0.25.0.md).
