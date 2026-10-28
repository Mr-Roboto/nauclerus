# ORB Remix Phase 4A: project persistence

## FDRF extension

The existing FDRF magic and 3840-byte record are unchanged. `rsv_mo` is
510 bytes at project offset 3252; the extension occupies its first 32 bytes.
`song` remains at 3762 and the normal project checksum at 3836.

| Relative offset | Bytes | Meaning |
| --- | --- | --- |
| 0 | 8 | Signature: `4e 41 55 4f 52 42 a5 5a` (`NAUORB` plus two markers) |
| 8 | 1 | Version: 1 |
| 9 | 1 | Selection payload length: 16 |
| 10 | 1 | Track count: 8 |
| 11 | 1 | Reserved: must be zero |
| 12 | 16 | Eight explicitly little-endian uint16 selections, track 1 first |
| 28 | 4 | Little-endian FNV-1a-32 of extension bytes 0..27 |
| 32 | 478 | Reserved, saved as zero |

Selection 0 means OFF. Codes 1..256 mean A1..P16 in canonical bank-major
order: `1 + bank * 16 + pattern`, with bank/pattern indices 0..15.
All OFF saves omit the extension entirely (zero bytes), preserving byte-exact
legacy FDRF OFF records and their checksums. Explicit valid all-OFF metadata
also decodes OFF. No native structs or unaligned integer reads encode metadata.

The normal FDRF checksum covers every extension byte, including its inner hash.
The inner hash distinguishes unsupported/random reserved data from a valid
extension after the outer record has been validated. This is corruption
detection, not authentication. The complete header, inner checksum and all eight
selection ranges must validate; otherwise all eight selections load OFF.
Absent, unsupported or malformed metadata does not reject an otherwise valid
project. A bad outer checksum still follows the existing corrupt-record path.

## Compatibility and ownership

FDR1 through FDRE conversions clear the reserved area, so they load OFF.
Existing FDRF records with empty reserved bytes load OFF. Old firmware ignores
the extension and will drop it on resave. There is no new project version,
flash allocation, lock-bank format, step spare-byte use or .noinit allocation.
The lock bank is still saved before its project record; the record remains the
transaction commit. Interrupted saves preserve the previous matching pair,
including its Remix selections.

`seq_remix_config[8]` holds project configuration (16 B of ordinary .bss).
`seq_remix[8]` remains temporary playback state. STOP clears only temporary
state; PLAY/restart rebuilds it from retained configuration. Track/default
initialization clears both. LOAD restores configuration under the existing IRQ
protection and stop request, and activates it on the next PLAY. The existing
SAVE-during-transport and LOAD-during-chain guards remain intact. Boot restores
the last project through the existing load path; safe boot stays OFF.

`proj_remix_get(project, track)` validates a complete record and reads any slot
without changing the edited project. This is the shared decoder for Phase 4B;
no song-row activation is added here. Song-chain and Grids playback still bypass
Remix. Recording/editing still use chronological positions; playback uses the
existing source mapping. No UI or MIDI selection controls are added. A valid
saved/imported project can restore selections, as intended by persistence.

## Validation

`sh tests/run_remix_tests.sh` runs the production mapper, sequencer,
Motion/lock, recording/editing and project/flash/boot tests with ASan/UBSan.
Project coverage includes all 256 selections plus OFF on eight tracks, A-D
independence, all supported legacy conversions, malformed metadata, corrupt
outer records, power-on/safe boot, transport retention, defaults, song bypass,
Motion/lock preservation, unchanged steps and interrupted bank/header/project
writes. Only prior tests asserting transport clears configuration were updated;
OFF-equivalence assertions, golden references and thresholds are unchanged.

An additional local differential probe compiles the same OFF fixture against
the pre-Phase-4A backup and the new tree. Across all models, six lengths and four
slots, raw saved project records and rendered PCM match byte for byte:
30,689,280 bytes, SHA-256
`b599611a6ae88ee5fb11347110fe2d2658c30fbdecf48ec6405b8b5257321c40`.
This comparison does not regenerate any golden reference.

Default firmware build and unchanged Gate 5 passed. Gate 5 reports byte-identical
341664-byte images with and without the Bluetooth sources. Target ELF sections
confirm the configuration array is 16 B in .bss; `proj_slot` is still 15360 B in
.noinit. The record size/offset assertions pass in both host and target builds.

| Resource | Used B | Capacity B | Free B | Delta from Phase 3B |
| --- | ---: | ---: | ---: | ---: |
| Application flash | 341664 | 581564 | 239900 | +428 |
| Static RAM (.data + .bss) | 73228 | 98304 | 25076 | +16 |
| Pool | 307316 | 344064 | 36748 | 0 |
| .noinit | 15576 | 15696 | 120 | 0 |

All memory limits remain intact, including the 8192 B minimum pool reserve.
Standalone stack analysis passes: main 2268/24320 B, interrupts 1476/7936 B,
both below the existing 75% limits. The firmware build reports no warnings.

`sh tests/run_tests.sh` completed with status 1 solely at the existing
frozen-source guard: upstream tag `frozen-base-10` is unavailable, pinned to
`731386008fae060ae885f44b4765d172ecfae4b3`. Its pin and guards are unchanged;
the drum runner's final frozen-source validation remains blocked. All executed
functional, sanitizer, boot, UI, Motion/lock, song, ACID and fidelity tests passed.
Fidelity includes all six drum models, 316 Grids cases, 216 compressor cases
with three signals, and 144 DaisySP cases (36 informational). Target render
budgets, loader pin, erase ordering, update-source checks and web tests pass.

Explicit skips are unchanged:

- Thirteen host instruction-budget checks in six reported groups: comparable
  instruction counting is unavailable on Linux. Functional assertions still run.
- H2/H3 upstream comparison: no upstream build is available (standalone stack
  analysis and its self-test pass).
- Installer logical-image and web package comparison: their legacy
  `build/felucca.fwsc` filename is absent; the documented local test package is
  `build/felucca-UNTESTED.fwsc`. Package/loader update-protocol simulations ran.
- Official V15 restoration: `FM-1_v15.fwsc` is unavailable.

No golden references, thresholds or integrity checks were changed. The original
15 intent-to-add entries were preserved exactly; the two new Phase 4A files
also have intent-to-add entries so Gate 5 sees the complete tree. No file content
is staged. No commits, pushes, hardware writes or publication were performed.

## Remaining Phase 4B work

Use the existing per-slot decoder for song-owned selections, establish row
transition ownership and reset behavior, and test those transitions with the
row-owned steps and Motion/lock banks. Song activation is intentionally absent.
UI/MIDI selection controls and their validation remain later work.
