#!/usr/bin/env python3
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).parent

def run(cmd):
    print("+", " ".join(map(str, cmd)))
    subprocess.check_call(cmd)

def main():
    src = ROOT / "examples" / "hello_world.gpac"
    out = ROOT / "examples" / "build" / ("hello_world.exe" if sys.platform.startswith("win") else "hello_world")
    run([sys.executable, str(ROOT / "src" / "gpac.py"), str(src), "-o", str(out)])
    print("Built:", out)

if __name__ == "__main__":
    main()
