#!/usr/bin/env python3
# Copyright 2020 VUKOZ
#
# This file is part of 3D Forest.
#
# 3D Forest is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# 3D Forest is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with 3D Forest.  If not, see <https://www.gnu.org/licenses/>.

"""Compile a Qt-independent subset of .qrc XML to ordinary C++17 arrays."""

import argparse
from pathlib import Path, PurePosixPath
import re
import sys
import xml.etree.ElementTree as ET


def resource_name(prefix, alias):
    if "\\" in prefix or "\\" in alias or ":" in prefix or ":" in alias:
        raise ValueError("Resource names must use forward slashes, without a drive prefix")
    if not alias or alias.startswith("/"):
        raise ValueError("A file needs a non-empty relative resource name (or alias)")
    parts = (prefix.strip("/") + "/" + alias).split("/")
    if ".." in parts:
        raise ValueError("Resource names cannot contain '..'; use a file alias")
    return ":/" + "/".join(part for part in parts if part not in ("", "."))


def parse_manifest(filename):
    manifest = Path(filename).resolve()
    root = ET.parse(manifest).getroot()
    if root.tag != "RCC" or set(root.attrib) - {"version"}:
        raise ValueError("Expected <RCC> with only an optional version attribute")
    entries = []
    names = set()
    for group in root:
        if group.tag != "qresource" or set(group.attrib) - {"prefix"}:
            raise ValueError("Only <qresource prefix=...> is supported; no locale selection")
        for node in group:
            if node.tag != "file" or set(node.attrib) - {"alias"} or len(node):
                raise ValueError("Only <file alias=...> is supported; no compression attributes")
            source_name = (node.text or "").strip()
            if not source_name:
                raise ValueError("Empty <file> entry")
            alias = node.get("alias", source_name.replace("\\", "/"))
            name = resource_name(group.get("prefix", "/"), alias)
            if name in names:
                raise ValueError("Duplicate resource: " + name)
            names.add(name)
            source = (manifest.parent / source_name).resolve()
            if not source.is_file():
                raise ValueError("Resource file does not exist: " + str(source))
            # CMake's dependency list cannot faithfully carry these characters.
            if any(char in str(source) for char in (";", "\n", "\r")):
                raise ValueError("Unsupported character in dependency path: " + str(source))
            entries.append((name, source))
    return entries


def cpp_string(value):
    # Encode UTF-8 bytes explicitly, independent of MSVC's source encoding.
    result = '"'
    for byte in value.encode("utf-8"):
        if byte == 34:
            result += '\\"'
        elif byte == 92:
            result += "\\\\"
        elif 32 <= byte < 127:
            result += chr(byte)
        else:
            result += "\\%03o" % byte
    return result + '"'


def generate(entries, name, out_dir):
    if not re.fullmatch(r"[A-Za-z][A-Za-z0-9_]*", name):
        raise ValueError("Bundle name must be a C++ identifier beginning with a letter")
    if "__" in name:
        raise ValueError("Bundle name cannot contain reserved double underscores")
    out_dir = Path(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    header = out_dir / (name + ".hpp")
    source = out_dir / (name + ".cpp")
    header.write_text(
        "// Generated; do not edit.\n#pragma once\n"
        "#include <ResourceBundle.hpp>\n\n"
        "ResourceBundle register" + name + "();\n", encoding="utf-8")
    lines = ["// Generated; do not edit.", '#include "' + header.name + '"', "", "namespace", "{"]
    sizes = []
    for index, (_, filename) in enumerate(entries):
        data = filename.read_bytes()
        sizes.append(len(data))
        lines.append("const std::uint8_t resource_" + str(index) + "[] = {")
        if not data:
            lines.append("    0 // Placeholder storage for a zero-byte resource.")
        for offset in range(0, len(data), 16):
            lines.append("    " + ", ".join("0x%02x" % b for b in data[offset:offset + 16]) + ",")
        lines.append("};")
    lines += ["}", "", "ResourceBundle register" + name + "()", "{"]
    if entries:
        lines.append("    return ResourceBundle({")
        for index, (path, _) in enumerate(entries):
            lines.append("        {" + cpp_string(path) + ", resource_" + str(index) + ", " + str(sizes[index]) + "},")
        lines.append("    });")
    else:
        lines.append("    return ResourceBundle();")
    lines.append("}")
    source.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--qrc", required=True)
    parser.add_argument("--name")
    parser.add_argument("--out-dir")
    parser.add_argument("--list", action="store_true", help="Print absolute source dependencies")
    args = parser.parse_args()
    try:
        entries = parse_manifest(args.qrc)
        if args.list:
            for filename in sorted({source for _, source in entries}):
                print(filename.as_posix())
        else:
            if not args.name or not args.out_dir:
                parser.error("generation requires --name and --out-dir")
            generate(entries, args.name, args.out_dir)
    except (ValueError, OSError, ET.ParseError) as error:
        print("embed_resources: " + str(error), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
