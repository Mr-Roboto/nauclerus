# ORB Remix Phase 2: test-activated sequencer playback

The canonical table and pure mapping API remain unchanged from Phase 1. The
production sequencer now uses `seq_source_step()` only when reading the current
step in `step_fire()` and the next step in `seq_next()`. ACID ratchet lookahead
already calls `seq_next()`, so it uses the same mapping. Mapping reads the complete
stored `step_t`: note, ON, accent, condition, ratchet count, slide and tie.

`seq_idx`, `seq_cnt`, `seq_pos`, division remainders, swing phase, duration, loop
count and recording-grid positions remain chronological. Lookahead wraps the
chronological position by the playing length before mapping. A partial block can
therefore read any of its 16 stored sources. Previous-slide state belongs to the
event actually played. No alternative gate, swing or probability engine is added.

## Activation and intentional limits

Eight private volatile 16-bit runtime snapshots store bank/pattern and ON. BSS
initializes them to A1/OFF. START, STOP and restart clear all snapshots; a focused
test must explicitly activate its selections after START. The only setter is
compiled when both `DRUM_REMIX_TEST` and `DRUM_HOST` are defined. The focused test
translation unit does this; the normal firmware does not. No UI, MIDI, console or
project writer is added, and the release ELF contains no test setter.

Grids-driven tracks (`P_SRC != 0`) and armed/running song chains bypass mapping.
Song rows still read their original chronological source arrays. Song code,
including the prior timing fix, is unchanged by Phase 2.

Motion and locks still use chronological lookup. Their playback is NOT mapped.
Live note/Motion/lock recording is NOT remapped. These incomplete interactions
are intentional test-only limitations, not user-facing behavior. No persistence,
track/project layout change, activation UI or APPLY policy is implemented.
User activation must wait for mapped Motion/locks and recording semantics, song
state ownership, persistence and activation-boundary integration and tests.

## Focused checks

```sh
sh tests/run_remix_tests.sh
```

This runs the Phase 1 canonical data/storage checks plus ASan/UBSan tests using
the actual production sequencer and the explicit host-only setter. It covers:

- OFF/A1 identity and active playback versus physically rearranged stored events.
- Drums and ACID, all eight independent tracks, and ACID on each track.
- Lengths 1/8/16/24/40/64, partial blocks and wrapping, and all ten division rates.
- Mapped notes, accents, rests, repeated sources, ties and previous-slide glides.
- Mapped lookahead for the final ACID ratchet's held gate.
- Chronological swing even when an odd slot repeats an even source; ratchets at
  quarter-step times; chronological probability loops and deterministic RNG.
- Grids and two song rows bypassing active test selections.
- OFF defaults, rejected invalid setters, transport reset, chronological recording
  grid, and the explicitly unchanged Motion/lock coordinate behavior.

An additional temporary differential probe compares OFF playback against the
pre-Phase-2 sequencer and exact pre-Phase-2 host harness. Audio/state fingerprints
match byte-for-byte for 270 cases: 45 models times six lengths. This adds no golden
reference files or thresholds to the repository.

## Firmware measurements

Standard non-Bluetooth `./build.sh` succeeds. Compared with Phase 1 plus themes:

| Region | Used bytes | Capacity | Free | Delta used |
| --- | ---: | ---: | ---: | ---: |
| Application flash | 341232 | 581564 | 240332 | +140 |
| Static RAM, including alignment | 73212 | 98304 | 25092 | +16 |
| Pool | 307316 | 344064 | 36748 | 0 |
| .noinit | 15576 | 15696 | 120 | 0 |

The required pool reserve is 8192 bytes, leaving 28556 bytes beyond the reserve.
Target instruction budgets, loader pin and erase-order checks pass. Standalone
stack analysis reports main 2268/24320 bytes and interrupts 1476/7936 bytes, both
below the enforced 75% limits. These are current-build checks, not comparisons
against the unavailable frozen upstream build. Linux host instruction-count
checks remain explicit skips; target-disassembly budgets are separate checks.

The existing `frozen-base-10` reference remains unavailable. Its pinned SHA is
`731386008fae060ae885f44b4765d172ecfae4b3`; no tag, pin or guard is changed. Full
frozen-source/upstream comparison validation remains blocked.

## Existing regression outcome

`sh tests/run_drum_tests.sh` exits 1 at the unchanged missing-tag guard. All
preceding functional/sanitizer/UI/recording/song/Motion/lock checks pass
(2336 logged assertion lines), including the ten-theme tests. Drum, Grids,
Streams and DaisySP reference comparisons pass. Thirteen Linux instruction-budget
assertions are explicitly skipped in six groups. No thresholds or golden files
were changed. The outer packaged suite was not rerun; no Phase 2 installation
package was created. Frozen-source checks and upstream binary/stack comparisons
remain blocked by the unavailable pinned reference.

Phase 2 files: `firmware/src/seq.c`, `tests/drum_host.h`,
`tests/run_remix_tests.sh`, `tests/remix_seq_test.c`, `docs/REMIX_PHASE2.md`.
All prior local changes are preserved. The host harness only gains the production
mapping include and runtime reset; the focused runner retains its prior commands.
No UI, theme, persistence, Motion, recording or song source is changed in Phase 2.
