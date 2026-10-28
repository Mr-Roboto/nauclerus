# Phase 4C: signed-shift correctness in the master filters

## Change and root cause

Only `firmware/src/fx.c` changes production behavior at the C-language level.
The three expressions changed are:

| Function | Before | After |
| --- | --- | --- |
| dc_block | `x << 6` | `x * 64` |
| dc_block | `d << 12` | `d * 4096` |
| lowcut1 | `d << sh` | `d * (int32_t)(1u << sh)` |

Left-shifting a negative signed value is undefined C behavior. Negative audio
makes `x` negative, and a falling DC/low-pass estimate legitimately makes `d`
negative. The original Phase 4B ASan/UBSan Song test reproduced the exact failure
at pre-edit `fx.c:117`: `d << sh` in lowcut1, with negative value -1.

The new operations multiply by exactly the same powers of two. `sh` is always
5 or 6 at the only production call site. Its positive unsigned scale is cast to
int32 before multiplication to retain signed arithmetic. Existing arithmetic
right shifts, rounding bias, state updates and error-feedback order are intact.
No saturation, new division, float, 64-bit target arithmetic or extra state is
introduced. No Remix, Song, project, UI, themes or loader code is changed.

## Range and mathematical checks

The mixer's full-master clamp produces at most 524287 * 4 = 2097148 in magnitude
before PERFORM/METRO. The focused scalar test uses a conservative wider envelope
`X = 8388607`. Starting from a valid state and error 0..4095, the DC estimate
stays within +/-64X = 536870848; its update moves toward the scaled input.
Thus `e = 64*x - state + error` is between -1073741696 and 1073745791, safely
inside int32. Its rounded output has magnitude at most 2X. The first and second
lowcut stages have conservative input envelopes 2X and 4X, with differences
at most about 4X and 8X plus the 0..63 remainder (a coefficient switch can add
one state LSB). These bounds are far inside int32. The focused test exercises
lowcut inputs up to +/-4X and both coefficient settings with existing state.

For either scale S = 4096, 32 or 64, `d` is floor(e/S). Consequently
`e - d*S` is exactly the original remainder in 0..S-1. Since INT32_MIN is a
multiple of each S, d*S is representable for every int32 e. Multiplication
therefore introduces no signed overflow in the supported range. The test uses
an independent int64 floor-division model, explicitly asserts intermediate
ranges and compares every sample, state and remainder. A defined unsigned-shift
model also verifies the former pi32v2 two's-complement shift results, including
negative values; no undefined signed shift is used in that reference model.

## Tests and bit equivalence

New files: `tests/fx_filter_test.c`, `tests/run_fx_filter_tests.sh`, this document.
Command: `sh tests/run_fx_filter_tests.sh` (after the standard build generates
headers). The runner enables ASan/UBSan with no exclusions and no recovery.

The scalar tests cover 2 million samples: positive/negative constants, silence,
unit/rounding boundaries, nominal Q15 endpoints, the full-master clamp boundary,
conservative headroom endpoints, alternating signs, random input, coefficient
changes, DC decay, state transitions and a 64-bit model. Additional lowcut
runs verify the exact accumulated error-feedback identity for both signs and
both coefficients.

Speaker tests cover 600000 frames per initial mode FLAT/LOWCUT/BASS+ (1.8 million
stereo frames total), with positive/negative DC, opposing channels, endpoint
alternation, noise, mode changes with existing states, and a long silence tail.
FLAT and LOWCUT settle exactly to zero. The unrelated quantized BASS+ poles keep
the same steady 77/77 tail in this fixture as before the fix; the test preserves
that behavior rather than redefining the BASS+ engine. No existing thresholds
or golden references change.

Normal pre/post host builds match these PCM/filter-state FNV64 fingerprints:

| Initial EQ | Fingerprint |
| --- | --- |
| FLAT | 8417f2b80f781e4b |
| LOWCUT | 1c003bfe17e7221d |
| BASS+ | d12d66923a647392 |

An additional pre/post capture compares the actual stereo PCM byte for byte:
14400000 bytes (1800000 stereo int32 frames), SHA-256
`ab96cf01cbb4756e70097272da179c5abb552730d1b517a746901c10e9587b17`.
The captures are temporary differential artifacts, not regenerated golden files.

The complete target application is byte-identical before/after the source fix:
341772 bytes, SHA-256
`5cb05edcf2fa7fc38b6e9d07327cfd8ab2b4d4e711458228694d30e9d67e504a`.
The compiler already generated the intended machine shifts; the fix makes the
source arithmetic defined without changing any target instruction or audio bit.

The unchanged `sh tests/run_remix_tests.sh` passes all Phase 1 through 4B checks,
including the original failing full-audio ASan/UBSan test. No new sanitizer
failure occurs. Target instruction-budget output is identical before/after.

## Build, regressions and preservation

The documented default build (`DRUM_PACKAGE=1 ./build.sh` for the local regression
artifact) succeeds without warnings. No package is installed or published.

| Resource | Used B | Capacity B | Free B | Phase 4B delta B |
| --- | ---: | ---: | ---: | ---: |
| Application flash | 341772 | 581564 | 239792 | 0 |
| Static RAM (.data + .bss) | 73292 | 98304 | 25012 | 0 |
| Pool | 307316 | 344064 | 36748 | 0 |
| .noinit | 15576 | 15696 | 120 | 0 |

The 8192 B pool reserve remains enforced. Stack use remains main 2400/24320 B
and interrupts 1476/7936 B, within the unchanged 75% limits.

The unchanged Gate 5 passes: default images with/without Bluetooth files are
byte-identical, 341772 B. This final image also matches the saved pre-fix target
image exactly. Target instruction-budget checks pass with identical output
before/after (zero instruction/cost delta).

`sh tests/run_tests.sh` completes with status 1 solely at the existing frozen
guard: `frozen-base-10` is unavailable, pinned to
`731386008fae060ae885f44b4765d172ecfae4b3`. Its tag, pin, integrity checks and
loader checks are unchanged. All executed functional, existing sanitizer, UI,
Song, ACID, Motion, lock, boot, fidelity, loader/protocol, target-budget and web
checks pass. Fidelity includes all six drum models, 316 Grids cases, 216
compressor cases with three signals, and 144 DaisySP cases (36 informational).
No additional unrelated sanitizer failure was found.

The following existing checks are explicitly skipped, not reported as passed:

- Thirteen CPU instruction-budget checks in six reported groups: comparable host
  instruction counting is unavailable on Linux; functional assertions run.
- H2/H3 upstream comparisons: no upstream build. Standalone stack checks and the
  H3 self-test pass.
- Installer logical-image and web package comparisons: the legacy
  `build/felucca.fwsc` filename is absent. The documented local package is
  `build/felucca-UNTESTED.fwsc`; loader/package simulations ran.
- Official V15 restoration: `FM-1_v15.fwsc` is unavailable.

The complete pre-edit project/index backup is
`/tmp/nauclerus-before-phase4c-20261027-103919`. All prior work and 19 existing
intent-to-add entries are preserved exactly. The three new test/report files
have intent-to-add entries too (22 total), for the complete Gate 5 snapshot;
no content is staged. All other pre-existing project files are byte-identical
to the backup, including every Remix/Song test, threshold, golden reference,
theme and integrity check. Branch HEAD and main are unchanged. No branch switch,
reset, stash, commit, push, hardware write, dependency installation or release
is performed.
