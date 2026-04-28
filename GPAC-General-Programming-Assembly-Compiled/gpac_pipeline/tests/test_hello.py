from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]

def test_emit_hello():
    src = ROOT / "examples" / "hello_world.gpac"
    subprocess.check_call([sys.executable, str(ROOT / "src" / "gpac.py"), str(src), "--emit-only"])
    assert (ROOT / "examples" / "build" / "hello_world.cpp").exists()
    assert (ROOT / "examples" / "build" / "hello_world.ll").exists()
