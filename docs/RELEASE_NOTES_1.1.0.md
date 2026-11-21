# Nauclerus 1.1.0 Stable

Released 2026-11-21. [GitHub release](https://github.com/Mr-Roboto/nauclerus/releases/tag/v1.1.0)
and [Web installer](https://mr-roboto.github.io/nauclerus/webapp/installer/).

The owner chose to conclude 1.1.0 with the hardware-tested #FF4D00 orange palette.
The later #FF9500 color experiment was reverted without changing any accepted feature.

## Changes since 1.0.0

### Tempo-synchronised delay times

- Add **4D** (dotted quarter = 1/4D), **8D** (dotted eighth = 1/8D) and
  **16D** (dotted sixteenth = 1/16D) as distinct FX delay TIME choices,
  using the existing
  Hortator delay-division arithmetic.
- Preserve the existing **8T** and **16T** triplets; additionally expose
  the upstream delay-specific **4T** and **32T** choices.
- Present delay-time choices in descending nominal duration order without
  renumbering their stored values: **4BAR, 2BAR, 1/1, 1/2, 4D, 1/4, 8D,
  4T, 1/8, 16D, 8T, 1/16, 16T, 1/32, 32T**. Labels 4D/8D/16D deliberately
  fit the four-glyph LCD value field; 16D must not appear as a second 1/16. Legacy selection IDs **0–9** retain their exact meanings;
  new selections use append-only IDs **10–14**.
- Set the factory TIME default for a newly initialized project to **8D** (stored
  ID **12**) instead of the regular eighth **1/8** (stored ID **1**). Projects
  already saved with an explicit TIME value keep that value on LOAD.
- Keep the existing delay buffer, firmware project layout, parameter identity,
  and preset persistence unchanged; no migration or silent reinterpretation
  of saved delay times.

**Delay-time ceiling:** the unchanged 65,536-sample delay ring supports
approximately **1.49 seconds** at the current audio sample rate.
A requested longer delay is **clamped to the maximum interval**, not cut off
mid-echo. The repeating feedback tail continues at the capped interval.
For instance, 1/4D at 40 BPM requests 2.25 s but uses approximately 1.49 s;
1/8D (1.125 s) and 1/16D (0.5625 s) fit at 40 BPM.
The ceiling is not new: the 1.0.0 delay had the same buffer limit.

### Stereo ping-pong delay

- Add an FX DELAY second page MODE: NORM (existing mono/stereo-equal wet return)
  or PING (successive echoes alternate L/R), using the same 65,536-sample
  delay buffer. Switching modes clears old delay contents over successive
  audio blocks rather than replaying stale echoes.
- Keep earlier project records 3840 bytes: the new parameter is appended
  after stored globals and persists as rsv[0] (previously zero). Old records
  therefore select NORM by default.

### Expanded LFO destinations

- Preserve all old destination IDs, especially Nauclerus ACID MORPH = 19.
- Append DEST 20 DST (distortion amount), 21 CHO, 22 DLY and 23 REV
  (per-track effect sends). Every existing LFO routing remains meaningful.

### Eight-track solo

- GLO held: eight black keys F#3..A#4 select solo tracks 1..8.
  F#5 switches between TOGGLE and MOMENTARY. SOLO is a runtime state;
  the white keys retain their mute functionality and MIDI LEARN remains G4.

### USB MIDI Clock Out, routing and Thru

- SYSTEM SYNC OFF/USB provides MIDI Clock and Start/Stop on USB when
  internally clocked; when following TRS clock, its Start/Stop/Clock is
  forwarded to USB rather than being regenerated.
- SYSTEM ROUT NOTE (legacy drum mapping), SEL (selected track pitched
  from C4), TRACK (MIDI channels 1..8 to tracks 1..8, pitched from C4).
- GLOBAL THRU OFF/ON forwards channel events from TRS MIDI input to USB,
  with no USB-input echo back to USB. THRU is a device setting using a
  spare settings-record bit, OFF for earlier records.
- FM-1 TRS is an input only. No new BLE requirements, no MIDI TRS output.

### LCD orange theme refinement

- Keep ORANGE (index 0) at **#FF4D00** background / **#120804** ink,
  and ORANGE INV (index 1) at **#100604** background / **#FF4D00** ink.
- The user found the color subjectively insufficiently vivid but explicitly
  elected to retain it and finalize the stable release. The later **#FF9500**
  experiment was reverted.
- All other twelve themes, their IDs and settings remain unchanged.

### Browser emulator visual accuracy

- Remove absolute-position tick marks from the continuous FM-1 encoders in
  the full browser panel and the feature-lab control knobs.
- Retain the position indicator for **MASTER volume**, which is a bounded
  potentiometer. Encoder interaction, gestures and MIDI parameter behavior
  are unchanged.
- Update the browser Controls help, FX feature description and development
  user guide for the new delay divisions and encoder appearance.

## Compatibility and scope

- Base: **Nauclerus 1.0.0 Stable**.
- Preserve firmware/product identity `FM-1_25000`, USB product name
  `Nauclerus FM-1`, loader identity `ota-FM-1_900`, and the existing
  USB VID/PID and OTA protocol.
- Keep the original drum/ACID synthesis, ORB Remix, TB-3PO, Grids
  and flash-memory record sizes unchanged. MIDI input/playback behavior
  changes only when ROUT is set beyond its legacy NOTE default; the
  existing LFO defaults retain their prior behavior; the delay TIME
  default is intentionally changed to 8D for new/initialized projects.
- The 1.0.0 published firmware and its release artifacts are not modified.

## Verified release artifact

- File: `nauclerus-1.1.0.fwsc` (609,649 bytes)
- SHA-256: `68e15f3bfed73bb3202f18ed877be6716b7aa99a6686036780e2a48e533413dd`
- Application SHA-256: `290519d8076ea94bb3667f63b694216dfabbdc40a7156494f9b74b7c309f9127`
- Pinned loader SHA-256: `cc98eed224299fa42ca22a546073e0f8b98d8362e26b5f325a9aaeab287bdf5d`
- Verified candidate source: `6ca5ba3ff5ad7ea76912bf161d29719837c221c0`
- [Successful regression and production build run #44901940987](https://github.com/Mr-Roboto/nauclerus/actions/runs/44901940987)

The stable release retains the verified candidate bytes. The final release
source restores the exact firmware implementation tested in that build.

## Completed validation

- **PASS:** seven host regression programs, package build and pinned loader.
- **Owner-confirmed hardware PASS:** stereo ping-pong, LFO FX send modulation
  and visual rings, SOLO/MUTE, MIDI clock/routing/THRU observed with a computer
  MIDI monitor, three-second confirmation and new SYSTEM layout.
- **Subjective limitation accepted:** Orange is not as vivid as desired.
- **Not claimed:** exhaustive external-clock or interrupted-update recovery tests.

Historical 1.0.0 artifacts remain unchanged.

**Credits:** Nauclerus modifications by Mr. Roboto; original Hortator by
DEADACTIVE; Felucca by Leo Kuroshita / Hügelton Instruments. Original
attribution and licences are retained.
