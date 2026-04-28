# GPAC Backend Bundle

This is the backend companion package for GPAC.

It contains the concrete backend side of the pipeline:

```text
GPAC IR
  -> bytecode assembly
  -> bytecode verifier
  -> ZVM module loader
  -> LLVM IR emission
  -> native object/exe driver
```

## Included components

- `include/gpac_backend/ir.hpp` — GPAC SSA-style IR model
- `include/gpac_backend/bytecode.hpp` — GPAC bytecode instruction format
- `src/assembler.cpp` — IR-to-bytecode assembler
- `src/verifier.cpp` — bytecode verifier
- `src/llvm_backend.cpp` — LLVM IR text backend
- `src/native_driver.cpp` — clang/LLVM native compilation driver
- `runtime/zvm_loader.hpp` — ZVM module loading/runtime interface
- `tools/gpac_backend.py` — working backend bootstrap tool
- `examples/hello.gpir` — sample GPAC IR
- `docs/BACKEND_PIPELINE.md` — backend architecture notes

## Quick start

```bash
python tools/gpac_backend.py examples/hello.gpir --emit-all
```

This emits:

```text
build/hello.gpbc.json
build/hello.ll
build/hello.cpp
```

To build an executable when `clang++` or `g++` is installed:

```bash
python tools/gpac_backend.py examples/hello.gpir --exe build/hello
```

On Windows:

```powershell
python tools\gpac_backend.py examples\hello.gpir --exe build\hello.exe
```

## Status

This backend is a real bootstrap backend with a hardened layout. It is built to accept GPAC IR from the frontend bundle and lower it into bytecode, LLVM text IR, and native C++ execution output.
