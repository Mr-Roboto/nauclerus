# Nauclerus 0.25.0 asset and publication audit

Status: **PASS with the documented optional-image exclusions**, updated 2026-10-28 for RC12. Canonical project:
[Nauclerus](https://github.com/Mr-Roboto/nauclerus),
[releases/downloads](https://github.com/Mr-Roboto/nauclerus/releases).
This is a distribution-evidence audit, not a ruling that a particular pattern is
copyrightable or that infringement has occurred. Permission is not inferred from
public availability, a successful checksum or possession of hardware.

The complete [per-file inventory](ASSET_AUDIT_0.25.0.json) records paths, sizes,
SHA-256 values, notices, provenance, runtime use and disposition for all local
`assets/` files, the web font/notices, documentation bitmap assets, generated
sample material and the three required SDK binary payloads. The current inventory
contains44 entries. License conditions/notices must accompany any later distribution.

## Findings

| Material | Evidence and conclusion | Disposition |
| --- | --- | --- |
| `assets/remix/remixes.txt` and its generated nibble table | Explicit current owner declaration: independently created data, conceptual Orb inspiration only, Nauclerus redistribution authorized. Exact pinned X0X import and generated table/digest chain verified. | Supported under the retained GPL-3.0-only notice. Earlier transcription wording is superseded; no speculation-based blocker remains. |
| Hügelton generator/sample pack | Independent rights-holder commit5738617 explicitly offers the generator and its sounds as CC0. Local and granted generator executable ASTs are identical after removing docstrings; remaining differences are notices/comments/docstrings. | Supported; preserve sounds/functionality. Local source headers remain untouched; newer permission is documented independently. |
|22 Versilian WAV files and provenance records | Named VSCO-2 CE/VCSL sources, local per-file attribution, and the two primary repositories' explicit CC0 licenses. | Supported under CC0; keep attribution and legal text. Existing clipping/processing does not require replacing notes/sounds. |
| Terminus BDF | Internal copyright/OFL notice, bundled OFL text and the author's primary site agree on OFL1.1. | Supported; retain name/notices/license. Not the current device font. |
| UnifrakturCook / UnifrakturMaguntia TTFs | Named authors and bundled OFL1.1 texts, including Reserved Font Names. | Supported with notices/RFN conditions; optional source boot-study assets, not current firmware fonts. |
| `assets/fonts/md_font.txt` and generated glyphs/icons | GPL-3.0-only SPDX, DEADACTIVE copyright and original hand-drawn attribution. | Supported; retain GPL and authorship. Current device font and icons remain unchanged. |
| `web/fukiai.ttf` | Explicit Hügelton MIT grant in `web/FUKIAI-LICENSE.txt` and `LICENSES/MIT-Fukiai.txt`. | Supported; keep both attribution/license records. |
| `docs/panel.jpg` | Local historical reservation; no verified grant for this exact image. The newer upstream grant names a different `docs/controls.jpg`. | Exclude from release source staging, retain working original. Remove README embedding; not a stable screenshot. |
| Four historical MD UI PNGs listed below | No individual redistribution grant established for these exact images by this review. They are historical mockups/review images, not accepted stable screenshots. | Conservatively exclude from source staging; retain originals. No firmware/browser build input depends on them. |
| SDK `uboot.boot`, `cfg_tool.bin`, `cfg/eq_cfg_hw.bin` | Installed SDK root Apache2.0 license, inherited license record and exact existing package pins. All three local hashes match those pins. | Supported declared Apache2.0 package payloads; preserve license/SDK attribution. No SDK binaries are copied into the source archive. No M-VAVE stock firmware is redistributed. |

### Independent sample-pack permission

The rights-holder's immutable
[commit5738617968a22b5d4c0d3470cfcc95559052f446](https://github.com/hugelton/Felucca/commit/5738617968a22b5d4c0d3470cfcc95559052f446)
and [generator at that commit](https://github.com/hugelton/Felucca/blob/5738617968a22b5d4c0d3470cfcc95559052f446/tools/gen_waves.py)
cover the script and the sounds it generates. Executable statements are unchanged
from our version; code was not replaced with newer upstream DSP. Local script SHA-256:
`d574048795c3e3824bdf367e952b855fdad568efc28c55d42029ece9dd46d0fe`.
Granted script SHA-256:
`d7af8c153eaff0e735b6db7495b83081d6a1634990e94401dbbdd9b43befdd00`.

Primary sample licenses:
[VCSL](https://github.com/sgossner/VCSL/blob/master/LICENSE),
[VSCO-2 CE](https://github.com/sgossner/VSCO-2-CE/blob/master/LICENSE).
The downloaded license blob IDs/hashes and upstream evidence copies are preserved
in the local release-preparation evidence directory. [CC0 legal text](../LICENSES/CC0-1.0.txt)
is now included. The previous “sample terms pending” concern is superseded by this
actual grant, not waived by assumption.

### ORB evidence and resolved provenance

On 2026-10-28, the owner explicitly declared: **Mr. Roboto independently created the
ORB Remix pattern data; Future Retro Orb was conceptual inspiration, not copied
pattern data; redistribution of his own data in Nauclerus is authorized.** The
existing GPL-3.0-only notice is retained. This is the present authority for authorship
and redistribution; hashes are evidence of identity/derivation, not proof of creation.

Read-only reference comparison: X0X-RMX v1.1.0 commit
`a4e78f3a6e16fa310b7614e5876d07e16a0020f9`, and its current local/remote provenance
records. Those records had preserved an earlier transcription account instead of
inventing permission from availability. The new explicit declaration corrects that
account in Nauclerus; the reference repository was not edited. The exact historical
conflicting wording is in X0X-RMX `assets/remix/README.md` (Attribution record) and
`LICENSING.md` (Remix row / Provenance records to confirm): they say patterns were
recorded from the physical Orb. This is superseded owner-supplied narrative, not an
independent device dump or third-party data artifact. It is disclosed here rather
than silently erased. No independent contradictory source artifact was found.

The `Luminary.ksp` header identifies an intermediate source artifact, not a separate
copyright owner. The local text remains byte-identical to the pinned owned material:
SHA-256 `95262b7de2d9461d9dbad7428ca58ec285b1fc4b9fc1e2ce405f42efc67cd47e`.
The 4096 decoded bytes remain
`88034625d620ce4a1a87f954edf81850adf261155a7891fabfea68b01b6d60cb`.
Production 2048-byte nibble data derives from those values; no replacement pattern
source or altered digest is used. No Orb firmware, ROM or audio sample is imported.
Conceptual inspiration is credited without treating it as a speculative
redistribution blocker. See [current declaration](../assets/remix/README.md#attribution-record).

## Safe exclusions and preservation

Only these optional images are omitted from the prepared source candidate:

- `docs/panel.jpg`
- `docs/superpowers/specs/2026-10-19-md-ui/home.png`
- `docs/superpowers/specs/2026-10-19-md-ui/layout-a-vs-b.png`
- `docs/superpowers/specs/2026-10-19-md-ui/screens.png`
- `docs/superpowers/specs/2026-10-19-md-ui/themes.png`

They remain in the working tree and preservation backup. README no longer embeds
the unverified panel image. Historical design text may mention omitted image
filenames as historical context; it does not supply those images or present them
as stable screenshots. No accepted UI graphics, model, preset, source table, DSP,
loader, USB descriptor, project/Song record or recovery code is removed/changed.

The previously prepared RC11 source-candidate archive retains its historical BLOCKED
label/hash and is not rewritten. This RC12 record supersedes its ORB status; that
archive is not the current RC12 source snapshot. Five optional unverified images
remain outside release staging, with originals preserved; they are not compiled
into the RC12 firmware. No required asset is removed or replaced.

## README publication audit

Actual README, CHANGELOG, user guide and release notes were corrected:

- Canonical repository:`https://github.com/Mr-Roboto/nauclerus`.
- Canonical release/download index:`https://github.com/Mr-Roboto/nauclerus/releases`.
- Both canonical URLs return HTTP200. GitHub API confirms the repository exists
  and currently lists **no releases**. No stable asset/tag download URL is invented.
- No upstream hosted installer is advertised as a Nauclerus installation option.
  No Nauclerus GitHub Pages/custom-domain installer or browser site is invented.
- Local RC11 remains named/hash-identified as RC11; stable0.25.0 stays explicitly
  pending. The0.25.0 feature description is the accepted implementation, not a
  claim that a stable binary already exists. No historical screenshot is reused
  as proof of the stable UI.
- Current-project references in SMC/BLE prose now name Nauclerus. The separately
  gated inherited BLE behavior is not advertised as an accepted stable BLE build.
- Actual application USB string **Hortator** and loader string **Felucca Update**
  remain accurately documented; these are intentional compatibility identities,
  not stale branding to rewrite in firmware.
- Original Hortator/Felucca repositories and author/license links remain as
  attribution. Hortator's link is explicitly a credit, not a download substitute.
- DaisySP's redirect was followed and its credit URL corrected from
  `https://github.com/electro-smith/DaisySP` to `https://github.com/daisyaudio/DaisySP`.
- All18 external README URLs returned HTTP200 (DaisySP redirected successfully
  before correction). Internal publication-document file/heading links are checked
  independently of remote site availability.

Unresolved external publication dependencies: a future approved Nauclerus release
entry and uploaded stable package/source/checksums; no hosted installer is required
or promised. The resolved ORB record is separate from URL availability. Original
external recovery links are references, not proof that restoration artifacts or
hardware recovery were exercised during this audit.

## Current RC12 gate

The core ORB redistribution-evidence blocker is resolved by the owner's explicit
independent-authorship/authorization declaration and verified source-to-table chain.
Unverified optional documentation images remain excluded from planned public source
staging; this does not alter device functionality. Stable source/version/package and
publication approval remain future steps, not reasons to block the local RC12 build.

RC12 adds only per-slot LFO BIP/UNI+/UNI- routing and its UI interpretation. Engine
DSP/filter/oscillator, presets, table values, storage layout, loader and USB identity
remain unchanged from accepted RC11. The subjective resonance concern stays accepted.
RC12's new routing needs physical acceptance; no device was accessed during this task.

## Owner-supplied release presentation assets

- `docs/img/nauclerus-banner-1280x320.png`: original owner-supplied publication banner, copied byte-for-byte from the explicitly identified Downloads file; 1280×320 PNG; SHA-256 `9bb994c16e72c0e5011a87e0535be87e46095bb31052d5273c2fec5c1658b688`. Included under the owner’s explicit instruction to reproduce and publish this exact artwork for Nauclerus. No substitute or third-party design was used.
- `docs/img/nauclerus-display-240.png`: genuine 240×240 production LCD framebuffer from the local RC13 WASM, SEQ > REMIX, T3, C7. Screenshot SHA-256 `ad61704e0cc66e102b4695d2e3335d5460b965fd51b12d209886b70ee893468f`; WASM `78dd4006e40cefd2eb6280ee2875d99fd83eac43298ab9ae7a9302f6e426a7be`. It is a generated view of the licensed project software; source credits remain intact.

The five unverified historical images remain excluded. These new, explicitly authorized presentation assets do not reinstate any excluded image.
