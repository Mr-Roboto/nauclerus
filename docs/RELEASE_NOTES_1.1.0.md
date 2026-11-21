# Nauclerus 1.1.0 — Development Release Notes (Draft)

**Status:** development branch only. **Nauclerus 1.0.0 remains the published Stable release.**
No 1.1.0 package, GitHub release, web deployment or hardware validation is claimed by this document.

## Changes since 1.0.0

### Tempo-synchronised delay times

- Add **1/4D** (dotted quarter), **1/8D** (dotted eighth) and **1/16D**
  (dotted sixteenth) as distinct FX delay TIME choices, using the existing
  Hortator delay-division arithmetic.
- Preserve the existing **8T** and **16T** triplets; additionally expose
  the upstream delay-specific **4T** and **32T** choices.
- Present delay-time choices in descending duration order without renumbering
  their stored values. Legacy selection IDs **0–9** retain their exact meanings;
  new selections use append-only IDs **10–14**.
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
- No changes to drum/ACID synthesis, sequencing, ORB Remix, TB-3PO,
  Grids, saved project layout, or flash-memory organization.
- The 1.0.0 published firmware and its release artifacts are not modified.

## Development status and release checklist

The changes above have been committed to `dev/nauclerus-1.1.0`.
Initial source inspection checked the old parameter IDs, extended duration
mapping and CSS selectors; this is **not a firmware build or hardware test**.

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
