# Nauclerus local security baseline adoption

Adopted by explicit owner authorization in “Security Baseline Adoption and First
Release Candidate”, 2026-10-27. This is a local attestation, not a cryptographic
signature or authentication of historical authors. No remote publication or device
installation is implied. The working application remains uncommitted as requested.

## Immutable reference

- Name: `nauclerus-security-base-1` (annotated local tag).
- Tag object: `07462232c82f044bb5a50cf46d16a9ed2a333260`.
- Commit: `2690e74aba6fb99eeb547010e65e52cc2da395e9`.
- Tree: `3b0583347cc1e91e7870b0051fe4256e0066055d`.
- Canonical protected manifest SHA-256:
  `b55fc9404167b9098314a2af508bbf40f2d36fc8fc5a7f82ad020f6bccfe8738`.
- Loader SHA-256, unchanged:
  `cc98eed224299fa42ca22a546073e0f8b98d8362e26b5f325a9aaeab287bdf5d`.
- Locked compiler/SDK input descriptor SHA-256 (canonical JSON):
  `c9926adb5470fd653f0212596c4f2bbc28540cb60a3bafc73c0a9b5a94bc7160`.
- Canonical seven-output reference digest (two independent builds):
  `b01c312d9906257842205fc13a20c82ba5a662207c86b3c826fd0f17317edf65`.
- Independently built reference application SHA-256, 338876 bytes:
  `31f6c94ed3e1645e8ec321e79460f679d76ac511ae5b1a15b42b4474449ca4b2`.

Canonical JSON means UTF-8 `json.dumps(value, sort_keys=True, separators=(',', ':'))`.
The protected digest covers `files` in tools/security_baseline.json. It is bound
into the tag annotation and strict resolver, rather than derived from current
application files. Git blob IDs, file modes and every recorded SHA-256 are also
checked against the pinned pre-Remix commit before comparing working files.

## Independent evidence and authorization scope

The original proposal remains the detailed per-file provenance ledger:
[NAUCLERUS_SECURITY_BASELINE_PROPOSAL.md](NAUCLERUS_SECURITY_BASELINE_PROPOSAL.md).
Its 33 hashes and exact historical deltas were reverified before adoption, together
with the prior 139 preparation fixtures. Available frozen-base-2..7 objects provide
the independent lineage. The recorded backlight hook, application USB name and
lock-bank allocation are confirmed in ed9f0dd, c2264cc and 70f44a5. The three new
BLE HAL files equal their 59c220f addition blobs. The boot-guard tail is unchanged.

The owner accepts these available unsigned objects and the reviewed bounded deltas
as the Nauclerus trust basis. This does not authenticate unavailable external
727f272/fm1-lsdj material or claim recovery of the original frozen reference trees.
The missing original `frozen-base-10` pin was
`731386008fae060ae885f44b4765d172ecfae4b3`; it has NOT been fabricated. Earlier missing
-8/-9 objects and all previous references remain untouched. Only the separately
named Nauclerus baseline was created. main and the development branch did not move.

## Stronger operational checks

- Raw byte, mode, type and complete directory-inventory checks for HAL/loader;
  unexpected untracked additions and symlinks fail.
- All 29 prior byte-frozen files remain frozen. Complete main.c/storage.c and
  libc.c are also byte-frozen. No general init/title/ST_MAGIC exception remains.
  The former ST_MAGIC-positive fixture now requires rejection under this explicitly
  approved stronger policy. Existing OTA/USB/recovery/moved-pin negatives remain.
- core.h retains its exact boot suffix and single canonical magic/declaration;
  duplicate/undef substitutions fail. Its ordinary body remains application code.
  The linked bootguard must be exactly one 12-byte .noinit object.
- The reference name, annotation object, commit, tree, schema, canonical manifest
  and loader pin are mandatory. Missing/moved refs, changed annotation on the same
  commit, altered/duplicate JSON or pin records and missing objects fail closed.
- 156 production filesystem/Git fixtures supplement the original preparation tests.
  Git tampering occurs only in disposable fixture repositories with read-only object
  alternates. No real checkout/index/ref tampering is used to test the guard.
- Reference builds use `git archive` of the exact commit, not the current working
  tree. Locked existing compiler/SDK hashes, complete source blob/mode verification
  and output receipts replace the old HEAD-only cache. Stale/injected source or
  modified outputs fail, including jointly modified output/receipt pairs. No forced worktree removal or alternate baseline fallback.
- H2 comparison allowances, inlining facts, loader checksum, audio goldens and resource limits
  are unchanged. The first complete run exposed an old self-test assumption: a
  byte-identical caller bypassed the wrong-PTR_ARG check. The comparator now also
  proves each pointer binding against the pinned frozen source before accepting
  identical machine code. The existing negative expectation is retained and passes;
  this adds a rejection condition rather than an inlining allowance. Comparative H3 fails if its baseline cannot be analyzed. The full
  runner cannot succeed when its required reference receipt is unavailable.
- Offline package validation checks UFW bounds/CRCs, exact embedded application
  and loader, nested application CRCs, version/product and target canonical table.
  Corruption fixtures include repaired outer CRCs around a changed loader.
- Real production project/1024-lock saves are checked against the real loader's
  record-cleanup scan in a simulated flash image. This covers normal FDRF saves,
  not every possible hostile or legacy arbitrary payload.

Baseline builds preserve the original committed source, including its historical
tool comments/pins; those are not rewritten to pretend frozen-base-10 exists.
The new current-tree resolver is the adopted authority. The loader built from
both trees must retain the same existing checksum.

## Release identity and limits

Current candidate identity: **Nauclerus 0.25.0-rc.1**. Boot text and ABOUT/console
version use Nauclerus. USB descriptors, hardware IDs, `FM-1_900`, compatibility
markers and loader/recovery code remain untouched. Existing offline test package
paths are byte-identical copies of the newly named package, not alternate builds.

This baseline preserves the existing trusted-host update model. Marker checks,
CRC and device-key compatibility checks are not publisher authentication or secure
boot. Physical power-loss resume, recovery and musical acceptance remain untested.
Unavailable Linux instruction counters and stock V15 restoration artifacts must
remain explicit skips. Passing static target budgets does not replace them.

All adoption/tool/source changes remain locally reviewable and uncommitted. An
exact source archive, artifact/loader hashes, baseline bundle and validation logs
accompany the final candidate; these identify this dirty snapshot without a release
commit. Any publication or device access requires separate authorization.

## Approved identity exception — NAU-0250-015

The owner subsequently approved application/package FM-1_25000 and USB product
Nauclerus. The original immutable commit, tree, tag object, manifest and loader pins
above remain unchanged. The source guard normalizes only the exact application
STR2 declaration/context (18-byte Hortator -> 20-byte Nauclerus) before comparing
to the independent original file hash. Any other descriptor/USB change still fails.
No ota.c source or loader bytes change. H2 checks the exact new/reference identity
objects and allows only the corresponding ID literal and copy-bound 7->9 in
ota_reply_identity, with every other normalized instruction identical. The reply
width stays27 and the protocol/checksums/bounds remain unchanged. Added negative
fixtures reject changed names, lengths, PIDs, loader names, extra edits, duplicate
descriptors and altered identity reply widths/bounds.
