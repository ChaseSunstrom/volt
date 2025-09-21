#!/usr/bin/env python3
import subprocess
import pathlib

# Directories to skip
EXCLUDE_DIRS = {"build", "vendor"}

# File extensions to format
EXTENSIONS = {".cpp", ".hpp", ".c", ".h"}

root = pathlib.Path(".")

for path in root.rglob("*"):
    if (
        path.is_file()
        and path.suffix in EXTENSIONS
        and not any(part in EXCLUDE_DIRS for part in path.parts)
    ):
        print(f"Formatting {path}")
        subprocess.run(["clang-format", "-i", "-verbose", "-style=file", str(path)])
