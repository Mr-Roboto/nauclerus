# Versions of the drum firmware

The drum firmware is numbered `MAJOR.MINOR.PATCH`, starting at **0.5.0** (2026-10-06). The number lives in one
place, the file `VERSION.txt` at the top of the repository; `CHANGELOG.md` says what each version changed.

## What changes which number

- **PATCH** (0.5.0 → 0.5.1): fixes only. No new feature, no new control; projects and settings stay as they are.
- **MINOR** (0.5.1 → 0.6.0): a new feature or control, or a new project format that still loads the older
  projects (e.g. FDR7 for the song chain). Most roadmap steps are minors.
- **MAJOR** (0.x → 1.0.0, 1.x → 2.0.0): projects or settings saved by the previous version no longer load, or the
  controls change so much that old habits break. 1.0.0 is also the version the user declares finished and stable;
  until then the major stays 0.

A merge with both fixes and a feature is a minor. Upstream Felucca's own numbers (its `v1.0.2` tags, `--release
X.Y` builds) are separate and unchanged.

## Where it shows

- The FM-1: MENU → ABOUT shows `DRUM-<version>`; the USB console (`status`, the greeting) prints it too.
- The local web installer (`build/site`): `drum-<version>+<commit>`, with `-dirty` when the source had
  uncommitted changes. Between versions, builds keep the last version and differ by the commit.
- Git: the local tag `drum-v<version>` on the commit that set it (upstream's `v*` tags are a different line).

## Releasing a version (when work is merged to `main`)

1. Add a line per user-visible change under `## Unreleased` in `CHANGELOG.md`.
2. `python3 tools/version.py bump patch` (or `minor` / `major`): writes `VERSION.txt`, dates the notes.
3. Commit `VERSION.txt` and `CHANGELOG.md`, then `git tag drum-v<version>`.
4. `DRUM_PACKAGE=1 ./build.sh`: the package and `build/site` carry the new number; the host suite checks it
   (`tools/version.py check build`).

## Nauclerus website preparation

`tools/publish.sh --pages-only` now prepares a local site only. It does not push branches,
create tags, create releases, upload packages or publish GitHub Pages. The former upstream
Hortator publication operations have been removed. See [SIMULATOR.md](SIMULATOR.md).

The current application identity is `Nauclerus <VERSION.txt>`. Historical `DRUM-*`,
`drum-v*` and upstream release procedures above describe inherited conventions, not
approval to publish Nauclerus. Stable firmware publication requires a separate reviewed
source snapshot, explicit release approval and the release-readiness checks.

## Nauclerus identity (NAU-0250-015)

`tools/version.py` declares the stable project identity FM-1_25000 and USB product
Nauclerus. VERSION.txt independently selects the display version, currently
Nauclerus 0.25.0. Normal builds pass this exact ID into the application and package.
The legacy --release X.Y identity override is refused. The shared ota-FM-1_900
loader and VID/PID remain unchanged. Do not globally replace legacy identities in
migration fixtures or historical evidence. Public Nauclerus creator fields use
Mr. Roboto; new Git creator metadata uses the GitHub noreply address.
