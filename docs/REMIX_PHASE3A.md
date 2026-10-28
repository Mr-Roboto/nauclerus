# ORB Remix Phase 3A: existing Motion and lock playback

The existing `motion_step()` now distinguishes its chronological argument from
`source = seq_source_step(t, chronological)`. This is the same Phase 2 mapper used
for drum and ACID events; there is no second mapping algorithm or event bank.

Chronological positions continue to own timing, transport, swing, lengths and
loop boundaries. Only chronological zero restores Motion to its base. Source
zero at another chronological position does not reset held Motion. The mapped
source owns the existing lock-bank binary search and the step's ON eligibility.

Every chronological entry first releases the prior locks through the original
`lock_release()`. The existing sorted bank still applies Motion before a lock on
the same knob, saves the underlying held value, and reapplies LFO modulation.
Repeated sources release/reapply locks rather than stacking lock values. STOP,
restart and row transitions continue using the existing restoration paths.

The shared 1024-event bank, per-track PLAY bits, clamping, ON-only eligibility,
song model filtering and held-value Motion engine are unchanged. In particular,
Motion still holds until another event or chronological wrap: no interpolation.
Grids and armed/running song chains use the Phase 2 identity bypass. Song rows
retain their own source steps and banks. No project or bank format changes.

## Recording and activation limits

Live note recording, Motion capture, lock editing and Step Record retain their
chronological destinations. This phase does not remap recording. Remix activation
is still possible only through the explicit focused host-test interface; it is
absent from normal firmware. There are no UI/MIDI controls, persistent Remix
fields, song-chain selections or activation-boundary/APPLY behavior.

Phase 3B needs explicit Remix-aware recording semantics and tests. Persistence,
song selection ownership and user activation remain later work. The feature must
remain unavailable to users until those interactions are defined and validated.

## Focused validation

```sh
sh tests/run_remix_tests.sh
```

The runner includes the original canonical-data and Phase 2 sequencer checks,
plus ASan/UBSan production-code Motion/lock tests for:

- OFF and A1 identity; active mapping compared with rearranged stored events and
  keys, using the existing production engine for both cases.
- Source zero at nonzero chronology, held values between events, and wrap reset.
- Beyond-LEN sources, mapped ON eligibility, locks on probability-skipped ON
  events, dormant OFF locks, and Motion PLAY OFF with locks still active.
- Motion-before-Lock on one parameter, release to held Motion, repeated locks,
  STOP and restart, and locks on the final event of a partial-length loop.
- Eight independent selections with all six lengths (1/8/16/24/40/64), all four
  blocks, and a full existing 1024-event bank (512 Motion plus 512 lock events).
- LFO modulation around the lock, incoming-step reapplication and restoration;
  the UI accessor is checked after restoring knobs, matching its existing use.
- ACID ties, slides, rests, mapped Motion and lock eligibility.
- Grids bypass, two song rows with separate banks, row restoration and existing
  model-mismatch filtering of engine parameters.
- Unchanged chronological live-note recording in the extended Phase 2 check.

All focused checks pass. An independent temporary differential probe compares
OFF against the pre-Phase-3A Motion code: 810 audio/state fingerprints match
(45 models x six lengths x Motion/lock/LFO modes), including STOP restoration.
No new golden references or threshold changes are involved.

## Firmware measurements versus Phase 2

Standard non-Bluetooth `./build.sh` succeeds, as does the documented local test
package build (`DRUM_PACKAGE=1 ./build.sh`) for the outer host regression runner.
The test artifact is local only and is not a release.

| Region | Used bytes | Capacity | Free | Delta used |
| --- | ---: | ---: | ---: | ---: |
| Application flash | 341236 | 581564 | 240328 | +4 |
| Static RAM, including alignment | 73212 | 98304 | 25092 | 0 |
| Pool | 307316 | 344064 | 36748 | 0 |
| .noinit | 15576 | 15696 | 120 | 0 |

All enforced build limits and the unchanged 8192-byte pool reserve pass. Target
instruction-budget analysis and loader pin pass. Standalone stack analysis passes
the 75% limits: main 2268/24320 bytes; interrupts 1476/7936 bytes. These checks do
not substitute for unavailable frozen-upstream comparisons or hardware tests.

The pinned `frozen-base-10` reference remains unavailable; required commit
`731386008fae060ae885f44b4765d172ecfae4b3` is unchanged. No reference tag, pinned
SHA or integrity guard is created, altered or bypassed.

## Existing regression results and blockers

`sh tests/run_tests.sh` ran the existing drum suite, including Motion, lock,
ACID, song and sequencer regressions, then the independent protocol/tool/web
checks. Functional, sanitizer and reference fidelity checks passed. Grids reports
316 cases x 96 steps identical; Streams reports 216 cases x 3 signals x 4096
samples identical; DaisySP reports 144 cases (36 informational), zero failures.
The ten-theme UI tests also passed. No existing thresholds or golden files changed.

The full runner exits 1; this is not a complete regression pass:

1. The drum suite ends at the unchanged frozen-source guard because
   `frozen-base-10` is absent. Frozen-source and upstream binary/stack comparisons
   remain unavailable; the upstream comparison build is absent.
2. Gate 5 uses `git ls-files` to construct its scratch tree. Phase 1 files remain
   untracked, so that tree omits `tools/gen_remix.py` and cannot build. Normal
   builds of the complete working tree succeed. The index/guard was not altered
   to conceal this limitation.

Thirteen Linux host instruction-budget assertions are explicitly skipped in six
reported groups because comparable counting is unavailable. Target-disassembly
budgets are separate and passed. The logical-image and web package-specific
comparisons expect legacy `build/felucca.fwsc`, while the documented local artifact
is `build/felucca-UNTESTED.fwsc`, so those comparisons are skipped. Official V15
restore is skipped because `FM-1_v15.fwsc` is absent. Installer/update tests that
use the current UNTESTED package pass against simulated hardware.

Phase 3A files: `firmware/src/motion.c` (functional change), `firmware/src/seq.c`
(comment only), `tests/remix_seq_test.c` (Phase 2 assertion adapted to the new
playback semantics; recording check retained), `tests/remix_motion_test.c`,
`tests/run_remix_tests.sh`, and `docs/REMIX_PHASE3A.md`. Prior local work is
preserved. No UI, theme, song, lock-bank, project or recording source was changed.
