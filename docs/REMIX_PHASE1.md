# ORB Remix Phase 1: data and pure mapping

Canonical data is imported unchanged from `Mr-Roboto/fm1-x0x-rmx` tag
`v1.1.0`, commit `a4e78f3a6e16fa310b7614e5876d07e16a0020f9`.
See [data provenance and packing](../assets/remix/README.md).

`firmware/src/remix.c` exports checked lookup and source mapping, returning `-1`
for invalid indices. Bank/pattern are 0..15; chronological positions are 0..63.
Each 16-step block maps independently; length limits chronological slots only,
never the available source steps in a partial block. A1 is identity.

The normal generator makes `build/gen/nauclerus_remix_table.h`. The app's existing
single translation unit includes `remix.c`; no separate linker path is needed.
The packed table is `static const`, 2048 bytes, placed in flash/XIP by `app.ld`.
There is no writable/unpacked copy or initialization. Phase 1 makes no calls from
the sequencer, song, Motion, recording, storage or UI: the feature remains inactive.

Run focused Linux tests with existing Python, C compiler and GNU binutils:

```sh
sh tests/run_remix_tests.sh
./build.sh
```

Tests compare all production lookups against the canonical import, pin decoded
SHA-256, unpack/repack all nibbles, check A1/repeats/all four blocks, lengths
1/8/16/24/40/64, invalid API inputs and malformed generator input. The C checks
use ASan/UBSan and the actual production module. An ordinary object inspection
requires a 2048-byte read-only table and no writable static storage. These focused Linux tests run separately; the existing drum runner and its
guards are unchanged, including its macOS path.

Validation limitation: local tag `frozen-base-10` is unavailable. The existing
guard pins it to `731386008fae060ae885f44b4765d172ecfae4b3`. Frozen-source validation
remains blocked; do not create a substitute tag, change the pin or bypass guards.
Linux host instruction-count checks remain explicit skips; macOS behavior is
preserved. The pre-existing song timing fix and Linux harness changes are retained.

## Firmware measurements

Standard non-Bluetooth `./build.sh`, compared with original commit
`2690e74aba6fb99eeb547010e65e52cc2da395e9`. Deltas include the retained local
song timing fix as well as Phase 1.

| Region | Used (bytes) | Capacity | Free | Delta used |
| --- | ---: | ---: | ---: | ---: |
| Application flash | 341020 | 581564 | 240544 | +2144 |
| Static RAM (.data + .bss including alignment) | 73196 | 98304 | 25108 | 0 |
| Pool | 307316 | 344064 | 36748 | 0 |
| .noinit | 15576 | 15696 | 120 | 0 |

The enforced pool reserve is 8192 bytes, leaving 28556 bytes above the reserve.
The target ELF has one `remix_packed` symbol, size 0x800, in XIP `.text` at
0x0202d56c. The build's memory/entry/ROM/RAM-code checks and loader pin pass.
No loader code or pin is changed.

## Phase 1 regression result

Focused tests passed independently (and at the start of the regression run), including
ASan/UBSan and ordinary-object read-only storage inspection. All preceding drum
functional, sanitizer, UI, song, recording, lock, Motion and reference fidelity
checks passed. Drum, Grids, Streams compressor and DaisySP comparisons passed;
DaisySP reports 144 cases (36 informational), zero failures. The runner exits 1
at the unchanged integrity guard because `frozen-base-10` is missing. This is
not a complete suite pass. No tag or guard was altered. The outer packaged suite
was not rerun; no Phase 1 installation package was created.

Linux instruction budgets: 13 assertions explicitly skipped in six groups;
no CPU-measurement pass is inferred. Target-disassembly render budgets, loader
pin and erase-order checks passed separately. macOS execution was not available;
all pre-existing portability files are byte-identical to the pre-Phase-1 snapshot.

Phase 1 files (pre-existing local changes are additional and preserved):

- `assets/remix/remixes.txt`
- `assets/remix/README.md`
- `firmware/src/remix.c`
- `firmware/src/remix.h`
- `firmware/src/felucca.c`
- `tools/gen_remix.py`
- `tools/build.py`
- `tests/remix_test.c`
- `tests/remix_data_test.py`
- `tests/run_remix_tests.sh`
- `docs/REMIX_PHASE1.md`
