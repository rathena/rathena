#!/usr/bin/env python3
# [RO Sandbox] List or extract files from a client GRF archive.
#
#   python3 tools/sandbox/grf.py <file.grf> list <text>
#   python3 tools/sandbox/grf.py <file.grf> extract <text> <out folder>
#
# <text> filters paths (case-insensitive), e.g. "skillinfoz".
# Extracted files keep their GRF path under the out folder. They are
# Gravity client files: never commit them.
# Reads GRF versions 0x200 and 0x300, with zlib or LZMA data (GRF Editor's
# cps.dll compression). Standard library only.

import lzma
import os
import struct
import sys
import zlib

HEADER_SIZE = 46


def decode(raw, real):
    """File data -> bytes. Tries zlib, then LZMA; ValueError if neither fits."""
    try:
        return zlib.decompress(raw)
    except zlib.error:
        pass
    try:
        return lzma.decompress(raw, format=lzma.FORMAT_ALONE)[:real]
    except lzma.LZMAError:
        pass
    if len(raw) > 5:
        # Raw LZMA1: 5 property bytes, then the stream.
        props, dict_size = raw[0], struct.unpack_from("<I", raw, 1)[0]
        lc, lp, pb = props % 9, (props // 9) % 5, props // 45
        try:
            d = lzma.LZMADecompressor(format=lzma.FORMAT_RAW, filters=[{
                "id": lzma.FILTER_LZMA1, "dict_size": dict_size,
                "lc": lc, "lp": lp, "pb": pb}])
            data = d.decompress(raw[5:], max_length=real)
            if len(data) == real:
                return data
        except (lzma.LZMAError, ValueError):
            pass
    raise ValueError(raw[:16].hex(" "))


def read_table(path):
    """[(name, comp_size, aligned_size, real_size, flags, offset)]"""
    with open(path, "rb") as f:
        header = f.read(HEADER_SIZE)
        if not header.startswith(b"Master of Magic"):
            sys.exit(f"{path} is not a GRF file")
        version = struct.unpack_from("<I", header, 42)[0]
        if version == 0x200:
            table_offset, seed, count = struct.unpack_from("<III", header, 30)
            count = count - seed - 7
            f.seek(HEADER_SIZE + table_offset)
        elif version == 0x300:
            table_offset, count = struct.unpack_from("<QI", header, 30)
            f.seek(HEADER_SIZE + table_offset)
            f.read(4)  # unused
        else:
            sys.exit(f"{path}: GRF version {version:#x} is not supported")
        comp_len, real_len = struct.unpack("<II", f.read(8))
        table = zlib.decompress(f.read(comp_len))
    entries, pos = [], 0
    offset_fmt = "<Q" if version == 0x300 else "<I"
    offset_len = struct.calcsize(offset_fmt)
    while pos < len(table) and len(entries) < count:
        end = table.index(b"\0", pos)
        name = table[pos:end].decode("cp949", errors="replace")
        pos = end + 1
        comp, aligned, real, flags = struct.unpack_from("<IIIB", table, pos)
        pos += 13
        offset = struct.unpack_from(offset_fmt, table, pos)[0]
        pos += offset_len
        entries.append((name, comp, aligned, real, flags, offset))
    return version, entries


def main():
    if len(sys.argv) < 4 or sys.argv[2] not in ("list", "extract"):
        sys.exit("usage: grf.py <file.grf> list <text>\n"
                 "       grf.py <file.grf> extract <text> <out folder>")
    path, command, text = sys.argv[1], sys.argv[2], sys.argv[3].lower()
    version, entries = read_table(path)
    matches = [e for e in entries if e[4] & 1 and text in e[0].lower()]
    print(f"{path}: GRF {version:#x}, {len(entries)} files, {len(matches)} match '{text}'")
    if command == "list":
        for name, _, _, real, flags, _ in matches:
            note = " (encrypted)" if flags & 6 else ""
            print(f"  {real:>9}  {name}{note}")
        return
    if len(sys.argv) < 5:
        sys.exit("extract needs an out folder")
    out_dir = sys.argv[4]
    with open(path, "rb") as f:
        for name, comp, aligned, real, flags, offset in matches:
            if flags & 6:
                print(f"  skipped (encrypted): {name}")
                continue
            f.seek(HEADER_SIZE + offset)
            try:
                data = decode(f.read(aligned)[:comp], real) if real else b""
            except ValueError as e:
                print(f"  can't decode {name}: data starts {e}")
                continue
            out = os.path.join(out_dir, *name.replace("\\", "/").split("/"))
            os.makedirs(os.path.dirname(out), exist_ok=True)
            with open(out, "wb") as o:
                o.write(data)
            print(f"  {out}")


if __name__ == "__main__":
    main()
