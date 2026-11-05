# Nauclerus 1.0.0 release candidate

Based on `dev/nauclerus-0.25.1`, commit
`33b4502bc57111962f96338cffc270aa73646e03`. This candidate is not a published
stable release. Historical 0.25.0 release and validation records remain unchanged.

The only firmware logic change accepts both supported FM-1 1 MiB SPI NOR chips:
Puya P25Q80 (`0x856014`) and UC25HQ80 (`0xB36014`). Unknown IDs retain RAM-only
saves. The allowlist comes from [DEADACTIVE/hortator, drum-v1.0.1](https://github.com/DEADACTIVE/hortator/blob/c8cca2ce9a6a1ba5a055ef3bd4aadc9ef5237bb4/firmware/src/project.c),
commit `c8cca2ce9a6a1ba5a055ef3bd4aadc9ef5237bb4`. Nauclerus reads the ID once
with interrupts disabled before applying that allowlist.

FDRF project records, persistence, flash layout and loader protections are
unchanged. ORB Remix, TB-3PO, Grids, ACID, SONG and PERFORM are retained, including
the 0.25.1 Grids LEN, TRIANGLE and color-theme corrections. No Hortator Bluetooth
MIDI, sequencer, FILL / REPEAT STEP / NUDGE, sampler, effects, web editor or new
storage-format changes are imported.

The display version is `Nauclerus 1.0.0`; application/package identity remains
`FM-1_25000`, USB product `Nauclerus`, shared loader `ota-FM-1_900` (USB:
`Felucca Update`). The expected package is `build/nauclerus-1.0.0.fwsc`.

Validation scope is only a focused flash-ID check and, on success, one standard
non-Bluetooth package build with site generation disabled. No existing isolated
flash-ID test was found; a temporary harness checks the production gate. Full
regressions, benchmarks, simulator/site builds, hardware flashing, hardware
storage/update behavior and audio/UI acceptance are outside this candidate task.
Candidate checks (2026-11-05):

- `python3 /tmp/nau-1000-001-flash-id-test.py`: PASS. The temporary harness
  extracts and compiles the production allowlist and boot gate with
  `cc -std=c99 -O1 -Wall -Wextra -Werror`. It accepts `0x856014` and `0xB36014`,
  rejects `0x123456` before flash mapping, and checks one IRQ-protected ID read
  with interrupts restored for each case. A copy is retained locally at
  `build/nau-1000-001/flash_id_test.py`; generated harness: `flash_id_test.c`
  in the same directory. This isolates the gate, not full persistence behavior.
- `env FELUCCA_BLE=0 FELUCCA_CORE_REF=0 DRUM_BUNDLE=0 DRUM_SITE=0 DRUM_PACKAGE=1 ./build.sh`:
  PASS (one build). Log: `build/nau-1000-001/build.log`. Package: 609,649 bytes.
  The build's internal RAM/register checks and pinned-loader check passed;
  loader remains 6,493 bytes, SHA-256
  `cc98eed224299fa42ca22a546073e0f8b98d8362e26b5f325a9aaeab287bdf5d`.
- Package SHA-256 (`sha256sum build/nauclerus-1.0.0.fwsc`):
  `f28dff7fca14a1f311e649f9d9d8484a18deb0f6bd50bd243c2776cbefcbec58`.

No blockers were encountered in NAU-1000-001. The candidate source was subsequently
committed as `07bb53f` on `dev/nauclerus-1.0.0`; the checks above describe the
original candidate build. NAU-1000-002 changes documentation and website source
only and does not rebuild or alter that package. Its website changes have not
been exercised through staging or browser automation. Publication is still pending.

Current control limitations are unchanged:

- GRIDS MAP page 2 K4 `ASSIGN / 3 TRACKS`, clockwise, assigns T1–T3 sources to
  kick/snare/hat without choosing their sounds. Track LEN is set on TRACKS,
  not GRIDS; EUCL channel lengths are separate generator settings.
- Stop playback, then press OCT+ WRITE on either GRIDS page. This overwrites
  eligible Grids-sourced drum tracks with ordinary STEP patterns, sets DIV 1/32,
  preserves LEN and clears unused steps beyond LEN. Live Grids bypasses ORB
  Remix; written STEP patterns can use Remix. Swing can make the snapshot's
  timing differ from live Grids.
- PROJECT SAVE needs two activations within approximately 1.5 seconds even for
  an empty slot. Save that slot before NAME; K2 clockwise opens NAME only for
  an occupied slot. An empty slot reports `EMPTY SLOT`.

Remaining publication work requires separate authorization: build the matching
1.0.0 simulator/source archive, stage and review the integrity-verified standard
package with the updated installer, then publish the reviewed release assets and
Pages deployment. Local staging must remain marked unpublished until deployment
and public verification actually complete. No Bluetooth release is prepared.

The previously accepted subjective ACID resonance/filter-character limitation
remains; this task does not reassess it.

Nauclerus development, modifications and remix work: Mr. Roboto. Upstream
Hortator: DEADACTIVE. Felucca: Leo Kuroshita / Hügelton Instruments. Original
contributor, copyright and license notices are retained.
