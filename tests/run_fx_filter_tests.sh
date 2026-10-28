#!/bin/sh
# Focused production DC/LOWCUT and speaker-EQ checks; no sanitizer exclusions.
set -e
cd "$(dirname "$0")/.."
mkdir -p build/host
"${CC:-cc}" -O1 -g -Wall -Wno-unused-function -Wno-int-to-pointer-cast \
    -fsanitize=address,undefined -fno-sanitize-recover=all \
    -Ibuild/gen -Ifirmware/src tests/fx_filter_test.c -lm -o build/host/fx_filter_test
build/host/fx_filter_test
