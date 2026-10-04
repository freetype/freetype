#!/usr/bin/env python3
"""Construct synthetic fonts for the composite-count regression.

No font files or third-party Python packages are used.  Glyph 0 is empty;
glyph 1 references it repeatedly.  The wght axis has no glyph deltas: selecting
its non-default coordinate alone exercises the variable-composite path.
This file is distributed under the FreeType project license, LICENSE.TXT.
"""
import hashlib
from pathlib import Path
import struct
import sys


def checksum(data):
    data += b"\0" * (-len(data) % 4)
    return sum(struct.unpack(">" + "I" * (len(data) // 4), data)) & 0xFFFFFFFF


def make_font(components):
    head = struct.pack(">IIIIHHQQhhhhHHhhh", 0x10000, 0x10000, 0, 0x5F0F3CF5,
                       0, 1000, 0, 0, 0, 0, 0, 0, 0, 8, 2, 1, 0)
    hhea = bytearray(36)
    struct.pack_into(">IhhhH", hhea, 0, 0x10000, 800, -200, 0, 500)
    struct.pack_into(">h", hhea, 18, 1)  # caretSlopeRise
    struct.pack_into(">H", hhea, 34, 2)  # numberOfHMetrics
    maxp = struct.pack(">I14H", 0x10000, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0,
                       0, 0, components, 1)
    glyf = struct.pack(">5h", -1, 0, 0, 0, 0)
    # ARGS_ARE_XY_VALUES, with MORE_COMPONENTS on all but the last entry.
    glyf += struct.pack(">HHbb", 0x22, 0, 0, 0) * (components - 1)
    glyf += struct.pack(">HHbb", 0x02, 0, 0, 0)
    fvar = struct.pack(">8H", 1, 0, 16, 2, 1, 20, 0, 8)
    fvar += struct.pack(">4siiiHH", b"wght", 0, 0, 65536, 0, 256)
    gvar = struct.pack(">HHHHIHHI3H", 1, 0, 1, 0, 26, 2, 0, 26, 0, 0, 0)
    # Compact format 4: two segments, A -> glyph 1 and the terminal sentinel.
    cmap4 = struct.pack(">7H", 4, 32, 0, 4, 4, 1, 0)
    cmap4 += struct.pack(">5H2h2H", 65, 65535, 0, 65, 65535, -64, 1, 0, 0)
    names = [(1, "Composite Count Test"), (2, "Regular"),
             (4, "Composite Count Test Regular"),
             (6, "CompositeCountTest-Regular"), (256, "Weight")]
    records, strings = bytearray(), bytearray()
    for name_id, value in names:
        encoded = value.encode("utf-16-be")
        records += struct.pack(">6H", 3, 1, 0x409, name_id, len(encoded), len(strings))
        strings += encoded
    name = struct.pack(">3H", 0, len(names), 6 + len(records)) + records + strings
    tables = {
        b"cmap": struct.pack(">HHHHI", 0, 1, 3, 1, 12) + cmap4,
        b"fvar": fvar, b"glyf": glyf, b"gvar": gvar, b"head": head,
        b"hhea": bytes(hhea), b"hmtx": struct.pack(">HhHh", 500, 0, 500, 0),
        b"loca": struct.pack(">3I", 0, 0, len(glyf)), b"maxp": maxp,
        b"name": name, b"post": struct.pack(">I", 0x30000) + bytes(28),
    }
    count = len(tables)
    power = 1 << (count.bit_length() - 1)
    font = bytearray(struct.pack(">I4H", 0x10000, count, power * 16,
                                 power.bit_length() - 1, count * 16 - power * 16))
    font += bytes(16 * count)
    head_offset = None
    for index, (tag, data) in enumerate(sorted(tables.items())):
        offset = len(font)
        struct.pack_into(">4sIII", font, 12 + 16 * index, tag, checksum(data), offset, len(data))
        font += data + bytes(-len(data) % 4)
        if tag == b"head":
            head_offset = offset
    struct.pack_into(">I", font, head_offset + 8, (0xB1B0AFBA - checksum(font)) & 0xFFFFFFFF)
    assert checksum(font) == 0xB1B0AFBA
    return font


if __name__ == "__main__":
    if len(sys.argv) != 5:
        raise SystemExit("usage: make-fonts.py output32764 output32765 output32766 output32767")
    for components, output in zip(range(32764, 32768), sys.argv[1:]):
        data = make_font(components)
        Path(output).write_bytes(data)
        print(f"{components} {len(data)} {hashlib.sha256(data).hexdigest()} {Path(output).name}")
