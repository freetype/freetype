#!/usr/bin/env python3
"""Add HVAR or VVAR to the static VARC/gvar fixture using only stdlib.

The component sets the private axis to a nonzero coordinate.  Loading its
metrics must therefore parse an item variation store without an 'fvar'.
Distributed under the FreeType project license, LICENSE.TXT.
"""

from pathlib import Path
import struct
import sys


def checksum(data):
    data += bytes(-len(data) % 4)
    return sum(struct.unpack(">" + "I" * (len(data) // 4), data)) & 0xFFFFFFFF


def read_tables(data):
    count, = struct.unpack_from(">H", data, 4)
    tables = {}
    for index in range(count):
        tag, _, offset, size = struct.unpack_from(">4sIII", data, 12 + 16 * index)
        tables[tag] = data[offset:offset + size]
    return tables


def make_font(tables, vertical):
    tables = tables.copy()
    axis_count, = struct.unpack_from(">H", tables[b"gvar"], 4)
    glyph_count, = struct.unpack_from(">H", tables[b"maxp"], 4)
    assert b"fvar" not in tables and axis_count == 1

    # One positive-axis region and one delta set per glyph, with implicit
    # glyph-to-delta mapping.  The component is glyph 2.
    store = struct.pack(">HIHI", 1, 12, 1, 22)
    store += struct.pack(">HHhhh", 1, 1, 0, 16384, 16384)
    store += struct.pack(">4H", glyph_count, 1, 1, 0)
    store += struct.pack(">" + "h" * glyph_count,
                         *(25 if i == 2 else 0 for i in range(glyph_count)))
    if vertical:
        tables[b"VVAR"] = struct.pack(">HH5I", 1, 0, 24, 0, 0, 0, 0) + store
        vhea = bytearray(tables[b"hhea"])
        struct.pack_into(">H", vhea, 10, 1000)  # advanceHeightMax
        struct.pack_into(">H", vhea, 34, glyph_count)
        tables[b"vhea"] = bytes(vhea)
        tables[b"vmtx"] = struct.pack(">Hh", 1000, 0) * glyph_count
    else:
        tables[b"HVAR"] = struct.pack(">HH4I", 1, 0, 20, 0, 0, 0) + store

    head = bytearray(tables[b"head"])
    struct.pack_into(">I", head, 8, 0)
    tables[b"head"] = bytes(head)
    count = len(tables)
    power = 1 << (count.bit_length() - 1)
    font = bytearray(struct.pack(">I4H", 0x10000, count, power * 16,
                                 power.bit_length() - 1, count * 16 - power * 16))
    font += bytes(16 * count)
    for index, (tag, data) in enumerate(sorted(tables.items())):
        offset = len(font)
        struct.pack_into(">4sIII", font, 12 + 16 * index,
                         tag, checksum(data), offset, len(data))
        font += data + bytes(-len(data) % 4)
        if tag == b"head":
            head_offset = offset
    struct.pack_into(">I", font, head_offset + 8,
                     (0xB1B0AFBA - checksum(font)) & 0xFFFFFFFF)
    return font


if __name__ == "__main__":
    if len(sys.argv) != 4:
        raise SystemExit("usage: make-hvvar-fonts.py input.ttf hvar.ttf vvar.ttf")
    tables = read_tables(Path(sys.argv[1]).read_bytes())
    for vertical, output in enumerate(sys.argv[2:]):
        Path(output).write_bytes(make_font(tables, vertical))
