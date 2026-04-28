from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]

def test_backend_emit_all():
    src = ROOT / "examples" / "hello.gpir"
    subprocess.check_call([sys.executable, str(ROOT / "tools" / "gpac_backend.py"), str(src), "--emit-all"])
    assert (ROOT / "examples" / "build" / "hello.gpbc.json").exists()
    assert (ROOT / "examples" / "build" / "hello.ll").exists()
    assert (ROOT / "examples" / "build" / "hello.cpp").exists()
