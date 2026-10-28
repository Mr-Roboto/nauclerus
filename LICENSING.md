# Nauclerus / Hortator / Felucca licensing

Felucca's code is licensed under the GNU General Public License, version3 only
(`GPL-3.0-only`, full text in `LICENSE`), with component notices below.

**Updated rights-holder evidence:** upstream Felucca commit
[`5738617968a22b5d4c0d3470cfcc95559052f446`](https://github.com/hugelton/Felucca/commit/5738617968a22b5d4c0d3470cfcc95559052f446)
explicitly dedicates `tools/gen_waves.py` and the sounds it generates (the Hügelton
Sample Pack) under CC0 1.0. Its executable Python AST matches the local generator;
only notices, comments and docstrings differ. This supersedes the stale local
“asset terms will be published later” description for that generator/sample pack.
The local generator source/header is preserved; the independent grant is recorded
here, with [CC0 legal text](LICENSES/CC0-1.0.txt).

The legacy panel photograph `docs/panel.jpg` is a separate work. The old local
licensing record reserved rights to it. The newer grant for `docs/controls.jpg`
does not establish identity/permission for this different image. It and four
unverified historical design images are omitted from the prepared source candidate,
while the working-tree originals are preserved. See the
[asset audit](docs/ASSET_AUDIT_0.25.0.md) for exact files, exclusions and the resolved ORB outcome.

Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments

## Nauclerus additions and attribution

Nauclerus development, modifications and remix integration: **Mr. Roboto**. The
Hortator drum-machine fork and BLE MIDI work are by **DEADACTIVE**. The original
Felucca author is **Leo Kuroshita (@kurogedelic), Hügelton Instruments**. These credits
supplement the original notices above and in source; they do not replace them or
change any third-party license terms. MENU > ABOUT retains the original credits
and includes Mr. Roboto's Nauclerus development/modifications/remix credit.

Nauclerus imports the TB-3PO core, ORB mapping data and MORPH mathematical reference
from **Mr-Roboto/fm1-x0x-rmx v1.1.0**, commit
`a4e78f3a6e16fa310b7614e5876d07e16a0020f9`. TB-3PO's retained source notice identifies
**schwung-tb3po**, derived from the **Phazerville Hemisphere Suite TB_3PO applet,
djphazer and contributors**, GPL-3.0. The X0X-RMX phrase/mutation refinements remain
attributed to that pinned implementation; historical upstream generator output is
not claimed identical. The fixed-point oscillator adaptation retains its X0X-RMX
attribution rather than claiming to import the complete floating-point bass engine.

The ORB table's current authorship/permission record is in
[assets/remix/README.md](assets/remix/README.md#attribution-record). On 2026-10-28,
Mr. Roboto explicitly declared independent creation of the pattern data and
permitted redistribution in Nauclerus; Future Retro Orb was conceptual inspiration,
not a copied pattern-data source. That declaration supersedes the earlier
transcription account. The existing GPL-3.0-only notice remains applicable.
Historical inspiration credits do not create a speculative redistribution blocker.
The pinned import/digest/compiled-table chain is verified; those hashes alone do not
prove authorship, which is the owner's explicit declaration. No independent
contrary source artifact was identified.

The historical GPL section7 asset-combination permission below is retained as
inherited text. It does not supply missing rights for a different asset; the
specific newer sample-pack grant above and individual third-party terms govern
their respective material. The ORB record is resolved by the explicit owner declaration, separately from
the independently verified sample-pack grant.

## What is code (GPL-3.0-only)

Every file in this tree that carries an `SPDX-License-Identifier: GPL-3.0-only` header:

- the firmware: `firmware/` (app, HAL, update loader)
- the build script and tools: `build.sh`, `tools/`
- the web pages (installer, editor) and their tests: `web/` (not the Fukiai font, below)
- the host tests: `tests/`

You may use, study, change and share it under the GPL. If you distribute Felucca, or
firmware derived from it, you must also give your recipients its complete corresponding
source under the same licence. That includes devices that ship with modified Felucca
inside.

## Additional permission (GPL-3.0 section 7)

As an additional permission under GPL-3.0 section 7, you may combine Felucca, or a work
based on it, with the Felucca Assets (above), and convey the combination.
This is allowed even though the Felucca Assets are not licensed under the GPL, provided
that:

- you follow the GPL for every part that is not a Felucca Asset; and
- you follow the terms published for the assets.

The Felucca Assets are data (wavetables, icons, sample data). They are not program
code. A firmware image built from the GPL sources with replacement assets, or with no
assets, is entirely governed by the GPL.

## Third-party material

| What | Licence | Where |
| --- | --- | --- |
| TB-3PO musical core and data subset from X0X-RMX v1.1.0; schwung-tb3po / Phazerville TB_3PO lineage | Retained GPL-3.0-only headers; upstream lineage notice says GPL-3.0 | `firmware/src/tb3po.c`, `tb3po.h`, `tb3po_pattern.h`, `tb3po_adapter.c`; `LICENSE` |
| Owner-created ORB-inspired mapping data imported through X0X-RMX v1.1.0 | GPL-3.0-only notice retained; explicit independent-authorship/redistribution declaration above | `assets/remix/remixes.txt`, `assets/remix/README.md`, `tools/gen_remix.py` |
| Fixed-point MORPH geometry adaptation from X0X-RMX v1.1.0 | GPL-3.0-only | `firmware/src/acid_wave.c`; `LICENSE` |
| Hügelton Sample Pack generator and its generated sounds | CC0 1.0; independent rights-holder grant above | `tools/gen_waves.py`; `LICENSES/CC0-1.0.txt`; generator code preserved |
| Instrument samples (Versilian Studios VSCO-2 CE, VCSL) | CC0 1.0 | `assets/samples-cc0/`, provenance in `ATTRIBUTION.txt` there |
| Terminus font 8x16 (ter-u16n): the web pages' typeface (`tests/sim_font.mjs`); no longer in the firmware | SIL OFL 1.1 | `assets/fonts/ter-u16n.bdf`, `assets/fonts/Terminus-LICENSE.txt` |
| UnifrakturMaguntia (j. 'mach' wust, Peter Wiegel): the boot screen study (`tools/boot_variants.py`; no longer in the firmware: the screens are drawn in the pixel font below) | SIL OFL 1.1 | `assets/fonts/UnifrakturMaguntia-Book.ttf`, `assets/fonts/UnifrakturMaguntia-LICENSE.txt` |
| UnifrakturCook Bold (j. 'mach' wust, Peter Wiegel): one face of the boot screen study (`tools/boot_variants.py`; its other faces are SIL OFL 1.1, fetched from Google Fonts at run time, not shipped) | SIL OFL 1.1 | `assets/fonts/UnifrakturCook-Bold.ttf`, `assets/fonts/UnifrakturCook-LICENSE.txt` |
| Fukiai icon font (Hügelton Instruments), web editor only | MIT | `web/fukiai.ttf`, `web/FUKIAI-LICENSE.txt`, `LICENSES/MIT-Fukiai.txt` |
| Grids by Emilie Gillet / Mutable Instruments (<https://github.com/pichenettes/eurorack>): the GRIDS pattern engine is a C port of `grids/pattern_generator.cc` with its pattern tables (and avrlib's random generator) | GPL-3.0-or-later | `firmware/src/grids.c`, `firmware/src/grids_tables.h` |
| Streams by Emilie Gillet / Mutable Instruments (<https://github.com/pichenettes/eurorack>): the COMP sidechain compressor is a C port of `streams/compressor.cc` with its tables | MIT | `firmware/src/comp.c`, `firmware/src/comp_tables.h`; `LICENSES/MIT-Mutable-Instruments.txt` |
| Plaits and stmlib by Emilie Gillet / Mutable Instruments (<https://github.com/pichenettes/eurorack>): integer ports of its drum algorithms | MIT | `firmware/src/dm_kick.c`, `dm_snare.c`, `dm_metal.c`, `dm_dsp.c`; `LICENSES/MIT-Mutable-Instruments.txt` |
| DaisySP by Electrosmith and Emilie Gillet (<https://github.com/daisyaudio/DaisySP>): the modal resonator and the membrane (SKIN engine, RESON effect) are fixed-point ports of its physical models (Source/PhysicalModeling, Filters, Noise, Utility) | MIT | `firmware/src/phys_dsp.c`; `LICENSES/MIT-DaisySP.txt` |
| jl-uboot-tool by Andrey Grigoryev (kagaimiq) (<https://github.com/kagaimiq/jl-uboot-tool>): the boot-mode cipher and command framing in the Mac recovery tool. Its `wl82loader.bin` is not in this tree; users fetch it (docs/RECOVERY.md) | MIT | `tools/rescue/fm1_uboot.py`; `LICENSES/MIT-jl-uboot-tool.txt` |
| JieLi AC79 SDK: `uboot.boot`, `cfg_tool.bin`, `eq_cfg_hw.bin` are read from your SDK checkout at build time and placed in the package; no SDK files are in this tree | Apache-2.0 | <https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK>; `LICENSES/Apache-2.0.txt` |
| JieLi AC79 SDK BT libraries (btctrler, btstack, wl_rf_common, crypto_toolbox, lib_ccm_aes, compiler_rt; members of system.a, cpu.a) and its BT config sources (`lib_btctrler_config.c`, `bt_profile_config.c`), linked into BLE builds only (`FELUCCA_BLE=1`): read from your SDK V1.2.0 copy at build time, none in this tree | Apache-2.0 | SDK V1.2.0, pinned in `tools/ble_libs.py`; `LICENSES/Apache-2.0.txt` |
| JieLi AC79 SDK `apps/common/net/wifi_conf.c`: the radio's RX thresholds and RF look-up table, BLE builds only | Apache-2.0 | `firmware/src/ble/ble_rf_tables.c`; `LICENSES/Apache-2.0.txt` |

The licence texts in `LICENSES/` travel with every published package (`web/make_site.py`).

## Research (not in the firmware)

The MD bank's voices (TRX, EFM) were tuned against measurements of Joe Landers' Machinedrum emulator, a fork of
[Gearmulator](https://github.com/dsp56300/gearmulator) by The Usual Suspects (the DSP56300 project). Only
measurements were taken (renders, analysed by ear and by tools); no code, firmware or data from the emulator or the
Machinedrum is in this tree or in the firmware. Machinedrum is a trademark of Elektron. Hortator is not affiliated
with, endorsed by or supported by Elektron, Joe Landers or The Usual Suspects.

## Services used by the site

The published landing page, installer and editor use [GoatCounter](https://www.goatcounter.com) (a hosted service,
not part of the firmware or the packages): its script count.js (from gc.zgo.at) counts visits, and the installer
sends one anonymous event per finished install. No cookies, no personal data; off while `GC_CODE` in
`web/sim/counter.js` is empty, and never on a local copy.

## Contributions

Contributions are welcome under GPL-3.0-only. By submitting one, you agree that it may be
combined with the Felucca Assets under the section 7 permission above.

## Trademarks

"Felucca" and "Hügelton Instruments" are names of Hügelton Instruments.

"M-VAVE" and "FM-1" are trademarks of their respective owners. Felucca is independent
firmware that runs on FM-1 hardware. It is not affiliated with, endorsed by or supported
by those owners.

## Radio

The default Nauclerus package does not enable the hardware radio. The separately gated
Hortator-derived BLE variant enables Bluetooth only through its existing controls;
its presence in source is not a claim of BLE acceptance for the default package.
No Wi-Fi user feature is provided.

The screen's pixel fonts (`assets/fonts/md_font.txt`) and the VU logo (`cv_vu`, `firmware/src/gfx.c`) are drawn by
hand for this fork, in the style of the Elektron Machinedrum's LCD (no Elektron material is included): GPL-3.0-only,
2026 DEADACTIVE.
