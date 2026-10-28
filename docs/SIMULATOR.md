# Nauclerus browser portal and interactive manual

Intended publication URLs (prepared locally; public deployment is not part of this task):

- Portal: https://mr-roboto.github.io/nauclerus/
- Live firmware panel: https://mr-roboto.github.io/nauclerus/#play
- Interactive manual: https://mr-roboto.github.io/nauclerus/#manual
- Source: https://github.com/Mr-Roboto/nauclerus
- Firmware release status: https://github.com/Mr-Roboto/nauclerus/releases

## Build and preview

```sh
tools/build_sim.sh --preview
python3 tools/serve_portal.py --port 8765
# http://127.0.0.1:8765/nauclerus/
```

Requires an existing native Emscripten `emcc`, C compiler, Node, Python with Pillow and
markdown-it-py, and the repository's normal data-generation dependencies. The script
installs nothing and does not launch Docker, contact hardware, generate a device
package, modify Git refs/index, or publish anything. `--host-only` stops after native
simulator checks, IDs, reels and font generation; it still requires Node for the font.
The obsolete `--ref` and `--package` options are intentionally unsupported: build the
actual local source, including uncommitted files, without substituting older binaries.

Outputs:

- `build/sim/source/`: exact source inputs, followed by generated build headers.
- `build/sim/source-manifest.json`: SHA-256 and size of every archived input.
- `build/sim/source.tar.gz`: matching dirty source snapshot, not `git archive HEAD`.
- `build/sim/fm1sim.wasm`, `version.json`: WASM and source/archive/WASM identity hashes.
- `build/sim/manual.json`, `manual.html`: current guide, interactive and static views.
- `build/pages/`: deployment tree with `deployment.json`, licensing and all runtime assets.

The manifest explicitly excludes the five optional images whose redistribution permission
is unresolved (the legacy panel photo and historical UI mockups). They remain untouched
in the working tree and are not runtime dependencies. Source notices, GPL code and font
licenses are distributed with the site. Web assets retain DEADACTIVE/Felucca attribution;
Mr. Roboto's Nauclerus, TB-3PO lineage and independent ORB authorship are credited.

## Actual firmware, browser adapters

`tests/sim_core.c` includes the current production UI, sequencer, Remix, TB-3PO, Motion,
lock banks, projects/Song, engines and FX via `tests/sim_host.h`. Emscripten builds this
same core. Audio runs in an AudioWorklet at 44.1 kHz; JavaScript handles presentation,
panel events, flash-sector storage and lab controls, not a replacement DSP engine.
The generated IDs/counts come from C, including the current preset bank. RC12 routing
supports all six LFO source/mode choices and retains old BIP identities.

Each lab uses a separate WASM instance. The ACID/PWM/morph, polarity, TB-3PO, Remix,
Motion/lock, condition and ratchet demonstrations call production firmware routines.
Lab changes do not rewrite the main simulator's project or a physical instrument.

The inherited 3D FM-1 panel is preserved. Website colors are centralized in `sim.css`
(`--bg`, `--surface`, `--accent`, `--secondary`, `--text`). LCD pixels are firmware
RGB565 output and retain their independent saved color setting. Reduced-motion
preferences disable page animation and animated screen reels.

## Controls, manual and persistence

Click buttons or piano keys, drag/wheel encoders, or use the documented keyboard map.
Multiple pointers/keyboard keys can be held together. Right-click a button or long-touch
it to latch a modifier; tap it again to release. Focus loss releases held inputs.
Controls help lists the keyboard equivalents. HOME/GLO, SEL/ENV/OCT priorities and
all generator/Remix actions are handled by the actual firmware.

The manual is generated from [USER_GUIDE.md](USER_GUIDE.md), supplemented by source-backed
README topics. Search filters topics; navigation links address each section. Hover,
focus or tap the panel map or a control name to highlight it. Context can follow the
live page or be selected manually. “Try this on the live panel” links navigate the real
simulator; destructive/generation operations still require explicit user input.

IndexedDB stores flash sectors in the `nauclerus-sim` database on the current origin.
Use the firmware's PROJECT SAVE to save edits, just as on the device. Reload/power-on
restores the last saved project. Clearing site data or confirming “Reset to demos” erases
browser saves. Storage failures are reported instead of pretending to save. Older
Hortator `fm1-sim` data is left untouched and is not automatically imported; this avoids
loading or overwriting a different project's browser storage. Localhost saves and future
GitHub Pages saves are separate browser origins.

## Verification

The build runs native `sim_test`, `sim_lab_test`, reel generation and the 35 existing
`sim_glue.mjs` checks against the new WASM, followed by 30 portal/layout/licensing/staging checks. A matching successful native code fingerprint
can be reused; changed C/header/version content forces those native checks to run again.
No firmware golden references or thresholds are altered.

Browser automation: `tests/sim_portal_browser.mjs`, using an already available Playwright
installation supplied through `NAUCLERUS_PLAYWRIGHT`. Example (set local runtime paths as
needed, outside the project):

```sh
NAUCLERUS_PLAYWRIGHT=/path/to/playwright node tests/sim_portal_browser.mjs
```

Screenshots and the browser report are written under `build/portal-checks/`. The test
uses the actual `/nauclerus/` path, desktop/mobile viewports, AudioWorklet output,
manual/panel controls, feature labs and IndexedDB reloads. No device is contacted.

## Simulation limits and publication

No hardware USB/TRS MIDI, USB audio device, Bluetooth radio, physical calibration,
update/recovery or hardware CPU-headroom claim is made. This is Web Audio, not an FM-1
USB device. Browser speed and browser storage are not hardware performance/reliability
measurements. ACID resonance quality retains the accepted physical-listening limitation.

`tools/publish.sh --pages-only` only builds/stages locally. `web/make_site.py` verifies
source and WASM hashes, refuses an old installer/package destination, and stages no
firmware updater in a preview. The separately approved stable installer is staged
only by `web/make_site.py` after exact application/loader/package integrity checks.
It verifies package SHA-256 and size in the browser before enabling installation,
requires explicit acknowledgement/confirmation, and supports both legacy and
Nauclerus port names. No obsolete Hortator download is offered as Nauclerus.
A package is never automatically published with a preview.

Before a separately authorized deployment, review `build/pages`, test it under
`/nauclerus/`, and append a deployment commit to the existing `gh-pages` history in an
isolated checkout. Preserve unrelated branches/history and do not force-replace them.
Enable/verify GitHub Pages for the intended repository, then check the actual public
URL, fragments and resource hashes before calling the new portal live. Do not publish
a stale source archive or add an installer without a separate approved implementation.
