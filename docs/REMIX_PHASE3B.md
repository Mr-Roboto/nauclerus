# ORB Remix Phase 3B: chronological recording and editing

Remix is a read-only playback transformation. Recording and editing always address
the original chronological pattern. No inverse mapping, baking or new recording
engine is introduced. Inspection and focused production-code tests found no
recording-coordinate incompatibility, so no firmware source changes are needed.

## Inspected paths

| Existing path | Authoritative write coordinate |
| --- | --- |
| `rec_hit()` / `input_hit()` | `seq_idx`, or the next chronological step past the swung midpoint |
| `keys_on()` / `keys_rec()` | the same nearest-step result; explicit keyboard note |
| `keys_tie()` | the next chronological step after the last recorded step |
| Legato SLIDE | the preceding recorded phrase step, including wrap |
| `motion_capture()` | chronological nearest step; existing bank insertion/replacement |
| `step_grab()` / `step_tap()` / `step_edit()` / `step_accent()` | visible bank plus held grid key |
| `srec_*()` | the stopped Step Record cursor, wrapping by LEN |
| `lock_knob()` / `lock_set()` | the original held step; existing `(track, step, parameter, kind)` bank key |

These paths in `seq.c`, `motion.c`, `ui.c`, `ui_input.c` and `lockbank.c` do not
consult the Remix source mapper for writes. The existing chronological one-shot
retrigger suppression is retained. Held ACID recording stops before overwriting
its phrase start; length cuts retain the existing guard. Grids input recording
and editing restrictions, song REC disarming and song edit gates are unchanged.

Playback still reads the mapped original source for notes, Motion and locks, as
implemented in Phases 2/3A. Editing step 5 changes original step 5, not whichever
source is audible at chronological slot 5. A map may play that original step
later, more than once, or not at all. Disabling Remix reveals the original edited
pattern. Repeated playback never writes the original steps or lock bank.

## Focused validation

```sh
sh tests/run_remix_tests.sh
```

`tests/remix_record_test.c` uses the existing UI/DSP host harness and production
recording/editing functions. It compares the complete original patterns and bank
after 672 OFF/A1/non-identity sessions: six lengths (1/8/16/24/40/64), eight track
positions, drums/ACID and six ON selections plus OFF. Those sessions cover drum
and KEYS recording, ACID NOTE/TIE/SLIDE, Motion, lock creation/replacement, manual
step editing and stopped Step Record. No expectations or golden hashes are
generated or replaced.

576 mapped/materialized production-playback pairs test the recorded result,
including repeats and beyond-LEN sources. The rearranged oracle exists only as
transient host-test data and is never saved to a project. Additional cases check swung midpoint
boundaries, chronological skip consumption, all eight tracks armed, legato wrap,
held phrase-start protection, length cuts/growth, STOP/restart, playback immutability,
Grids bypass and song row bypass. Motion PLAY and bank ordering retain their
existing implementation. Activation remains exclusively through the host-only
test interface; no UI/MIDI/persistence controls are added.

## Gate 5 and protected state

Before editing, tracked changes, the complete index and untracked project source
files were backed up under `/tmp`. Intent-to-add is used only for an explicit
list of confirmed untracked project files, as requested. This exposes working-tree
files to Gate 5's existing `git ls-files` snapshot without staging their content,
creating a commit or changing the Gate 5 implementation. The final validation
report records the exact list and result. Existing index entries are preserved.

The separate frozen-source limitation remains: `frozen-base-10` is unavailable,
and its pinned SHA `731386008fae060ae885f44b4765d172ecfae4b3` is unchanged. No tag,
pin or integrity guard is fabricated, edited or bypassed.

Recording policy is now chronological by design; future work does not require
inverse/source-mapped recording. Remaining work is project persistence and its
compatibility rules, song-owned Remix selections and transitions, activation
boundaries, and user-facing controls only after those paths are validated.

## Firmware and Gate 5 results

The documented `./build.sh` succeeds. Firmware SHA-256 remains
`a3e2ec0edec163ecae0f9a144c70afd1c7a6b4857949335628e80cd6ab7fb488`, identical
to the pre-Phase-3B image. No production source changes were necessary.

| Region | Used bytes | Capacity | Free | Delta vs Phase 3A |
| --- | ---: | ---: | ---: | ---: |
| Application flash | 341236 | 581564 | 240328 | 0 |
| Static RAM, including alignment | 73212 | 98304 | 25092 | 0 |
| Pool | 307316 | 344064 | 36748 | 0 |
| .noinit | 15576 | 15696 | 120 | 0 |

All enforced build limits and the 8192-byte pool reserve remain unchanged.
`sh tools/check_default_build.sh` passes: the complete working tree and the copy
without Bluetooth sources produce byte-identical 341236-byte application images.
The script and Git's existing index content entries are unchanged. No file content
is staged; these 15 project files remain intent-to-add:

- `assets/remix/README.md`
- `assets/remix/remixes.txt`
- `docs/REMIX_PHASE1.md`
- `docs/REMIX_PHASE2.md`
- `docs/REMIX_PHASE3A.md`
- `docs/REMIX_PHASE3B.md`
- `firmware/src/remix.c`
- `firmware/src/remix.h`
- `tests/remix_data_test.py`
- `tests/remix_motion_test.c`
- `tests/remix_record_test.c`
- `tests/remix_seq_test.c`
- `tests/remix_test.c`
- `tests/run_remix_tests.sh`
- `tools/gen_remix.py`

## Final regression status

The original focused Remix data, sequencer and Motion/lock suites pass under
ASan/UBSan. The final recording suite passes its matrix, playback pairs and all
additional chronological recording/editing cases, including active held-note LEN
growth and the LEN=1 phrase-start guard. All prior tests/thresholds are retained.

`sh tests/run_tests.sh` executes the existing drum suite and independent protocol,
loader, storage, tool and web checks. Functional/sanitizer/UI/fidelity checks pass,
and Gate 5 now passes inside this runner as well as independently. The runner
still exits 1 at the known frozen-source guard: `frozen-base-10` is missing.
No remaining Gate 5 file-visibility blocker exists.

Expected gaps: 13 Linux instruction-budget assertions are explicitly skipped
because comparable counting is unavailable; upstream binary/stack comparisons
lack their frozen reference build. Legacy logical-image/web-package comparisons
expect `build/felucca.fwsc` rather than the existing UNTESTED test artifact.
Official V15 restore lacks `FM-1_v15.fwsc`. Current UNTESTED-package update/loader
protocol tests pass against simulated hardware. No hardware was accessed.

Files changed in Phase 3B: `tests/remix_record_test.c`,
`tests/run_remix_tests.sh` (appended test invocation), and this document.
There are no firmware source changes. Prior tracked/untracked work is preserved;
only the explicitly listed new files have intent-to-add index entries, with no
file content staged. Existing index entries and `main` remain unchanged.
