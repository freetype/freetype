#!/usr/bin/env python3
"""Generate shared-GSUB regression fonts using only Python's standard library.

Distributed under the FreeType project license, LICENSE.TXT.
"""

from pathlib import Path
import struct
import sys


def ushort(*values):
    return struct.pack(">" + "H" * len(values), *values)


def offset_list(items):
    offset = 2 + 2 * len(items)
    offsets = []
    for item in items:
        offsets.append(offset)
        offset += len(item)
    return ushort(len(items), *offsets) + b"".join(items)


def tagged_list(items):
    offset = 2 + 6 * len(items)
    records = b""
    for tag, item in items:
        records += tag + ushort(offset)
        offset += len(item)
    return ushort(len(items)) + records + b"".join(item for _, item in items)


def feature(lookups):
    return ushort(0, len(lookups), *lookups)


def script(features):
    return ushort(4, 0, 0, 0xFFFF, len(features), *features)


def single_subst(mapping):
    inputs = sorted(mapping)
    coverage = ushort(1, len(inputs), *inputs)
    subtable = ushort(2, 6 + 2 * len(inputs), len(inputs),
                     *(mapping[i] for i in inputs)) + coverage
    return ushort(1, 0, 1, 8) + subtable


def ligature_subst():
    # Glyphs 1 and 2 form glyph 5.  Coverage and LigatureSet follow the header.
    subtable = ushort(1, 8, 1, 14, 1, 1, 1, 1, 4, 5, 2, 2)
    return ushort(4, 0, 1, 8) + subtable


def feature_variations():
    records, data = b"", b""
    for minimum, maximum, lookup in [(0, 8192, 2), (8192, 16384, 3)]:
        condition = ushort(1) + struct.pack(">I", 6)
        condition += struct.pack(">HHhh", 1, 0, minimum, maximum)
        substitution = ushort(1, 0, 1, 2) + struct.pack(">I", 12)
        substitution += feature([lookup])
        offset = 24 + len(data)
        records += struct.pack(">II", offset, offset + len(condition))
        data += condition + substitution
    return ushort(1, 0) + struct.pack(">I", 2) + records + data


def gsub():
    scripts = tagged_list([(b"DFLT", script([0, 1, 2, 3])),
                           (b"cyrl", script([1, 2])),
                           (b"latn", script([0, 1, 2]))])
    features = tagged_list([(b"c2sc", feature([4])),
                            (b"liga", feature([0, 1])),
                            (b"rvrn", feature([])),
                            (b"ss01", feature([5]))])
    lookups = offset_list([
        single_subst({3: 12}),  # Reached only after the later rvrn lookup.
        ligature_subst(),
        single_subst({1: 3, 2: 4, 7: 8, 9: 11}),
        single_subst({1: 6}),  # Second FeatureVariations record.
        single_subst({1: 10}),  # Preserve the dedicated small-cap style.
        single_subst({1: 13}),  # Registered only under DFLT.
    ])
    return (struct.pack(">IHHHI", 0x10001, 14, 14 + len(scripts),
                        14 + len(scripts) + len(features),
                        14 + len(scripts) + len(features) + len(lookups))
            + scripts + features + lookups + feature_variations())


def glyph(height):
    points = [(0, 0), (0, height), (500, height), (500, 0),
              (80, 80), (420, 80), (420, height - 80), (80, height - 80)]
    xs, ys = [], []
    x, y = 0, 0
    for nx, ny in points:
        xs.append(nx - x)
        ys.append(ny - y)
        x, y = nx, ny
    return (struct.pack(">5h", 2, 0, 0, 500, height) + ushort(3, 7, 0)
            + bytes([1] * 8) + struct.pack(">8h", *xs)
            + struct.pack(">8h", *ys))


def checksum(data):
    data += bytes(-len(data) % 4)
    return sum(struct.unpack(">" + "I" * (len(data) // 4), data)) & 0xFFFFFFFF


def make_font(cyrillic):
    glyph_count = 14
    glyf, offsets = b"", [0, 0]
    for index in range(1, glyph_count):
        glyf += glyph(500 if index in (2, 4) else 700)
        offsets.append(len(glyf))
    mapping = {cp: 1 for cp in range(65, 91)}
    mapping.update({cp: 2 for cp in range(97, 123)})
    if cyrillic:
        mapping.update({0x041E: 7, 0x043E: 7})
    mapping[0xFFFF] = 0
    codes = sorted(mapping)
    segments = len(codes)
    power = 1 << (segments.bit_length() - 1)
    cmap = ushort(4, 16 + 8 * segments, 0, 2 * segments, 2 * power,
                  power.bit_length() - 1, 2 * segments - 2 * power)
    cmap += ushort(*codes, 0, *codes)
    cmap += struct.pack(">" + "h" * segments,
                        *((mapping[cp] - cp + 32768) % 65536 - 32768
                          for cp in codes))
    cmap += bytes(2 * segments)
    head = struct.pack(">IIIIHHQQhhhhHHhhh", 0x10000, 0x10000, 0, 0x5F0F3CF5,
                       0, 1000, 0, 0, 0, 0, 500, 700, 0, 8, 2, 1, 0)
    hhea = bytearray(36)
    struct.pack_into(">IhhhH", hhea, 0, 0x10000, 800, -200, 0, 600)
    struct.pack_into(">h", hhea, 18, 1)
    struct.pack_into(">H", hhea, 34, glyph_count)
    maxp = struct.pack(">I14H", 0x10000, glyph_count, 8, 2, 0, 0, 1, 0,
                       0, 0, 0, 0, 0, 0, 0)
    fvar = ushort(1, 0, 16, 2, 1, 20, 0, 8)
    fvar += struct.pack(">4siiiHH", b"wght", 0, 0, 65536, 0, 256)
    gvar_offset = 20 + 2 * (glyph_count + 1)
    gvar = struct.pack(">HHHHIHHI", 1, 0, 1, 0, gvar_offset,
                       glyph_count, 0, gvar_offset) + bytes(2 * (glyph_count + 1))
    names = [(1, "Shared GSUB Test"), (2, "Regular"),
             (4, "Shared GSUB Test Regular"), (6, "SharedGSUBTest-Regular"),
             (256, "Weight")]
    records, strings = b"", b""
    for name_id, value in names:
        encoded = value.encode("utf-16-be")
        records += ushort(3, 1, 0x409, name_id, len(encoded), len(strings))
        strings += encoded
    name = ushort(0, len(names), 6 + len(records)) + records + strings
    tables = {
        b"GSUB": gsub(), b"cmap": ushort(0, 1, 3, 1) + struct.pack(">I", 12) + cmap,
        b"fvar": fvar, b"glyf": glyf, b"gvar": gvar, b"head": head,
        b"hhea": bytes(hhea), b"hmtx": ushort(600, 0) * glyph_count,
        b"loca": struct.pack(">" + "I" * len(offsets), *offsets),
        b"maxp": maxp, b"name": name,
        b"post": struct.pack(">I", 0x30000) + bytes(28),
    }
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
    if len(sys.argv) != 3:
        raise SystemExit("usage: make-fonts.py latin.ttf mixed.ttf")
    for cyrillic, output in enumerate(sys.argv[1:]):
        Path(output).write_bytes(make_font(cyrillic))
