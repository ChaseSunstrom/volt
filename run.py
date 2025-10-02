#!/usr/bin/env python3
import subprocess
import pathlib
import os
import sys

root = pathlib.Path(".")
project_name = os.path.dirname(os.path.abspath(__file__))

subprocess.run(
    f"cd build && cmake .. && cmake --build . && cd ..",
    shell=True,
)

cache = pathlib.Path("build/CMakeCache.txt").read_text()
for line in cache.splitlines():
    if line.startswith("CMAKE_PROJECT_NAME:STATIC="):
        project_name = line.split("=", 1)[1]

if sys.platform == "win32":
    subprocess.run(f".\\build\\{project_name}.exe test.volt -o test.obj", shell=True)
else:
    subprocess.run(f"./build/{project_name} test.volt -o test.o", shell=True)
