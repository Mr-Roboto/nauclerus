#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Original guard: Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments; DEADACTIVE.
# Nauclerus strict baseline migration: 2026.
"""Owner-approved immutable Nauclerus source boundary; fail closed."""
import argparse
from pathlib import Path
import sys
from security_baseline import ROOT, NAME, COMMIT, SecurityError, source_errors, compiled_boot_guard


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--tree', type=Path, default=ROOT)
    ap.add_argument('--build', type=Path)
    args = ap.parse_args()
    try:
        errors = source_errors(args.tree)
        if args.build:
            compiled_boot_guard(args.build)
        for error in errors:
            print('FROZEN', error)
        print(f'check_untouched: {NAME} {COMMIT}: '+('FAILED' if errors else 'ok'))
        return bool(errors)
    except SecurityError as e:
        print('check_untouched: '+str(e), file=sys.stderr)
        return 1


if __name__ == '__main__':
    sys.exit(main())
