#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# Nauclerus portal: current local firmware; never builds/publishes a device package.
set -euo pipefail
cd "$(dirname "$0")/.."
exec "${PYTHON:-python3}" tools/build_portal.py "$@"
