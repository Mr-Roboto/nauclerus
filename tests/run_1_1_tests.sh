#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Focused Nauclerus 1.1 development tests: no firmware flash or release.
set -eu
cd "$(dirname "$0")/.."
sh tests/run_delay_tests.sh
mkdir -p build/host
${CC:-cc} -O1 -g -Wall -Wno-unused-function -Wno-int-to-pointer-cast \
    -fsanitize=undefined -fno-sanitize-recover=all \
    -Ibuild/gen -Ifirmware/src \
    -o build/host/solo_test tests/solo_test.c -lm
build/host/solo_test

${CC:-cc} -O1 -g -Wall -Wno-unused-function -Wno-int-to-pointer-cast \
    -fsanitize=undefined -fno-sanitize-recover=all \
    -Ibuild/gen -Ifirmware/src \
    -o build/host/midi_features_test tests/midi_features_test.c -lm
build/host/midi_features_test

${CC:-cc} -O1 -g -Wall -Wno-unused-function -Wno-int-to-pointer-cast \
    -fsanitize=undefined -fno-sanitize-recover=all \
    -Ibuild/gen -Ifirmware/src \
    -o build/host/confirm_window_test tests/confirm_window_test.c -lm
build/host/confirm_window_test
