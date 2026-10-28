# ORB Remix Phase 4B: song-owned selections

## Ownership and activation

`seq_remix_config[8]` remains the edited project's persistent configuration.
`seq_remix[8]` remains its transport snapshot. Song playback never writes either
configuration from a project slot. The host-only test setter is unchanged; no
UI/MIDI selection controls are added.

`chain_src_t` gains eight uint16 runtime snapshots, using the existing transport
bit encoding (bit 8 ON, low eight bits canonical pattern). This costs 64 B across
four prepared slots in ordinary .bss. It is not a second persistent format.
`chain_prepare()` first runs the existing project validation, then reuses the
Phase 4A `proj_remix_valid()` and `proj_remix_code()` decoder once per used slot,
and `seq_remix_state()` encodes the eight snapshots. The existing bank validation
is unchanged. No project or extension checksum is calculated in the audio ISR.

`seq_remix_effective(track)` chooses the running song's prepared slot snapshot,
otherwise the live snapshot (armed but not running stays OFF, as previously).
`seq_source_step()` continues to use the existing production mapping core.
Drum events, ACID current/next reads and Motion/lock playback all share it.
Grids still bypasses mapping. Recording and editing coordinates are unchanged.

`chain_apply()` already selects `chain.slot` before any new-row event. That same
index now selects all eight Remix snapshots together with the steps and bank.
No new transition callback, timing calculation or per-track switch is needed.
Existing lock release, Motion restoration, ACID gate closure, `chain.cut` and
sample remainder handling remain unchanged. Repeats keep their slot; a return or
loop selects its prepared configuration again. STOP clears temporary live
snapshots and restores the live timing/bank; ordinary PLAY rebuilds the snapshot
from the unchanged live configuration. A song restart selects its first row.

## Persistence and compatibility

FDRF remains 3840 B. No format magic, member offset, lock-bank format, flash
allocation or .noinit footprint changes. The Phase 4A 32-byte reserved-field
extension is unchanged. Missing, unsupported or malformed metadata decodes OFF
for all tracks. A corrupt outer project still follows the existing rejection
path. Converted FDRE and earlier records have no extension and remain OFF.

## Validation

`tests/remix_song_test.c` uses the real flash/project harness, chain preparation,
sequencer, Song, Motion/lock and audio implementations. The focused runner builds
it with ASan/UBSan and no sanitizer exclusions.

Coverage includes all 256 selections plus OFF on every track and project slot;
lengths 1/8/16/24/40/64; all divisions and four mapping blocks; swing, mapped
ratchets and wrapped lookahead; invalid metadata and outer record rejection;
converted FDRE; explicit NOTE/TIE/SLIDE/rest, held gates/locks and a 7-sample
mid-block row boundary with different row selections; live-state preservation,
ordinary playback, STOP and both stopped/playing restart.

An audio/state oracle compares active Remix with physically materialized step
and bank data played OFF. Both run production timing and DSP. It exercises all
16 song rows, A-D, row repeats, returns, consecutive rows of the same slot, two
song traversals, partial lengths, probability, ratchets, Grids, LFO, Motion and
Locks (including the same parameter). Saved records, banks and live step data
are checked unchanged after playback.

The pre-Phase-4B source was backed up before editing. Compiling the same OFF
fixture against that tree and this tree gives identical audio/state fingerprints:

| ACID | LOOP | Audio FNV64 | State FNV64 | Blocks | Rows | Repeat boundaries | Mid-block row changes |
| --- | --- | --- | --- | ---: | ---: | ---: | ---: |
| OFF | OFF | fe691e5441724035 | 69ae9abf1ca602c4 | 18781 | 16 | 15 | 14 |
| OFF | ON | b6376ecf51dc273e | 86c1da44335e313c | 37539 | 33 | 30 | 31 |
| ON | OFF | 7edf74cda387ee83 | ac79887246318d34 | 18781 | 16 | 15 | 14 |
| ON | ON | e79a7b3fb6b0853b | 8d199ccfea5035b4 | 37539 | 33 | 30 | 31 |

These fingerprints include rendered stereo PCM, chronological transport, voice,
gate/slide, ratchet, probability, timing, parameter and Motion/lock state. No
golden references or thresholds are regenerated or changed. Earlier focused
tests for song slots with no metadata still require OFF; the Phase 4A test that
explicitly required a saved active configuration to be bypassed now requires the
slot-owned mapped note instead, as intended by Phase 4B.

The functional Song suite passes in a normal optimized host build:

```sh
cc -O2 -Wall -Wno-unused-function -Wno-int-to-pointer-cast -Ibuild/gen -Ifirmware/src tests/remix_song_test.c -lm -o build/host/remix_song_functional_test
build/host/remix_song_functional_test
```

The unchanged full sanitizer configuration in `sh tests/run_remix_tests.sh`
passes the prior mapping/sequencer/Motion/lock/recording/project tests and the new
Song all-selection, metadata and explicit transition checks. It then **fails**
in the expanded full-audio fixture: `firmware/src/fx.c:117`, left shift of negative
value -1 in `lowcut1()` (`d << sh`). Compiling the same OFF audio fixture against
the saved pre-Phase-4B tree with ASan/UBSan reproduces the identical failure.
This DSP source is unchanged. No sanitizer suppression/exclusion was added;
the focused runner remains nonzero, and full-audio sanitizer validation is
blocked. Normal functional/audio comparisons passing does not resolve that UB.

`sh tests/run_tests.sh` completes with status 1 at the existing frozen-source
guard because tag `frozen-base-10` is unavailable, pinned to
`731386008fae060ae885f44b4765d172ecfae4b3`. Its pin and checks are unchanged.
All executed existing functional, sanitizer, Song, ACID, Motion, lock, boot,
UI, fidelity and protocol checks pass. Drum fidelity covers all six models;
Grids matches 316 cases, the compressor 216 cases with three signals, and
DaisySP 144 cases (36 informational). Target render budgets, pinned loader,
erase ordering, update-source checks and web tests pass.

The unchanged Gate 5 passes: default images are byte-identical with and without
Bluetooth files, 341772 B. The documented default build succeeds without
warnings. Its package is only a local regression artifact, never installed,
uploaded or published.

| Resource | Used B | Capacity B | Free B | Phase 4A delta B |
| --- | ---: | ---: | ---: | ---: |
| Application flash | 341772 | 581564 | 239792 | +108 |
| Static RAM (.data + .bss) | 73292 | 98304 | 25012 | +64 |
| Pool | 307316 | 344064 | 36748 | 0 |
| .noinit | 15576 | 15696 | 120 | 0 |

The ELF places `chain` in .bss (624 B, +64), `proj_slot` in .noinit (15360 B,
unchanged), and the live configuration/snapshot arrays in .bss. Project size
and offset assertions pass on host and target. The 8192 B pool reserve remains
enforced. Stack analysis passes the unchanged 75% limits: main 2400/24320 B,
interrupts 1476/7936 B.

Explicit skips in the existing regression runner remain:

- Thirteen host instruction-budget checks in six groups: comparable Linux
  instruction measurement is unavailable. Functional assertions still run.
- H2/H3 comparisons: no upstream build (standalone stack analysis/self-test pass).
- Installer logical-image and web package comparisons: legacy filename
  `build/felucca.fwsc` absent; the actual documented package is
  `build/felucca-UNTESTED.fwsc`. Loader/package protocol simulations ran.
- Official V15 restoration: `FM-1_v15.fwsc` unavailable.

Modified files this phase: `firmware/src/project.c`, `firmware/src/seq.c`,
`firmware/src/song.c`, `tests/remix_project_test.c`, `tests/remix_seq_test.c`,
`tests/remix_motion_test.c`, `tests/remix_record_test.c`,
`tests/run_remix_tests.sh`, new `tests/remix_song_test.c`, and this document.
The three older focused tests only change their output labels from song bypass
to OFF-slot ownership. The Phase 4A test changes the intentionally superseded
saved-selection bypass expectation; no OFF-equivalence assertion is weakened.

Backup: `/tmp/nauclerus-before-phase4b-20261027-100857`. All prior 17 intent-to-add
entries are preserved; the two new files also have intent-to-add entries (19
total) for Gate 5. No content is staged. All other pre-existing project files
are byte-for-byte unchanged, including DSP, themes, loader, frozen guards and
golden references. Branch/HEAD and main remain unchanged. No hardware writes,
installations, commits, pushes, releases or publication occurred.

## Remaining user-facing work

Resolve the independently reproduced existing DSP sanitizer issue and restore
the unavailable pinned upstream validation reference before claiming complete
validation. Add UI selection/editing controls with safe effective-state updates, clear live
versus song ownership, OFF/default behavior and visible bank/pattern labels.
Validate UI interaction, save/load and song transitions on hardware before any
user-facing release. No UI, MIDI selection controls, theme, recording, generator
or menu-navigation changes are part of Phase 4B.
