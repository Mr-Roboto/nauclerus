#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# Nauclerus: prepare only. No Git mutations, releases, uploads or installation.
set -euo pipefail
cd "$(dirname "$0")/.."
if [[ $# -gt 0 && "$1" != "--pages-only" && "$1" != "--prepare" ]]; then
  echo 'Usage: tools/publish.sh --pages-only (local preparation only; no publish)' >&2
  exit 2
fi
./tools/build_sim.sh --preview
printf '%s\n' 'Prepared build/pages for gh-pages review. Nothing committed or pushed.'
printf '%s\n' 'After separate approval, append a deployment commit to existing gh-pages history; never force-replace its history.'
