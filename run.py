#!/usr/bin/env python3
import subprocess
import pathlib
import os
import sys

def run_compile() -> None:

    cmake_arg = ""
    generator = "-DVOLT_USE_GCC=ON"
    compiler = "-G \"Ninja\""

    if len(sys.argv) > 1:
        cmake_arg = sys.argv[1].lower()
    
    if cmake_arg == "msvc":
        compiler = "-DVOLT_USE_MSVC=ON"
        generator = "-G \"Visual Studio 17\""

    if sys.platform == "win32":
        result = subprocess.run("cls", shell=True)
    else:
        result = subprocess.run("clear", shell=True)

    result = subprocess.run(
        f"cmake --fresh -S . -B build {compiler} {generator} && cmake --build build",
        shell=True,
    )

    if result.returncode > 0:
        exit(result)

def main() -> None:
    
    root = pathlib.Path(".")
    project_name = os.path.dirname(os.path.abspath(__file__))

    build_dir = pathlib.Path("build")
    build_dir.mkdir(exist_ok=True)


    run_compile()

    cache = pathlib.Path("build/CMakeCache.txt").read_text()
    for line in cache.splitlines():
        if line.startswith("CMAKE_PROJECT_NAME:STATIC="):
            project_name = line.split("=", 1)[1]

    if sys.platform == "win32":
        subprocess.run(
            f".\\build\\{project_name}.exe test/test.volt -o test.obj", shell=True
        )
    else:
        subprocess.run(f"./build/{project_name} test/test.volt -o test.o", shell=True)


if __name__ == "__main__":
    main()