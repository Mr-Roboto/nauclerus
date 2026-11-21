# Nauclerus 1.1.0 — Development Release Notes (Draft)

**Status:** development branch only. **Nauclerus 1.0.0 remains the published Stable release.**
A development 1.1.0 package was built and locally tested on the FM-1 for the delay
and encoder changes; final 1.1.0 validation and publication are still pending.

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

### Stereo ping-pong delay (development, not yet verified)

- Add an FX DELAY second page MODE: NORM (existing mono/stereo-equal wet return)
  or PING (successive echoes alternate L/R), using the same 65,536-sample
  delay buffer. Switching modes clears old delay contents over successive
  audio blocks rather than replaying stale echoes.
- Keep earlier project records 3840 bytes: the new parameter is appended
  after stored globals and persists as rsv[0] (previously zero). Old records
  therefore select NORM by default.

### Expanded LFO destinations (development, not yet verified)

- Preserve all old destination IDs, especially Nauclerus ACID MORPH = 19.
- Append DEST 20 DST (distortion amount), 21 CHO, 22 DLY and 23 REV
  (per-track effect sends). Every existing LFO routing remains meaningful.

### Eight-track solo (development, not yet verified)

- GLO held: eight black keys F#3..A#4 select solo tracks 1..8.
  F#5 switches between TOGGLE and MOMENTARY. SOLO is a runtime state;
  the white keys retain their mute functionality and MIDI LEARN remains G4.

### USB MIDI Clock Out, routing and Thru (development, not yet verified)

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

- Set ORANGE (saved index 0, also the default) to brighter, saturated orange:
  background **#FF9500**, almost-black ink **#120804**. The increased green
  component targets more perceived luminance than the too-dark **#FF4D00** candidate.
- ORANGE INV (saved index 1) uses background **#100604**, with matching bright
  **#FF9500** text and graphics.
- Preserve all fourteen theme IDs, the menu order, the twelve other colors and
  settings persistence. The hardware LCD and web emulator share the RGB565
  palette; site-wide CSS/physical panel colors do not change.

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

## Development status and release checklist

The changes above have been committed to `dev/nauclerus-1.1.0`.
Initial source inspection, simulator build and focused delay tests passed;
the owner confirmed the prior 1.1.0 delay/encoder build worked on FM-1 hardware.
The refreshed ORANGE palette and the four feature groups above require a new
production build, focused host regressions, simulator checks, and final hardware
validation before release.

Before publishing 1.1.0:

- Build the firmware and matching browser simulator using the established
  repository toolchain; verify the package and loader checks.
- Perform narrowly scoped timing and project-compatibility checks, followed
  by the agreed hardware acceptance.
- Record final release tag, build artifact, SHA-256, and public download
  references only after they exist.
- Replace this draft status with verified results and an actual release date.

**Credits:** Nauclerus modifications by Mr. Roboto; original Hortator by
DEADACTIVE; Felucca by Leo Kuroshita / Hügelton Instruments. Original
attribution and licences are retained.
