#!/usr/bin/env python3

import argparse
import itertools
import pathlib
import re
import sys

# FIXME: use #embed when switching to c++26

def main():
    parser = argparse.ArgumentParser(allow_abbrev=False)
    parser.add_argument("sources", nargs="+")
    parser.add_argument("-o", "--output", dest="output_source", type=pathlib.Path)
    parser.add_argument("--oh", dest="output_header", type=pathlib.Path)
    args = parser.parse_args()

    sources = {}
    varnames = set()
    for arg_src in args.sources:
        parts = arg_src.rsplit("@", 1)
        if len(parts) == 1:
            path = pathlib.Path(parts[0])
            name = path.name
        else:
            path = pathlib.Path(parts[0])
            name = parts[1]
        name = name.lower()
        varname = "var_" + re.subn("[^a-zA-Z_$]","_", name)[0]
        if not path.is_file():
            parser.error(f"{path} is not a file")
        if name in sources:
            parser.error(f"Duplicate: {name}")
        if varname in varnames:
            parser.error(f"{path}@{varname} is too similar to another path")
        varnames.add(varname)
        sources[name] = (path, varname)

    if not sources:
        parser.error("Missing sources")

    with (args.output_source.open("w", newline="\n") if args.output_source else sys.stdout) as f_out:
        f_out.write("#include \"rcfile.h\"\n")
        f_out.write("#include <array>\n")
        f_out.write("\n")
        f_out.write("#ifdef OPENW3D_SDL3\n")
        f_out.write("\n")
        f_out.write(f"namespace {{\n")
        for name, (path, varname) in sources.items():
            data_in = path.read_bytes()
            f_out.write(f"// {path.name}\n")
            f_out.write(f"std::array<std::uint8_t, {len(data_in)}> {varname} = {{\n")
            for byte_batch in itertools.batched(data_in, n=16):
                for b in byte_batch:
                    f_out.write(f" 0x{b:02x},")
                f_out.write("\n")
            f_out.write(f"}};\n")
            f_out.write(f"\n")
        f_out.write(f"}} // namespace\n")
        f_out.write(f"\n")
        f_out.write(f"std::unordered_map<std::string, StaticResourceFileClass> Static_Resources = {{\n")
        for name, (path, varname) in sources.items():
            f_out.write(f"  {{ \"{name}\", {{ \"{name}\", {varname}.data(), {varname}.size(), }}, }},\n")
        f_out.write(f"}};\n")
        f_out.write(f"#endif // OPENW3D_SDL3\n")


if __name__ == "__main__":
    raise SystemExit(main())
