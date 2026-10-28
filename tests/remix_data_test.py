#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Pin canonical bytes, validate packing, and inspect the production object's storage."""
import argparse
import hashlib
import re
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from gen_remix import SOURCE, generate, pack, read_rows

SHA256 = "88034625d620ce4a1a87f954edf81850adf261155a7891fabfea68b01b6d60cb"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--object", type=Path, required=True)
    args = parser.parse_args()
    rows = read_rows(SOURCE)
    values = bytes(v for row in rows for v in row)
    assert len(rows) == 256 and all(len(row) == 16 for row in rows)
    assert len(values) == 4096 and hashlib.sha256(values).hexdigest() == SHA256
    packed = pack(rows)
    assert len(packed) == 2048
    assert bytes((packed[i >> 1] >> ((i & 1) * 4)) & 15 for i in range(4096)) == values
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        first, second = tmp / "first.h", tmp / "second.h"
        generate(SOURCE, first)
        generate(SOURCE, second)
        assert first.read_bytes() == second.read_bytes()
        assert bytes(int(x, 16) for x in re.findall(r"0x([0-9a-f]{2})", first.read_text())) == packed
        for kind in ("rows", "columns", "range", "first", "identity", "digest"):
            bad = [row[:] for row in rows]
            if kind == "rows": bad.pop()
            elif kind == "columns": bad[1].pop()
            elif kind == "range": bad[1][1] = 16
            elif kind == "first": bad[1][0] = 1
            elif kind == "identity": bad[0][1] = 2
            else: bad[10][4] ^= 1
            path = tmp / "invalid.txt"
            path.write_text("\n".join(" ".join(map(str, row)) for row in bad))
            try:
                read_rows(path)
            except ValueError:
                pass
            else:
                raise AssertionError(f"accepted invalid {kind}")
    symbols = subprocess.check_output(["nm", "-S", "--defined-only", str(args.object)], text=True)
    table = []
    for line in symbols.splitlines():
        fields = line.split()
        assert len(fields) == 4, line
        address, size, kind, name = fields
        assert kind.lower() not in "bcdgs", f"writable production storage: {line}"
        if name == "remix_packed":
            table.append((int(size, 16), kind.lower()))
    assert table == [(2048, "r")], table
    print(f"remix data: SHA-256 {SHA256}; 2048-byte packing, determinism and invalid-data rejection passed")
    print("remix storage: production table is 2048 bytes read-only; no writable static storage")


if __name__ == "__main__":
    main()
