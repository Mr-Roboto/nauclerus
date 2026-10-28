# Accepted RC13 target-cost reference

The complete release runner exposed two inherited expected costs predating the
accepted PULSE/MORPH and Song-owned Remix implementation: chain_apply 174 versus
202, and acid_render 326 versus 540. Every other target entry passed.

An independent detached checkout was populated from the preserved RC13 source
snapshot SHA-256 6ee83df94e3e074bed321c49f0427b1f44116c17cbaeeaa77909a66d969eb406.
Its VERSION is 0.25.0-rc.13. The existing verified target toolchain rebuilt the
358196-byte application. The accepted RC13 package was first verified against
the owner's SHA-256 6d4af7cc05a93337d8ed2b9b51d7e25afa1b27688cac4e85155b882b766ed167,
then its application bytes extracted using the repository's package format.
The independently rebuilt application matches every accepted application byte
exactly, including the build date. No source or identity was substituted.

All 53 render/ISR/sequence function metrics in the stable candidate are identical
to that independently rebuilt RC13. Specifically:

| Function | Accepted RC13 and stable instructions | Loop instructions | Divides / calls | Weighted cost |
|---|---:|---:|---:|---:|
| chain_apply | 159 | 117 | 8 / 2 | 202 |
| acid_render | 851 | 540 | 0 / 3 | 540 |

Only these two expected values in tests/target_budget.txt are rebased to accepted
RC13. The algorithm, nesting/divide weights, +10% tolerance and all other 51
entries remain unchanged. An independently lowered temporary ACID reference
still fails the unchanged over-budget check; it is not an accepted baseline.
See the adjacent JSON for full metrics, artifact hashes and empty byte-difference
list. The original RC13 application and every prior package remain preserved.

This establishes no target-cost regression from accepted RC13. It does not turn
static instruction estimates into measured real-time cycle counts. RC13's owner
hardware acceptance and host dense-mix tests are separate evidence; new worst-case
hardware timing certification and Linux retired-instruction counting are not
claimed. No ACID DSP, filter, preset, envelope, modulation arithmetic, or musical
timing code changed during release preparation.
