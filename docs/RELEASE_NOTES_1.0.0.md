# Nauclerus 1.0.0 Stable

Released 2026-11-05. [GitHub release](https://github.com/Mr-Roboto/nauclerus/releases/tag/v1.0.0)
and [Web installer](https://mr-roboto.github.io/nauclerus/webapp/installer/).
Historical releases and their validation records remain unchanged.

## Changes

- Retain the 0.25.1 Grids WRITE track-length fix, ACID TRIANGLE waveform and
  fourteen color themes, including corrected VIOLET and added PINK variants.
- Accept both supported FM-1 1 MiB SPI NOR chips: Puya P25Q80 (`0x856014`)
  and UC25HQ80 (`0xB36014`). Unknown IDs retain RAM-only saves. The minimal
  allowlist comes from DEADACTIVE/hortator `drum-v1.0.1`, commit
  `c8cca2ce9a6a1ba5a055ef3bd4aadc9ef5237bb4`; the ID is read once with
  interrupts disabled before applying the allowlist.
- Change the application USB product from `Nauclerus` to exactly `Nauclerus FM-1`
  (30-byte UTF-16LE descriptor) for conventional FM-1 installer discovery.
- Both Python and Web MIDI installers recognize `FM-Tangram` alongside existing
  names. Web MIDI prefers exact input/output names; otherwise it accepts only a
  unique pair differing by whitespace-separated trailing numeric suffixes.
  Ambiguous pairs are rejected without probing them; no unrelated-name probing
  or arbitrary-output fallback was added.

Application/package identity remains `FM-1_25000`; display version is
`Nauclerus 1.0.0`. Manufacturer `Hügelton Instruments`, application VID/PID
`1209:0001`, shared loader `ota-FM-1_900` / `Felucca Update` (`1209:0002`),
OTA/SysEx protocol and baseline pins are preserved. Descriptor integrity checks
were updated narrowly for the exact approved name, without exemptions for other
source or loader changes. No Bluetooth release is included.

FDRF records, persistence and flash layout are unchanged. ORB Remix, TB-3PO,
Grids, ACID, SONG, PERFORM, presets and accepted musical behavior are retained.
No upstream sequencer, sampler, effects, editor or storage-format changes are imported.

## Verified release artifact

- File: `nauclerus-1.0.0.fwsc`
- Size: **609,649 bytes**
- SHA-256: `72e923840964a892a25dadb3db6b91576734fca1e1e73b86ec1a2aa007f478e0`
- Loader SHA-256: `cc98eed224299fa42ca22a546073e0f8b98d8362e26b5f325a9aaeab287bdf5d`

The previously validated production artifact is published byte-for-byte without
a release-time firmware rebuild. Matching source, retained licenses and checksums
accompany the package. The earlier candidate hash beginning `f28dff7f` is superseded
by the final compatibility build above.

## Completed validation

- **PASS:** 55 targeted compatibility checks (26 Python/source/integrity checks
  and 29 mocked Web MIDI cases), covering descriptor bytes/length, identities,
  unchanged VID/PID and loader descriptors, discovery names, unique/ambiguous
  pairing and unrelated-device rejection.
- **PASS:** production firmware build and package/loader integrity checks.
- **Owner-confirmed physical PASS:** Nauclerus 0.25.0 → Nauclerus 1.0.0.
- **Owner-confirmed physical PASS:** Nauclerus 1.0.0 → AMB1 through The Firmware Hub.
- **Owner-confirmed physical PASS:** AMB1 → Nauclerus 1.0.0 through our Web installer.

Physical results and the completed production checks were supplied by the owner
for stable publication. They are not new hardware tests performed by the release
agent. Publication checks cover deployed links, metadata and exact downloaded
artifact hashes; no additional full regressions or sanitizers are required here.

## Compatibility and retained limitations

Adding `FM-1` improves conventional name-based discovery; it does not guarantee
every third-party installer, package format or version policy. Reselect DAW ports
if the new USB name changes saved assignments; hosts may cache device names.
Identical `FM-1_25000` identities do not distinguish semantic Nauclerus versions.
Ambiguous multi-device names remain deliberately unsupported by Web MIDI pairing.

GRIDS MAP page 2 K4 assigns sources, not sounds; track LEN belongs to TRACKS.
Stopped OCT+ WRITE snapshots eligible Grids tracks to STEP, preserves LEN and
sets DIV 1/32. Live Grids bypasses ORB Remix; STEP snapshots can use it, and swing
can make snapshot timing differ. PROJECT SAVE needs two activations within about
1.5 seconds; save a slot before naming it. The previously accepted subjective
ACID resonance/filter-character limitation remains.

Nauclerus development, modifications and remix work: Mr. Roboto. Upstream
Hortator: DEADACTIVE. Felucca: Leo Kuroshita / Hügelton Instruments. Original
contributor, copyright and license notices are retained.
