#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2026 OpenW3D Contributors.
"""Check the linked PE headers, independent of the compiler or CMake generator."""
import pathlib
import struct
import sys

if len(sys.argv) != 3:
    raise SystemExit("usage: verify_subsystems.py CLIENT.exe SERVER.exe")

for filename, expected in zip(sys.argv[1:], (2, 3)):
    image = pathlib.Path(filename).read_bytes()
    assert image[:2] == b"MZ", filename
    pe = struct.unpack_from("<I", image, 0x3C)[0]
    assert image[pe:pe + 4] == b"PE\0\0", filename
    subsystem = struct.unpack_from("<H", image, pe + 24 + 68)[0]
    assert subsystem == expected, f"{filename}: subsystem {subsystem}, expected {expected}"
    print(f"{filename}: {'GUI' if subsystem == 2 else 'console'} subsystem verified")
