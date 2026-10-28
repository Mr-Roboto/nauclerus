#!/bin/sh
# Local flash simulation only. No device, transport or installation backend.
set -e
export AC79_SDK="${AC79_SDK:-$HOME/fw-AC79_AIoT_SDK}"
cd "$(dirname "$0")/.."
CC=${CC:-cc}
mkdir -p build/host
OUT=$(mktemp -d build/host/security-storage-XXXXXX)
"$CC" -O1 -g -Wno-unused-function -Wno-int-to-pointer-cast -fsanitize=address,undefined -fno-sanitize-recover=all \
    -Ifirmware/src -Ibuild/gen tests/security_storage_fixture.c -lm -o "$OUT/security_storage_fixture"
"$OUT/security_storage_fixture" "$OUT/security-userdata.bin"
"$CC" -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all tests/ldr_test.c -o "$OUT/security_storage_loader"
# Two actual packages; a partial old app forces the existing production loader to rewrite sectors.
head -c 100000 build/felucca.bin > "$OUT/security-old-app.bin"
python3 tools/fm1pkg_make.py "$OUT/security-old-app.bin" build/loader/ota.bin "$OUT/security-old.fwsc" >/dev/null
"$OUT/security_storage_loader" "$OUT/security-old.fwsc" build/felucca-UNTESTED.fwsc "$OUT/security-userdata.bin"
