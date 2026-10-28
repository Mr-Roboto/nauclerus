"""Offline candidate verification. No hardware backend; no update/installation calls."""
import hashlib
import json
from pathlib import Path
import struct
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import fm1pkg_make as fmt
import version
from security_baseline import load, LOADER
from elf_syms import Elf, _sections

PIN = LOADER


def sha(b):
    return hashlib.sha256(b).hexdigest()


def require(ok, why):
    if not ok:
        raise ValueError(why)


def unpack(raw):
    require(len(raw) >= 960, 'truncated markers')
    product = ''.join(chr((raw[i*48+47]-i-1) & 255) for i in range(20) if raw[i*48+47] != 125)
    require(product == version.PRODUCT, 'unexpected package identity')
    logical = b''.join(raw[i*48:i*48+47] for i in range(20)) + raw[960:]
    return logical, product


def validate(logical, app, loader):
    require(len(logical) >= 1024, 'short UFW')
    h = bytearray(logical[:64]); fmt.enc(h, 0, len(h))
    require(fmt.crc16(h[2:]) == struct.unpack_from('<H', h)[0], 'header CRC')
    total, count = struct.unpack_from('<IH', h, 4)
    require(total == len(logical) and count == 2, 'length/entry count')
    listing = logical[64:64+count*80]
    require(fmt.crc16(listing) == struct.unpack_from('<H', h, 2)[0], 'entry list CRC')
    files = {}; ranges = []
    for i in range(count):
        e = bytearray(listing[i*80:(i+1)*80]); fmt.enc(e, 0, len(e))
        typ, index, crc, reserved, off, size, aligned = struct.unpack_from('<HHHHIII', e)
        name = bytes(e[64:]).split(b'\0')[0].decode()
        require(typ not in files and index == i and reserved == 0, 'duplicate/invalid entry')
        require(off >= 1024 and off % 256 == 0 and size <= total-off and aligned == (size+31)&~31, 'entry bounds')
        require(all(off+size <= a or off >= b for a, b in ranges), 'overlapping entries')
        body = logical[off:off+size]
        require(fmt.crc16(body) == crc, 'payload CRC: ' + name)
        ranges.append((off, off+size)); files[typ] = (body, name)
    require(set(files) == {0, 100}, 'wrong entry types')
    flash, name = files[0]; packed_loader, lname = files[100]
    require(name == 'flash.bin' and lname == 'ota.bin', 'wrong entry names')
    require(len(flash) == 0x93000, 'wrong flash image size')
    require(packed_loader == loader and sha(packed_loader) == PIN, 'loader pin/bytes')
    require(b'FELUCCA-LOADER-1' in loader, 'loader marker')
    region = bytearray(flash[0x4000:]); fmt.sfc(region, 0, len(region), 0, fmt.KEY)
    require(fmt.crc16(region[2:32]) == struct.unpack_from('<H', region)[0], 'app head CRC')
    extent = struct.unpack_from('<I', region, 8)[0]
    require(32 <= extent <= len(region), 'app head extent')
    require(fmt.crc16(region[32:extent]) == struct.unpack_from('<H', region, 2)[0], 'app area CRC')
    require(fmt.crc16(region[34:64]) == struct.unpack_from('<H', region, 32)[0], 'app entry CRC')
    require(struct.unpack_from('<II', region, 36) == (0x120, fmt.APP_SLOT), 'application bounds')
    padded = bytes(region[0x120:0x120+fmt.APP_SLOT])
    require(fmt.crc16(padded) == struct.unpack_from('<H', region, 34)[0], 'app payload CRC')
    require(padded == app + b'\xff'*(fmt.APP_SLOT-len(app)), 'candidate app byte mismatch')
    require(version.firmware_string(version.read()).encode()+b'\0' in app, 'firmware version marker')
    require(version.PRODUCT.encode()+b'\0' in app, 'application/package identity mismatch')
    return dict(product=version.PRODUCT, firmware_version=version.firmware_string(version.read()), app_bytes=len(app), app_sha256=sha(app),
                loader_bytes=len(loader), loader_sha256=sha(loader), flash_bytes=len(flash),
                logical_bytes=len(logical), package_entry_crcs='PASS', nested_application_crcs='PASS',
                embedded_application_byte_match=True, embedded_loader_byte_match=True)


def repack_list_crc(logical, entry_index, new_crc):
    data = bytearray(logical)
    off = 64 + entry_index*80
    e = bytearray(data[off:off+80]); fmt.enc(e, 0, 80)
    struct.pack_into('<H', e, 4, new_crc); fmt.enc(e, 0, 80); data[off:off+80] = e
    h = bytearray(data[:64]); fmt.enc(h, 0, 64)
    struct.pack_into('<H', h, 2, fmt.crc16(data[64:224]))
    struct.pack_into('<H', h, 0, fmt.crc16(h[2:])); fmt.enc(h, 0, 64); data[:64] = h
    return bytes(data)


def main():
    load()
    app = (ROOT/'build/felucca.bin').read_bytes()
    loader = (ROOT/'build/loader/ota.bin').read_bytes()
    raw = (ROOT/('build/nauclerus-'+version.read()+'.fwsc')).read_bytes()
    logical, product = unpack(raw)
    report = validate(logical, app, loader)
    report.update(package_bytes=len(raw), package_sha256=sha(raw))
    tests = []
    def rejected(label, body, expected):
        try:
            validate(body, app, loader)
        except ValueError as e:
            require(expected in str(e), 'wrong failure: '+label+' '+str(e))
            tests.append(label)
        else:
            raise ValueError('corruption accepted: '+label)
    corrupt = bytearray(logical); corrupt[5] ^= 1
    rejected('header corruption', corrupt, 'header CRC')
    corrupt = bytearray(logical); corrupt[70] ^= 1
    rejected('list corruption', corrupt, 'entry list CRC')
    rejected('truncated artifact', logical[:-1], 'length/entry count')
    corrupt = bytearray(logical); corrupt[0x400+0x5000] ^= 1
    rejected('app corruption', corrupt, 'payload CRC')
    # Deliberately repair outer CRCs: candidate byte and loader pins must still catch the change.
    loader_off = (0x400 + 0x93000 + 255) & ~255
    corrupt = bytearray(logical); corrupt[loader_off+100] ^= 1
    rejected('loader corruption', corrupt, 'payload CRC')
    corrupt = repack_list_crc(corrupt, 1, fmt.crc16(corrupt[loader_off:]))
    rejected('loader corruption with repaired CRCs', corrupt, 'loader pin/bytes')
    report['negative_integrity_tests'] = tests
    elf = Elf(ROOT/'build/felucca.elf')
    usb=[(a,n) for a,n,s in elf.syms if s=='STR2']
    expected_usb=bytes([20,3])+version.USB_PRODUCT.encode('utf-16le')
    require(len(usb)==1 and usb[0][1]==20 and elf.bytes_at(usb[0][0],20)==expected_usb,'compiled USB product')
    dev=[(a,n) for a,n,s in elf.syms if s=='DEV_DESC']
    require(len(dev)==1 and elf.bytes_at(dev[0][0]+8,4)==bytes.fromhex('09120100'),'USB VID/PID changed')
    report['usb_product']=version.USB_PRODUCT
    report['usb_vid']='1209';report['usb_pid']='0001'

    target_sizes = {s: n for a, n, s in elf.syms}
    require(target_sizes.get('seq_remix_config') == 18 and target_sizes.get('seq_remix_pending') == 18,
            'target must contain eight Bass plus one global Drum uint16 committed/pending configurations')
    require(target_sizes.get('proj_slot') == 4*3840, 'target project slot footprint changed')
    require(b'REMIX\0' in app, 'REMIX UI label absent from application')
    report['target_track_configurations'] = 8
    report['target_remix_scopes'] = 9
    report['target_project_slot_bytes'] = target_sizes['proj_slot']
    packed = [(a, n, s) for a,n,s in elf.syms if s == 'remix_packed']
    require(len(packed) == 1 and packed[0][1] == 2048, 'production table symbol/size')
    a, n, _ = packed[0]
    d = (ROOT/'build/felucca.elf').read_bytes()
    secs = [s for s in _sections(d) if s[3] <= a and a+n <= s[3]+s[5] and s[1] == 1]
    require(len(secs) == 1 and not (secs[0][2] & 1), 'production table is writable')
    table = elf.bytes_at(a, n)
    values = bytes((table[i//2] >> ((i%2)*4)) & 15 for i in range(4096))
    require(sha(values) == '88034625d620ce4a1a87f954edf81850adf261155a7891fabfea68b01b6d60cb', 'compiled canonical table hash')
    require(b'remix_test_select' not in d, 'host activation hook leaked into target')
    report.update(compiled_table_bytes=n, compiled_table_sha256=sha(values), compiled_table_writable=False)
    noinit = struct.unpack_from('<H', d, 0x32)[0]  # section-name string table index
    sections = _sections(d); st = sections[noinit]; strings=d[st[4]:st[4]+st[5]]
    sizes = {}
    for s in sections:
        name = strings[s[0]:].split(b'\0')[0].decode()
        if name in ('.data','.bss','.pool','.noinit'): sizes[name]=s[5]
    report['sections'] = sizes
    report['security_status'] = 'LOCAL_RC_BASELINE_ADOPTED_HARDWARE_UNTESTED'
    (ROOT/'build/candidate-integrity.json').write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
