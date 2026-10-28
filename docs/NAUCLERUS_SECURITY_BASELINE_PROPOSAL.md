# Nauclerus independent security baseline proposal

Date: 2026-10-27. Status: **PROPOSAL ONLY — not an adopted baseline or release clearance.**

The only repository modification authorized and made for Phase 5S is this document.
No firmware, tests, guards, index entries, Git references, build outputs or devices were changed.
The original frozen-base-10 requirement remains active and failing. Preparing this proposal does
not authorize adopting its candidate, changing pins, creating tags/commits or installing firmware.

## 1. Preservation and evidence method

Inspected branch: `dev/orb-remix`; HEAD: `2690e74aba6fb99eeb547010e65e52cc2da395e9`.
Existing `main`: `1c6627a9a8b3ed3235c36366daeb5da26cd74e3b`.
There were 24 intent-to-add entries, no content staged, and 451 tracked/new project files at entry.
All pre-existing changes are retained; the proposal remains an ordinary untracked file.

Entry preservation fingerprints (SHA-256):

- Raw Git index: `f556a980c0977dd811c2c73d964364c866dd4c50761eb11bbb3bb41e7c49eef2`.
- Raw `git show-ref` output: `66c2403fa26becaa60a8240ba5d95c421cd8cc26926ca4bea0a6cfb733912518`.
- Sorted project-file manifest: `fdc2d0dc7723b66671ba64a1c057aa48620d687c183e45b2b40f7daf35c50935`.
  Algorithm: concatenate each sorted filename, NUL, raw file bytes, NUL, then SHA-256;
  inventory is the union of `git ls-files -z` and nonignored untracked files.
  Exclude this newly created document when comparing against entry.

Evidence comes from existing Git objects, their parent relationships, raw blob bytes,
committed specifications and actual current source. `git fsck --full --no-reflogs`
returned 0 with no diagnostics. The repository is not shallow. No fetch was performed.
SHA-256 comparisons were calculated on bytes, not normalized text. Historical commit
deltas were also inspected. Existing guard self-tests were run only where they operate
in memory and do not contact hardware.

Object integrity is not author authentication: the inspected reference commits have no
embedded commit signature, and frozen-base-2 through -7 are lightweight tags. Recorded
author names, dates and “user decision” claims are provenance evidence, not independently
authenticated copies of the historical owner's approval. This proposal requires explicit
Nauclerus ratification of that evidence.

Configured remotes remain origin `https://github.com/Mr-Roboto/nauclerus.git` and upstream
`https://github.com/DeadActive/hortator.git`. Phase 5R's successful read-only remote queries
found neither frozen-base-10 nor its exact commit in either repository. That availability
investigation was not replaced by a fetch or a new reference in Phase 5S.

## 2. Reference candidates and provenance

| Material | Exact existing commit | Assessment |
| --- | --- | --- |
| Original Felucca base | `1e838e17e170b20ff09b9660c9a7171aadfc5dca` | Earliest local comparison anchor; loader core, ota.c and usb.c still match it byte for byte. Not proof of a device's installed firmware. |
| frozen-base-2 | `5988fe4622496f9e597974448e8c21054b425f5c` | Input/TIMER5 fixes and reference-build glue. |
| frozen-base-3 | `56ad656a52862674016e82725519c4ad9e95df31` | Adds app USB driver; loader continues using original usb.c. |
| frozen-base-4 | `8aacc988100a1949b13558695dc6ea3f0bc39067` | Storage hardening and erase-order glue. |
| frozen-base-5 | `5a9d9a6504f5036edc3adfeadae4ffad014e6047` | TRS MIDI reference support. |
| frozen-base-6 | `e55d5d0debb4a6e28498c3e0f9f1808a14560f1d` | USB audio support and EP4 HAL additions. |
| frozen-base-7 | `6ae550002f62a59c269dac30235153f664abc8e9` | Latest available frozen tag; MIDI timestamp/source reference glue, no frozen-file change at this step. |
| drum-v0.24.0 | `2690e74aba6fb99eeb547010e65e52cc2da395e9` | Complete, buildable source candidate, accepted only after the independent comparisons below and explicit approval. |

Frozen-base-2's parent is the original base; each subsequent available frozen tag's parent
is the preceding frozen commit. These tags are a separate reference-build lineage, not
ancestors of drum-v0.24.0. Ordinary implementation commits carry the corresponding hunks
into the drum lineage. Therefore ancestry alone is not a valid comparison: blob and
hunk comparisons are necessary.

The stage-2 design and docs/UPSTREAM_1.0.2.md cite original upstream `727f272` for the
adopted 1.0 fixes. That object is **not available locally**; the exact upstream hunk
attribution cannot be independently rechecked against it here. The available frozen
objects and specifications establish local continuity, not full external provenance.
A future retrieval of authenticated upstream material would strengthen this evidence.

### Missing references, without reconstruction

All three later baseline commit objects are absent locally:

| Name | Historical pinned commit | Evidence for intended content |
| --- | --- | --- |
| frozen-base-8 | `c2b64217e30e603fb7cce9cdf1e7dd39b6d11cc2` | ed9f0dd and 2026-10-11 fixes spec §12 |
| frozen-base-9 | `b07efe4afa991a9589c07ea08bfdc45ced0bb186` | c2264cc and tools/frozen_base.txt comments |
| frozen-base-10 | `731386008fae060ae885f44b4765d172ecfae4b3` | 70f44a5 and parameter-locks spec |

The proposal does not claim to recreate any of these commits or prove their complete
trees. They must never be forged, retargeted or silently replaced.

### Exact recorded changes after frozen-base-7

1. **Backlight hook, predecessor frozen-base-8.**
   Implementation commit `ed9f0dd2a73f681630774031233be9a78d31b8d2`, recorded owner decision
   2026-10-12, adds exactly one main.c line immediately after fm1_input_tick():
   `led_dim_tick();`, including its explanatory comment. The spec also requires led_dim.c
   and its include as reference-build glue. No USB/OTA/loader change is part of this approval.
2. **frozen-base-9 USB name.**
   Implementation commit `c2264cc9f28b38d684a1823d87953b98928067cd`, author Eugene Vech,
   dated 2026-10-23, records the owner decision. Within the protected boundary it changes
   only usb_app.c's application STR2 from 16-byte “Felucca” to 18-byte “Hortator”, UTF-16LE.
   Loader “Felucca Update”, VID/PID, manufacturer, protocol and loader marker remain unchanged.
   Comparing the committed parent and result satisfies the existing exact one-line
   usb_name_only() predicate, and the resulting blob equals today's file.
3. **frozen-base-10 lock-bank allocation.**
   Implementation commit `70f44a53f0ae62a1d1ad204354616ce49cf114af`, author Eugene Vech,
   dated 2026-10-24, and docs/superpowers/specs/2026-10-24-parameter-locks-design.md
   explicitly record owner option D. The only protected-file change is storage.c:
   its map comment, object enum and st_sector() lock-bank case. The exact three-hunk
   storage_locks_only() predicate passes against that commit's parent; today's file
   equals the result. Storage CRCs, transaction ordering and prior object locations do not change.
4. **Three added BLE HAL files.**
   `59c220f29935c11962a86dd51bbe0660cec25fcc` adds fm1_ble_hal.h, fm1_ctx.h and fm1_ctx.S.
   All three current files equal those original addition blobs and v0.24.0.
   The BLE design's user-approved amendments 1, 2 and 6 explicitly authorize these additions,
   retain the loader pin and require default-build isolation. Existing frozen HAL files
   did not change. Gate 2a's NEW_OK names these exact three files. New baseline inventory
   must protect their contents, rather than perpetually allowing arbitrary additions.
   The cited external fm1-lsdj `548ce73` provenance was not authenticated in this task.

Classification: these are **documented authorized upstream changes, subject to Nauclerus
ratification of provenance and scope**. There is no unexpected protected-code delta in
the current tree. Authenticity of the unavailable frozen trees and original external
hunk sources remains unresolved; matching a released blob alone is not authorization.

## 3. Protected-file comparison and exact differences

### Boundary classes

**A — byte identity under the existing policy:** all 20 current HAL files, four loader
files, ota.c, usb.c, usb_app.c, crt0.S and app.ld: 29 files. Their complete current bytes
equal the corresponding v0.24.0 blobs. Twenty-five also equal frozen-base-7 directly;
usb_app.c has the reviewed USB change and three HAL files were added later.

**B — existing narrow exceptions:** storage.c except ST_MAGIC; main.c except
felucca_init() and boot-title lines; core.h's last-five-line boot-guard tail.
All three complete current files equal v0.24.0. Against frozen-base-7:

- storage.c has exactly the three recorded lock-bank hunks. ST_MAGIC remains
  `0x554C4546u`; no magic exception is needed for this candidate.
- main.c has the exact backlight hook plus its existing application-init/title exceptions.
  The complete net diff is reproduced below. Removing the existing exceptions leaves
  precisely the hook. The drum initialization delegates to drum_boot_init(); the title
  delegates to draw_boot_title(). The drum fork and logo commits document their purpose,
  including `abd62bda6715cf04d044b0521a4b2a03f2f0b64d` and
  `a78ccfd6378946cca4e93d2b4dd4978e15b9ed69`. These application exceptions are not
  evidence permitting arbitrary startup/recovery edits.
- core.h's whole-file delta is ordinary drum application types/parameters, outside its
  protected tail (156 inserted, 148 deleted lines against frozen-base-7). The protected
  last five lines are identical. Their LF-joined, final-LF SHA-256 is
  `e84d997b8c8fd34bf09c0598d131563e5ac6364a5fd8adc0d41f26b0a9644d83`.
  This tail digest is separate from the whole-file hash below.

**C — ordinary application:** sequencer, Remix, DSP, UI, project, Motion and lock-bank
implementation outside those regions. “Outside frozen” does not mean exempt from
functional/security review: these callers can affect boot, flash access and timing.

**Additional dependency:** libc.c is protected by Gate 2a even though not listed by
check_untouched.py. It is included below and should become explicitly byte-protected.
Likewise the build flags, loader packaging, include selection and comparison tools are
security-relevant inputs requiring reviewed provenance, even where not frozen firmware.

### Per-file SHA-256 manifest

Source keys: B7 = frozen-base-7 blob; USB9 = c2264cc result plus exact parent-hunk review;
LOCK10 = 70f44a5 result plus exact parent-hunk review; MAIN = v0.24.0, independently
checked against B7 with the recorded exceptions/hook; CORE = v0.24.0 whole file,
B7 comparison of the protected tail; BLE = 59c220f addition blob.

For every row, the displayed source/result SHA-256 equals both the current raw bytes and
the v0.24.0 raw bytes. Sources are committed objects independent of the dirty working tree.
“Identical” means no difference; reviewed deltas are those explicitly identified above.
There are 32 files in the declared boundary plus libc.c, 33 rows total.

| File | Comparison source | Current and source/result SHA-256 | Classification |
| --- | --- | --- | --- |
| `firmware/hal/fm1_adc.h` | B7 | `1917870bd802de9134d33335efbcf886b296705dff06688925fd7325589c0c1a` | Identical |
| `firmware/hal/fm1_audio.h` | B7 | `22351440699748546e932098e3016c618d6137dd0ae34557e08a107641d4e62e` | Identical |
| `firmware/hal/fm1_cc.h` | B7 | `46c533a0926ba9298ae36705f857ed609a61b95511ba7f0d1800857b4760dd73` | Identical |
| `firmware/hal/fm1_flash.h` | B7 | `ee9700389d8bf842606b7210e52e125c085a44fbb5d30301cf189d823c4d5e90` | Identical |
| `firmware/hal/fm1_gpio.h` | B7 | `5e3937e32a3b4b9b9ae622c5b42c8819ec8a16f2d76dbddec91654703c77a216` | Identical |
| `firmware/hal/fm1_guard.h` | B7 | `dda2ebdd3024aab7700f8ef00cc55de23c0b369be1d889741d2058eb5c33ce74` | Identical |
| `firmware/hal/fm1_input.h` | B7 | `31c0db0642a21b3b49875b942b4bae46a84086aaed9e965ec087a726326ae512` | Identical |
| `firmware/hal/fm1_irq.h` | B7 | `017b80db3e4da85fb110762d079f03c2ec9648e20e25d4e40f2458952ca940a0` | Identical |
| `firmware/hal/fm1_isr.S` | B7 | `5e45c960577354184efa3bf984a1c14bffb0c5967cbdb643e1f4c9cc196105c5` | Identical |
| `firmware/hal/fm1_lcd_hw.h` | B7 | `4fc3318a703fd55200153442acac4eb33ef56ff8e2eb82ba18ed0a3b9e655c1e` | Identical |
| `firmware/hal/fm1_sys.h` | B7 | `5d56781274406b0a1bf816c66d86c0b0b23b49757cc6f5644df566127827a9a7` | Identical |
| `firmware/hal/fm1_time.h` | B7 | `e83c725eae2ec5a163771ca2b8f1c0779354456f2e20a32febc7b6755758a97d` | Identical |
| `firmware/hal/fm1_timer.h` | B7 | `1ec236ce4a75a5b94b9c16999dcdf7d71e4e0c5e0d2f14dbfb87bbaf025e1004` | Identical |
| `firmware/hal/fm1_uart.h` | B7 | `0df7266a58996c688a4279c6b7edbaa7921e18805c8a65f1ff156680b03fcff6` | Identical |
| `firmware/hal/fm1_usb.h` | B7 | `12b9b31bde0dbb15fa15379a215eaec11e882bd33c18f4b250fa9e3a47c202f0` | Identical |
| `firmware/hal/fm1_vec.S` | B7 | `8f9e787871332f522157d561dc3cfe8906ed0abf160139ed5819cfdeab66f01a` | Identical |
| `firmware/hal/fm1_xip.h` | B7 | `dcba6ddb9cd2403e1d97c4db49d836110ef2805d2369db7ed323e3fbe26c56ed` | Identical |
| `firmware/loader/crt0_ldr.S` | B7 | `bd4149b97520d1dcc271f730c5a046b0dbf97909e48e968e0ba18304ffc37841` | Identical |
| `firmware/loader/ldr_core.c` | B7 | `b686100b8e633498b9188c43eee349381b3fb7d7ef63ca0564003c9ac19655a5` | Identical |
| `firmware/loader/loader.c` | B7 | `313d0b916510434dc7f662ca24ae96afcdd1c06662754acbfbc924a3cf5b2434` | Identical |
| `firmware/loader/loader.ld` | B7 | `4acae8c9a16cf9a70f53a546f9175b5a86d2bc5cb6bce39513a2ff3c4fdba4ae` | Identical |
| `firmware/src/ota.c` | B7 | `655616db0ccd424b512a3a32530ed12c22cacea95df4855d688fdeae1a8bd980` | Identical |
| `firmware/src/usb.c` | B7 | `da4d6637bf58c60df30ed13f46b21241f09d908d50ca6c75bce26dc426c2d8d2` | Identical |
| `firmware/src/usb_app.c` | USB9 | `02dd5da862e386d418ab2f41cd44f7d33512fa4e6e9c73b21da20f0d7377c910` | Reviewed difference; see §3 |
| `firmware/crt0.S` | B7 | `2212471c4a70e94022ba24ef65a4b208fadd5f7e33f80745c5f5e8b9f4aa840c` | Identical |
| `firmware/app.ld` | B7 | `6a9f41205e482fbdf2d24dc418416e45a3a8e1781d3e594279d751335af8942a` | Identical |
| `firmware/src/storage.c` | LOCK10 | `236fa939667fa3588f79c1228c7908abf377723348b7eccd55783a37cd50dba0` | Reviewed difference; see §3 |
| `firmware/src/main.c` | MAIN | `4a138bc1f12c0f2147183e3716dd548df5e9d7c01b165b7a3d401288d54764eb` | Reviewed difference; see §3 |
| `firmware/src/core.h` | CORE | `a784cea0f0090860b233f3db9cc4d9221fe2304f1b563f179285d727729731e5` | Reviewed difference; see §3 |
| `firmware/src/libc.c` | B7 | `344b09afe33da826feefeb1ea2a1a7c50732f91c133fee30174819645edc131a` | Identical |
| `firmware/hal/fm1_ble_hal.h` | BLE | `87550a810247ae528e3d904f4d81c0e1d7fd5712298d599aa363ac0ca3f2ab8f` | Identical to addition commit |
| `firmware/hal/fm1_ctx.h` | BLE | `8e7cd702ed3184d812ba009750ac169255a2805e0b7e49b47b89b9fda1f2eec2` | Identical to addition commit |
| `firmware/hal/fm1_ctx.S` | BLE | `150e2c285da9c9ab0e9120f9fc70f389c42085576a3d0dce8f35583983b9fdff` | Identical to addition commit |

B7 whole-file hashes for the four changed files (current/result hashes are in the table):

| File | B7 SHA-256 |
| --- | --- |
| `firmware/src/usb_app.c` | `ea2b55ade91cbb0e3d5f9a9fbc290349cb5e66dbf9b552065450345eb0ae6705` |
| `firmware/src/storage.c` | `74e949f656674c60c5f0ea7b403bf3f9286e8be20b872316046f0fb5b0549cbb` |
| `firmware/src/main.c` | `d818ba39c563e0a4fea91a3538246cfe7169a555dc5b4c8121ea4a264ded73fd` |
| `firmware/src/core.h` | `3333f01dd5a9d5b17d15fd01292441d4b5e1ec3bfc118916fe86febf010349c8` |

Exact USB9 protected-file diff:

```diff
diff --git a/firmware/src/usb_app.c b/firmware/src/usb_app.c
index 75dba5a..7b424b1 100644
--- a/firmware/src/usb_app.c
+++ b/firmware/src/usb_app.c
@@ -226,7 +226,7 @@ static const uint8_t STR1[] = {42, 3, 'H', 0, 0xFC, 0, 'g', 0, 'e', 0, 'l', 0, '
 static const uint8_t STR2[] = {30, 3, 'F', 0, 'e', 0, 'l', 0, 'u', 0, 'c', 0, 'c', 0, 'a', 0, ' ', 0, 'U', 0, 'p', 0,
                                'd', 0, 'a', 0, 't', 0, 'e', 0};
 #else
-static const uint8_t STR2[] = {16, 3, 'F', 0, 'e', 0, 'l', 0, 'u', 0, 'c', 0, 'c', 0, 'a', 0};
+static const uint8_t STR2[] = {18, 3, 'H', 0, 'o', 0, 'r', 0, 't', 0, 'a', 0, 't', 0, 'o', 0, 'r', 0};
 #endif

 static int get_desc(uint32_t wvalue, const uint8_t **d, uint16_t *l)
```

Exact LOCK10 protected-file diff:

```diff
diff --git a/firmware/src/storage.c b/firmware/src/storage.c
index 1212c44..8d41163 100644
--- a/firmware/src/storage.c
+++ b/firmware/src/storage.c
@@ -18,8 +18,10 @@
 #define ST_PAYLOAD_MAX (ST_SECTOR - ST_PAYLOAD_OFF)

 /* flash map (FL_DATA 0x97000..0xDFFFF, FL_GLOB 0xFC000..): settings 0xFC000, projects 0x97000..0x9EFFF,
- * user sample slots 0xA0000..0xDBFFF (eng_sample.c), user preset banks 0xDC000..0xDFFFF (upreset.c) */
-enum { OBJ_SETTINGS, OBJ_PROJECT0, OBJ_UPRESET0 = OBJ_PROJECT0 + 4, OBJ_COUNT = OBJ_UPRESET0 + 2 };
+ * user sample slots 0xA0000..0xD3FFF (eng_sample.c), lock banks 0xD4000..0xDBFFF (project.c),
+ * user preset banks 0xDC000..0xDFFFF (upreset.c) */
+enum { OBJ_SETTINGS, OBJ_PROJECT0, OBJ_UPRESET0 = OBJ_PROJECT0 + 4, OBJ_LOCKS0 = OBJ_UPRESET0 + 2,
+       OBJ_COUNT = OBJ_LOCKS0 + 4 };

 typedef struct {
     uint32_t magic;
@@ -52,6 +54,8 @@ static uint32_t st_sector(uint32_t obj, uint32_t copy)  /* flash offset of copy
 {
     if (obj == OBJ_SETTINGS)
         return 0xFC000u + copy * ST_SECTOR;
+    if (obj >= OBJ_LOCKS0)
+        return 0xD4000u + (obj - OBJ_LOCKS0) * 2u * ST_SECTOR + copy * ST_SECTOR;
     if (obj >= OBJ_UPRESET0)
         return 0xDC000u + (obj - OBJ_UPRESET0) * 2u * ST_SECTOR + copy * ST_SECTOR;
     return 0x97000u + (obj - OBJ_PROJECT0) * 2u * ST_SECTOR + copy * ST_SECTOR;
```

Complete main.c difference from B7 to the candidate/current file:

```diff
diff --git a/firmware/src/main.c b/firmware/src/main.c
index b13778e..7c998c6 100644
--- a/firmware/src/main.c
+++ b/firmware/src/main.c
@@ -22,6 +22,7 @@ void fm1_timer5_irq(void)
     fm1_timer5_ack();
     felucca_dbg.timer_irqs++;
     fm1_input_tick();
+    led_dim_tick();                             /* the dimmed LEDs (led_dim.c): the column the next tick lights */
     {   /* milliseconds from the 24 MHz TIMER4 (robust to a late tick) */
         static uint32_t last, acc;
         acc += t0 - last;
@@ -94,26 +95,10 @@ static void fm1_fault(const fm1_crash_t *c)
     fm1_reboot();
 }

-/* power-on: three parts with their default sounds (TRK_DEF), the drum track, empty patterns */
+/* power-on: the drum kit (drum_core.c KIT_DEF), empty patterns */
 static void felucca_init(void)
 {
-    uint32_t i;
-    for (i = 0; i < G_COUNT; i++)
-        song.g[i] = GP[i].def;
-    for (i = 0; i < NTRK; i++) {
-        track_t *t = &trk[i];
-        track_defaults(t);
-        if (i < NPART) {
-            set_engine_of(t, TRK_DEF[i][0]);
-            apply_preset_to(t, TRK_DEF[i][1]);   /* with its sends */
-            t->engine = t->eng_req;
-        }
-        track_defaults_steps(t);              /* the sequencers start empty */
-    }
-    song.sel = 0;
-    song.master_q12 = 2048;
-    ui.home = 1;
-    ui.force = 1;
+    drum_boot_init();
 }

 static void fm1_main(void)
@@ -126,8 +111,7 @@ static void fm1_main(void)
 #endif
     settings_init();
     lcd_init();
-    draw_text_box(0, 100, 240, &FONT_L, "FELUCCA", C_HI, 1);
-    draw_text_box(0, 130, 240, &FONT_S, "MULTI-ENGINE SYNTH", C_GRAY, 1);
+    draw_boot_title();                              /* ui_menu.c: the HortatoR logo */
     if (felucca_dbg.magic != DBG_MAGIC) {
         memset(&felucca_dbg, 0, sizeof felucca_dbg);
         felucca_dbg.magic = DBG_MAGIC;
```

For the three BLE additions, the exact difference from B7 is absence -> the complete
59c220f blob identified by each hash. They are not modifications of an existing B7 file.
The core.h boot-guard region has **zero differing lines**; its ordinary body differences
are deliberately not treated as frozen-code exceptions requiring a new baseline.

## 4. Independent update, storage and boot review

These are source-level invariants and supporting tests, not a claim of hardware validation.

### Flash ownership and application/loader separation

All ranges below are half-open:

| Region | Range | Invariant |
| --- | --- | --- |
| SPL/configuration head | [0x00000, 0x04000) | Nauclerus loader never writes it. |
| Application | [0x04000, 0x93000) | Only the RAM update loader rewrites it, sector-aligned. |
| Project A-D copies | [0x97000, 0x9F000) | Existing two-sector pair per slot; 0x9F000 remains unallocated. |
| User samples | [0xA0000, 0xD4000) | USR3 shrinks to [0xC8000, 0xD4000). |
| Lock banks A-D | [0xD4000, 0xDC000) | slot*0x2000 + copy*0x1000; eight distinct sectors. |
| User presets | [0xDC000, 0xE0000) | Existing two banks and A/B copies unchanged. |
| Loader staging | [0xE0000, 0xE5000) | OTA stage writes only here; update record at 0xE4F00. |
| Settings A/B | [0xFC000, 0xFE000) | Existing globals; 0xFE000 remains unallocated by storage.c. |

fm1_flash.h permits app writes only to its declared data/globals/staging windows.
FL_IN uses subtraction-based bounds checks to avoid offset+length overflow and 1 MiB
address wrapping. Erases require sector alignment; programs fit one nonempty 256-byte page.
Loader compilation substitutes its own application/record windows. RAM-run flash code
does not execute from XIP while flash is busy.

Important qualification: ldr_core.c also scans [0x93000, 0xFC000) for update records and
may erase a sector whose +0xF00 bytes match record type and CRC. That is a conditional
cleanup path, not blanket erasure, but it overlaps data sectors. Never describe the loader
as having no writes outside the application.

Lock-bank payloads remain below the +0xF00 record probe, with their unused tail erased.
The existing 3208-byte bank and 3840-byte project record/commit conventions need explicit
record-collision tests. The general storage maximum is 3840 bytes; it does not itself
guarantee every object avoids the probe. Existing project formats must be tested as they
actually are, rather than asserting that all storage payloads are <=3584 bytes.
The pre-bank USR3 clamp and failure-to-clamp write block remain required.

Storage saves use the alternate A/B sector: erase, payload, header LAST; header and body
CRC, object/copy/length, sequence wrap and full readback decide validity. Project save
writes its referenced lock bank before committing its project record; load can select
the bank copy matching the project link. This transaction protocol and all prior object
addresses must remain unchanged.

### Update integrity and authorization limits

- ota.c validates UFW header/list CRCs, loader structure and size. Its existing own-loader
  recognition uses FELUCCA-LOADER-1; an official-loader path has known header/body CRC
  constants and compares the entire device head before allowing its broader writer.
- Staged loader bytes are CRC-checked, programmed, read back and checked before writing
  the stage header and the verified persistent update record. No host “success”, no commit.
  The normal build must not enable RAM-only updates or dry-run/measurement shortcuts.
- The own loader reads the device's isd_config.ini key and validates the decrypted app-area
  header before its first application erase. A wrong key/damaged header is refused.
  It skips equal sectors and reads back every written sector, with bounded retry.
- It clears the update record after completion; an intact flash record supports resume
  after power loss. RAM record handling avoids a warm-reset loop.

**Threat-model limit:** the loader marker and CRCs are not cryptographic signatures.
The chip-key/header check is compatibility protection, not publisher authentication.
A trusted host/package-distribution channel is assumed. The device does not enforce
the host-side SHA-256 loader pin against a malicious host that supplies a forged marker.
Readback equality proves that delivered bytes were written, not that they are authentic.
Do not claim secure boot or malicious-package resistance on this evidence.

Likewise load_package() in fm1_install.py checks product syntax and the loader marker
(or the exact stock V15 hash); it is not a complete entry/payload CRC validator.
Release-side complete package validation must be an explicit additional gate, with
negative tests; no installer invocation is authorized in this phase.

### Boot and recovery

crt0.S preserves the SPL entry stub, disables interrupts and establishes stacks before
fm1_cstart. main.c arms the watchdog, evaluates the persistent boot-loop guard before
application initialization, installs fault/stack/write/bus/PC guards and copies RAM-run
code. Two pending failed early boots enter UBOOT; healthy operation clears the guard after
30 seconds. Physical OCT-/OCT+ and USB UBOOT paths, loader USB polling, and reserved
mailbox/vector areas remain unchanged. Recovery feasibility on an actual FM-1 is still
unverified here; a source/binary pin cannot prove ROM/SPL behavior or a usable recovery cable.

### Checks executed in Phase 5S

| Check | Actual result |
| --- | --- |
| Full local Git object fsck | PASS; no diagnostics |
| Per-file byte/hash and historical exact-hunk comparisons | PASS as described above |
| Loader SHA-256 | PASS; 6493 bytes, existing pin unchanged |
| Loader one-bit tamper self-test | PASS: altered loader rejected |
| Gate 2 source/normalization self-tests | PASS |
| Gate 2a sources | PASS: identical to drum-v0.24.0 |
| Erase-order check against existing disassembly | PASS |
| Existing check_untouched.py | BLOCKED/fails: original frozen-base-10 missing |

No firmware build, full output-producing test suite, flashing, update or installation tool
was run for this proposal. Gate 5/functional/fidelity/sanitizer evidence from Phase 5 remains
historical evidence, not a new execution in Phase 5S.

## 5. Proposed Nauclerus baseline mechanism

**Proposed reference: `nauclerus-security-base-1`. Do not create it yet.**

Recommended candidate target is the existing full commit
`2690e74aba6fb99eeb547010e65e52cc2da395e9`, tree
`3b0583347cc1e91e7870b0051fe4256e0066055d`.
Using this complete pre-Remix source retains its real reference-build glue and avoids
inventing a missing frozen tree. This is acceptable only after the evidence, exceptions,
uncertainties and trust model in this proposal receive explicit approval. It is not
automatic acceptance of drum-v0.24.0 and is not a claim that it equals missing frozen-base-10.

After approval, use an annotated, preferably signed tag. Pin the peeled commit, tree,
tag-object ID and SHA-256 of a canonical protected-file manifest. Bind the manifest,
source history, reviewed deltas, toolchain/SDK inputs and approval record in a signed or
independently attested baseline descriptor. Do not let the mutable candidate generate
its own trusted expected hashes. Keep a verified offline Git bundle/source archive and
descriptor; a hosting service or movable tag must not be the sole trust anchor.

Preserve the original frozen-base-10 pin and failure history in the adoption record.
Only a separately reviewed tool migration may change the active FROZEN_BASE line.
Keep LOADER_SHA256 exactly unchanged. There must be one active explicitly selected
baseline, never “use whichever tag is available” or a fallback on guard failure.

Strengthen the source contract:

1. Pin all 29 A files, including the three BLE additions, by raw-byte SHA-256, Git blob
   identity and file mode. Compare complete recursive directory inventories to reject
   additions, deletions, renames and symlink substitutions.
2. Freeze complete storage.c and main.c for baseline 1; today's application needs no
   exception to their candidate bytes. This is stricter than broad ST_MAGIC/init/title
   exemptions. Any future required exception must be an explicitly approved exact
   old->new patch plus expected result digest, applied once at an unambiguous location;
   no general regular-expression exclusion. Existing tests that permit arbitrary
   ST_MAGIC changes would need an explicitly reviewed stronger-policy replacement.
3. core.h's ordinary application body remains editable, but retain the exact boot-guard
   declaration/magic and protected suffix, at one canonical location. Reject duplicate
   definitions/redefinitions, deletion, movement or changed section placement. Test
   compiled size/section and early-boot behavior as well as source bytes.
   Its whole-file hash is reference evidence, not an unconditional equality rule:
   the descriptor must explicitly mark core.h as a region contract, unlike the other
   32 whole-file-protected entries. Ordinary body edits must not acquire a blanket
   exemption for declarations or macros that affect the protected code.
4. Add libc.c to explicit source protection. Treat include selection, compiler defines,
   linker scripts, build/package tools and guard changes as reviewed security inputs.
   Source identity alone cannot close over arbitrary callers or build flags.
5. Resolve the expected named reference and all pinned identities before checking files
   or using a cache. Missing/moved tag, absent object, wrong tree, malformed/duplicate
   pin entries or manifest mismatch fail closed.
6. Build the baseline and candidate independently from exact committed archives with
   the same locked toolchain and SDK inputs. Preserve H2's current comparison semantics,
   approved inlining facts and self-tests; do not expand allowlists to accommodate an
   unexplained difference. A new candidate reference may expose real H2 differences;
   those must be reported/reviewed, not automatically excused.
7. Reference builds and artifacts must carry a source/tool/flag receipt and hashes.
   An existing disassembly plus matching HEAD is insufficient cache validation.
   Require clean source and verified receipt, or rebuild in a fresh approved directory.

## 6. Exact future tool changes — separately authorized implementation

Nothing in this list is implemented by Phase 5S.

| File/tool | Proposed future change |
| --- | --- |
| New tools/security_baseline.json | Schema-versioned descriptor with approved immutable identities, 33-file inventory/modes/hashes, exact core contract, provenance and empty general exception list. Record its digest in the approval/attestation. |
| New tools/security_baseline.py | Shared strict resolver/manifest verifier; reject missing/moved refs and malformed/duplicate records; no implicit fallback. Test-only fixtures confined to isolated repositories. |
| tools/frozen_base.txt | Only after adoption approval: active FROZEN_BASE becomes the approved Nauclerus name and exact commit; loader pin stays unchanged; preserve old pin/history in documentation. |
| tools/check_untouched.py | Use resolver, raw bytes and full inventories; whole-file main/storage protection; bounded core contract; explicit deletion/addition/mode diagnostics. |
| tools/upstream_build.sh | Use same resolver; build exact immutable candidate reference with locked inputs/receipt. Replace unsafe stale-cache acceptance; never remove an unrelated existing checkout. |
| tools/check_loader.py | Retain exact current SHA-256/one-bit failure; add descriptor consistency and verify the package's extracted loader is the pinned artifact too. |
| tools/check_update_path.py | Retain existing narrow historical-hunk self-tests; bind authoritative release security checks to explicit descriptor rather than relying only on newest drum-v* discovery/NEW_OK. Preserve BLE gates separately. |
| New tools/check_package_integrity.py | Offline-only parser validating complete package structure, bounds, header/list/entry CRCs, embedded loader hash, expected product and candidate application bytes/hash. No device backend or force bypass. |
| tests/guard_test.sh and new security-baseline tests | Extend isolated-fixture negative tests listed below; preserve real-tree/index/refs; expected stronger-policy tests replace only obsolete permissive cases after review. |
| tests/run_tests.sh / release validation entry point | Release mode requires baseline build, H2/H3, loader and artifact checks; skips cannot produce release success. Development skips remain explicitly reported. |
| tools/compare_upstream.py, stack/budget/erase/loader checks | No relaxed semantics, new inlining exceptions, changed thresholds or replacement goldens proposed. Only explicit validated baseline receipt plumbing where necessary. |

Security-tool changes themselves need reviewed diffs and pinned release-source provenance;
a manifest stored alongside a maliciously changed verifier is not an independent root of trust.

## 7. Security regression plan

Implement/run these in temporary fixture repositories/copies after approval. Never mutate
the real checkout/index/refs for a negative test. Each negative row must return nonzero
from its intended guard, with the expected diagnostic; a crash or unrelated missing
dependency is not a successful negative test.

| Fixture | Required outcome |
| --- | --- |
| One-byte change in every byte-protected file | FAIL identifying that file |
| Protected file deleted, renamed, mode changed or replaced by symlink | FAIL identifying inventory/type/mode mismatch |
| Unexpected file added to either frozen directory, including untracked | FAIL inventory; no ignored addition |
| storage.c address/object/CRC/readback/transaction change | FAIL, including a change beyond any approved exact patch |
| main.c recovery change hidden on a title line or inside init exception | FAIL under complete-file protection |
| Boot-guard magic, declaration, placement or duplicate changed | FAIL source and relevant compiled/boot checks |
| Loader bit flipped, truncated, replaced or changed only inside package | FAIL exact loader hash/artifact checks |
| Baseline tag absent or commit object absent | FAIL before comparisons/build |
| Tag moved to another commit, or replaced tag object/signer with same peeled commit | FAIL pinned identity/attestation checks |
| Manifest/pin malformed, duplicated, hash changed or unsupported schema | FAIL; no fallback/default |
| App/OTA/record ranges widened, wraparound arithmetic or page limits changed | FAIL source guard; independent flash-sentinel tests demonstrate out-of-range writes are blocked |
| Package header/list/entry CRC wrong, offsets/lengths overflow or payload truncated | FAIL offline complete artifact validation |
| App payload or embedded loader corrupted | FAIL release artifact checks, including recomputed inner CRC but wrong approved artifact hash |
| Wrong chip key/damaged app header | Existing loader simulation refuses before any erase |
| Stage body corruption, readback fault, missing confirmation or timeout | OTA simulation: no committed record, safe cleanup |
| Interrupted A/B/project+bank save | Prior committed project/bank remains selected; no false SAVED |
| Record-like data at +0xF00, lock/sample/project adjacency | Independent sentinels and collision fixtures prove accepted records cannot erase unrelated live objects |
| Valid ordinary application edit outside frozen boundary | Source guard PASS; complete functional/build/resource gates still required |
| Unmodified accepted source with exact descriptor/artifacts | All required guards PASS; no skipped release gate |

Retain and extend tests/ota_test.c, ldr_test.c, storage_test.c, boot_test.c,
fm1_uboot_test.py, install_test.py and existing parser/comparison self-tests.
The simulated tests must check flash head preservation, every permitted write window,
record cleanup/resume, wrong-key refusal and exact sector adjacency with sentinel bytes.

Do not assert that the existing device rejects every corrupted body or a deliberately
forged package: that is not its security model. Complete payload integrity must fail at
the new offline release gate; device tests cover protections the device actually implements.
If collision/resume or other new security fixtures expose a failure, stop adoption and
report it as a separate correctness task rather than weakening a test.

H2 and comparative H3 can be restored using an approved, receipt-verified build of the new
reference with existing toolchain inputs. Their results are not known until actually run.
Absolute stack limits and static target budgets already work without that build.
Linux host instruction counting remains unavailable; do not replace skipped measurements
with zeros or claim those checks passed.

Simulated recovery tests do not need hardware. Official V15 restoration requires a verified
FM-1_v15.fwsc matching the recorded stock hash; it is currently absent. Actual update,
power-loss resume, boot-loop fallback and mask-ROM recovery need an FM-1, verified recovery
artifacts/cabling/data backups and separate device authorization. Never deliberately interrupt
a hardware update without approval and an independently verified recovery procedure.

## 8. Immutable release-candidate preparation — future approval required

1. Preserve the existing development tree and index first: approved external complete
   backup, tracked binary diff, untracked file archive, index/ref snapshots and artifact
   hashes. Do not reset/stash/discard the 24 intent-to-add entries as a shortcut.
2. Review/adopt the baseline descriptor and its tool implementation separately. Retain
   security decisions and before/after guard evidence. Do not declare success merely
   because the missing-tag error disappears.
3. Review an exact candidate file inventory including all current new files and this
   proposal. Exclude generated build outputs/secrets. Stage only explicit approved paths
   after separate authorization; inspect staged diff. Do not use git add -A.
4. Choose a Nauclerus version, changelog, product branding and release destination.
   Existing policy calls a new feature/control a MINOR change: 0.25.0 is a candidate,
   not a decision made here. Preserve protocol/loader identities unless separately reviewed.
   Do not use upstream --release X.Y as a substitute for the drum MAJOR.MINOR.PATCH workflow.
5. Commit the complete reviewed source and approved tool changes, then create an annotated,
   preferably signed RC tag on the full commit. Keep the security-baseline reference and
   RC reference distinct. No branch switch or main update is implicitly authorized.
6. Build two fresh clean copies of that exact source with the same locked compiler,
   linker, Python/generators, SDK inputs, flags and packaging tools; record their versions,
   source hashes and container digest if applicable. Avoid network dependency updates.
   Compare raw application, loader and package bytes/SHA-256. Record any nondeterminism
   and fix/review it separately; no undocumented normalization of differing artifacts.
7. Embed/show approved version and exact clean source identity. Record full commit/tree,
   baseline descriptor digest, application SHA-256, firmware-package SHA-256 and unchanged
   loader SHA-256 in a signed/attested candidate manifest. Current dirty-build hashes below
   are evidence only, not hashes of a future RC.
8. Run focused Remix/UI/ACID/Motion/Lock/Song/project tests, ASan/UBSan, fidelity and
   OFF-equivalence, complete regression runner, all security negative tests, H1–H4 where
   supported, H2/H3 comparisons, target budgets, stack/memory/reserves and Gate 5 unchanged.
   Keep goldens, limits and reviewed inlining facts unchanged. Every skip must remain
   visible and require an explicit release disposition; “runner returned zero” alone
   is insufficient. Resolve package-path prerequisites instead of silently omitting checks.
9. Before authorized installation, verify recovery artifacts/stock hash, USB protocol and
   product compatibility, loader pin, independent backups of personal/device-specific data,
   stable power/cable/host, known boot/recovery entry and a rollback plan. No recovery test
   evidence presently establishes actual device readiness.
10. After separate device approval, complete the Phase 5R hardware checklist: display/all
    palettes and controls, APPLY timing/STOP, eight-track drum/ACID/held-slide behavior,
    Motion/locks/LFO, chronological recording, Song transitions, save/load/power-on and
    prolonged audio/CPU stability. Publishing needs another explicit decision.

Current existing artifacts, rehashed in Phase 5S (not rebuilt):

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| build/felucca.bin | 344012 | `7f53b845e69c1d14fe0ead7a31c7f7a65caee67f14b27bfd175c978dac68f876` |
| build/felucca-UNTESTED.fwsc | 609649 | `40c261a06a4148ab6539747a5039e0473eb0f47da92902a48d57150c8cdafbd4` |
| build/loader/ota.bin | 6493 | `cc98eed224299fa42ca22a546073e0f8b98d8362e26b5f325a9aaeab287bdf5d` |

Current build identity remains FM-1_900 / DRUM-0.24.0, with a dirty source label.
Recorded resources remain flash 344012 B, static RAM 73332 B, pool 307316 B,
.noinit 15576 B; the 8192-byte pool reserve and existing limits are not to change.
This phase did not rebuild or newly measure memory.

## 9. Outstanding decisions, uncertainties and stop conditions

Explicit approval is still required to:

- Accept the documented, unsigned local history and recorded owner decisions as the
  Nauclerus trust foundation, or first obtain independently authenticated upstream evidence.
- Adopt the proposed candidate/name and define maintainers/signing keys/attestation policy.
- Create any baseline tag/commit/bundle, change the active pin, implement guards/manifests
  or run builds that create outputs. Preparation approval is not adoption approval.
- Accept the stronger complete-file main/storage policy and its reviewed test changes.
- Choose RC version/branding, stage/commit/tag source, or update main.
- Acquire/validate additional recovery artifacts, contact or install on hardware, and
  later publish a release.

Unresolved: missing frozen-base-8/-9/-10 objects, missing original 727f272 verification,
unsigned history/external BLE provenance, future H2/H3 results, complete artifact-validator
negative coverage, physical recovery/acceptance and unavailable Linux host instruction
measurements. None is “passed” by this proposal.

No unauthorized protected-file difference was identified. That finding establishes
consistency with the available historical material; it does not close the authenticity,
trusted-host or physical-recovery gaps. Adoption must stop if provenance is rejected,
a negative guard fails, reproducibility fails or a new security invariant is violated.
