#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Focused 1.1 delay-division/encoder regression: host code only; no flash.
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/host
${CC:-cc} -O1 -g -Wall -Wno-unused-function -Wno-int-to-pointer-cast \
    -fsanitize=undefined -fno-sanitize-recover=all \
    -Ibuild/gen -Ifirmware/src \
    -o build/host/delay_time_test tests/delay_time_test.c -lm
build/host/delay_time_test

${CC:-cc} -O1 -g -Wall -Wno-unused-function -Wno-int-to-pointer-cast \
    -fsanitize=undefined -fno-sanitize-recover=all \
    -Ibuild/gen -Ifirmware/src \
    -o build/host/delay_pingpong_test tests/delay_pingpong_test.c -lm
build/host/delay_pingpong_test
