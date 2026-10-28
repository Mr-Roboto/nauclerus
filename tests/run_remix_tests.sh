#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -e
cd "$(dirname "$0")/.."
PY="${PYTHON:-python3}"
CC="${CC:-cc}"
mkdir -p build/gen build/host
"$PY" tools/gen_remix.py build/gen/nauclerus_remix_table.h
"$CC" -std=c99 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -fno-sanitize-recover=all \
    -Ifirmware/src -Ibuild/gen tests/remix_test.c firmware/src/remix.c -o build/host/remix_test
build/host/remix_test
# Inspect an ordinary production object: sanitizer metadata is intentionally excluded.
"$CC" -std=c99 -O2 -Wall -Wextra -Werror -Ifirmware/src -Ibuild/gen \
    -c firmware/src/remix.c -o build/host/remix_layout.o
"$PY" tests/remix_data_test.py --object build/host/remix_layout.o
# Full production sequencer: only this translation unit compiles the activation hook.
"$CC" -O1 -g -Wall -Wno-unused-function -Wno-int-to-pointer-cast \
    -fsanitize=address,undefined -fno-sanitize-recover=all \
    -Ifirmware/src -Ibuild/gen tests/remix_seq_test.c -lm -o build/host/remix_seq_test
build/host/remix_seq_test
"$CC" -O1 -g -Wall -Wno-unused-function -Wno-int-to-pointer-cast \
    -fsanitize=address,undefined -fno-sanitize-recover=all \
    -Ifirmware/src -Ibuild/gen tests/remix_motion_test.c -lm -o build/host/remix_motion_test
build/host/remix_motion_test
"$CC" -O1 -g -Wall -Wno-unused-function -Wno-int-to-pointer-cast \
    -fsanitize=address,undefined -fno-sanitize-recover=all \
    -Ifirmware/src -Ibuild/gen tests/remix_record_test.c -lm -o build/host/remix_record_test
build/host/remix_record_test
"$CC" -O1 -g -Wall -Wno-unused-function -Wno-int-to-pointer-cast \
    -fsanitize=address,undefined -fno-sanitize-recover=all \
    -Ifirmware/src -Ibuild/gen tests/remix_project_test.c -lm -o build/host/remix_project_test
build/host/remix_project_test
"$CC" -O1 -g -Wall -Wno-unused-function -Wno-int-to-pointer-cast \
    -fsanitize=address,undefined -fno-sanitize-recover=all \
    -Ifirmware/src -Ibuild/gen tests/remix_song_test.c -lm -o build/host/remix_song_test
build/host/remix_song_test
mkdir -p build/remix_ui
"$CC" -O1 -g -Wall -Wno-unused-function -Wno-int-to-pointer-cast \
    -fsanitize=address,undefined -fno-sanitize-recover=all \
    -Ifirmware/src -Ibuild/gen tests/remix_ui_test.c -lm -o build/host/remix_ui_test
build/host/remix_ui_test
