# Canonical ORB Remix data

`remixes.txt` is imported byte-for-byte from Mr-Roboto/fm1-x0x-rmx,
`v1.1.0`, commit `a4e78f3a6e16fa310b7614e5876d07e16a0020f9`:
https://github.com/Mr-Roboto/fm1-x0x-rmx/tree/v1.1.0/assets/remix

It retains all historical 256 rows in bank-major order A1 through P16,
16 zero-based source steps per row. Repeats retain their historical order.
The historical Luminary import converted source values 1..16 to 0..15 only.
SHA-256 of the 4096 decoded bytes (not the text file):
`88034625d620ce4a1a87f954edf81850adf261155a7891fabfea68b01b6d60cb`.

`tools/gen_remix.py` derives from the same tag, validates the structure,
identity A1, zero first entries and pinned digest, then generates
`build/gen/nauclerus_remix_table.h`: 2048 bytes of `static const uint8_t`
data, even slots in the low nibble and odd slots in the high nibble.
No unpacked table is stored in the firmware or copied to writable RAM.

`firmware/src/remix.h` documents the pure mapping API. The Phase 1 notes describe the original data-only stage; they are historical.
Current Nauclerus includes sequencer, mapped Motion/locks, project/Song persistence and
the REMIX page. See the [current user guide](../../docs/USER_GUIDE.md#orb-remix)
for global Drum/independent Bass ownership and controls, and
[the Phase 1 notes](../../docs/REMIX_PHASE1.md) for the initial verification record.

## Attribution record

**Current owner declaration, 2026-10-28:** Mr. Roboto explicitly confirms that the
ORB Remix pattern data were independently created by him. Future Retro Orb provided
conceptual inspiration, not copied pattern data. He authorizes redistribution of
his own data in Nauclerus. The existing GPL-3.0-only data/source notices are retained.
Future Retro is credited for inspiration only; no endorsement or affiliation is implied.

This declaration corrects and supersedes the earlier provenance wording describing
recording/transcription of the physical Orb's patterns. That older wording remains
in the read-only pinned X0X-RMX reference and in historical audit artifacts; it is
not treated as the owner's current account. The `Luminary.ksp` import comment records
an intermediate artifact in the data's history, not proof of a different author or
third-party pattern source.

Chain of custody is verified: this text matches pinned X0X-RMX v1.1.0, all 4096
values retain the published decoded SHA-256 above, and the production nibble table
is derived from those exact values. Hashes establish identity/derivation; authorship
and permission are established here by the explicit owner declaration, not inferred
from availability. No independent contrary data source, device dump or third-party
source artifact has been identified. There is no continuing redistribution blocker
based only on conceptual inspiration. See [the updated audit](../../docs/ASSET_AUDIT_0.25.0.md).
