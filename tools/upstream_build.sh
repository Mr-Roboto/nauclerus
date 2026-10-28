#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Exact owner-approved baseline, no worktree/ref mutation or destructive cache reuse.
set -e
cd "$(dirname "$0")/.."
exec "${PYTHON:-python3}" tools/security_reference.py
