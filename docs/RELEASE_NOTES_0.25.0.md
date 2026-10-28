# Nauclerus 0.25.0

Canonical project: **Nauclerus** — [repository](https://github.com/Mr-Roboto/nauclerus),
[releases/downloads](https://github.com/Mr-Roboto/nauclerus/releases).
Nauclerus 0.25.0 promotes the accepted RC13 implementation with no further DSP changes.
[Simulator](https://mr-roboto.github.io/nauclerus/#play) ·
[Interactive manual](https://mr-roboto.github.io/nauclerus/#manual) ·
[Verified-package web installer](https://mr-roboto.github.io/nauclerus/webapp/installer/).

Nauclerus expands the FM-1 into an eight-track drum and ACID groovebox, retaining
Hortator's sequencing, recording, Motion, locks and effects.

- **Release UI fixes:** MENU/ABOUT transport headers refresh when playback changes; entering MENU immediately ends MIDI Learn and MIDI-held effects; held-step accents on REMIX remain available without applying a pending draft.
- **ORB Remix:** all 256 owner-created mappings, one shared Drum configuration and independent
  Bass configurations on physical ACID tracks. K1/K2 draft, K4 SET or OCT+ confirm;
  K3 ON/OFF acts without confirming a draft. Live changes latch at track-step boundaries.
- **TB-3PO:** the X0X-RMX v1.1.0 melody generator, 22 scales, seeded phrases and restrained
  NEW/MUTATE. ARP selects the bass generator or Grids according to the current model.
- **ACID:** SAW, variable 5–95% PULSE and three-family MORPH, FASTSLOW TUNE −12/default
  LEVEL −4 dB, final four-page EDIT layout, and A.MOR routing from either/both LFOs.
  Two ACID tracks remain supported, with one simultaneous MORPH voice.
- **LFO routing:** per-slot BIP/UNI+/UNI− via L1BI/L2BI/L1U+/L2U+/L1U-/L2U- on the existing DEST page.
  Legacy source 0/1 and signed-depth rounding are unchanged. UNI DEPTH is unsigned 0–100%; mode alone determines direction. BIP stays signed −100%..+100%; all four slots retain independent source/mode, destination and depth.
- **Grids WRITE:** stopped OCT+ writes channel rhythms into 64 normal steps for Drum Remix;
  page 2 K4 quickly assigns kick/snare/hat sources to T1–T3.
- **Workflow:** GLO opens MENU; HOME-long enters/exits Settings; HOME-short cycles Settings
  or escapes normal overlays. SEL enters dedicated MENU screens. PROJECT OCT−/+ reuse
  guarded LOAD/SAVE. TB-3PO OCT works with KEYS and ENV/STEPS enabled.
- **Appearance:** 12 compatible color choices, including the Orange LCD pair at IDs 0/1.
  Original author credits remain, with Mr. Roboto's Nauclerus credit added.

Project records remain 3840-byte FDRF. Source/mode uses existing values 0–5; old 0/1 stays BIP. Global Drum/Bass Remix, generator settings/seeds
and PWM/MORPH are retained; Song rows use their saved slot data. Recording/editing
remain chronological. Legacy WAVE identities are preserved: SQUARE becomes 50% PULSE,
old fixed 25% PULSE uses the nearest width, and MORPH stays MORPH. USB/update protocol
uses **FM-1_25000**, with USB product **Nauclerus**. The shared **ota-FM-1_900** loader and recovery implementation are unchanged. Use the updated installer/CLI for RC13 migration and rollback; both old and new port names remain supported.

**Known limitations:** the owner accepted RC11 after 10/11 physical checks PASS; the
remaining subjective resonance/filter character concern is accepted for 0.25.0.
Sound is not claimed identical to X0X-RMX or analogue hardware. Auto-MUTATE counts
pattern loops; ACID MORPH is manual/LFO-controlled, not Motion/lock recordable.
Grids WRITE is a rhythm snapshot, with potentially different swung STEP timing.

See the [user guide](USER_GUIDE.md), [changelog](../CHANGELOG.md) and
[credits/licenses](../LICENSING.md). Nauclerus by **Mr. Roboto**, based on **Hortator
by DEADACTIVE** and **Felucca by Leo Kuroshita / Hügelton Instruments**, with the
original third-party acknowledgements retained.

ORB provenance is resolved by the owner's explicit independent-authorship and Nauclerus redistribution
permission; Future Retro Orb is conceptual inspiration only. No pattern bytes or GPL notices changed.
New source/mode IDs require RC12 or newer; RC13 normalized legacy negative UNI depths. Retain project backups before downgrading.

Firmware: `nauclerus-0.25.0.fwsc` (609649 bytes), **FM-1_25000**.

SHA-256: `b496e081fd78c05eaa3cd368caf384285ec7b708477c5ff638f373e6a3c2a870`.

Matching corresponding source, full license notices, asset audit and checksums accompany this release. Five optional unverified documentation images are excluded.

Identity migration and compatibility: [NAU-0250-015](IDENTITY_MIGRATION_0.25.0.md).
