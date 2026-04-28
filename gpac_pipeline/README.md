# GPAC — General Programming Assembly Compiled

This package contains the first real GPAC toolchain skeleton:

- ANTLR grammar: `grammar/GPAC.g4`
- C++ compiler frontend scaffold: `src/frontend.cpp`
- LLVM lowering scaffold: `src/llvm_lowering.cpp`
- ZVM runtime boundary: `runtime/gpac_zvm_runtime.hpp`
- Working bootstrap compiler: `src/gpac.py`
- Example `.gpac` programs: `examples/`

## Quick start

```bash
python build.py
```

Or compile a file directly:

```bash
python src/gpac.py examples/hello_world.gpac -o hello_world
```

On Windows:

```powershell
python src\gpac.py examples\hello_world.gpac -o hello_world.exe
```

## Toolchain requirements

For executable output, install one of:

- `clang++`
- `g++`

The bootstrap compiler always emits:

- generated C++
- textual LLVM IR artifact

## Production ANTLR command

Install ANTLR4 and generate C++ parser files:

```bash
antlr4 -Dlanguage=Cpp -visitor -o generated grammar/GPAC.g4
```

Then wire generated parser classes into `src/frontend.cpp`.

## Current supported bootstrap features

- comments using `--`
- `Main: program = name`
- procedures
- variables
- integers, floats, bools, strings
- print to console
- conditionals
- loops
- structs
- arrays through C++ vector lowering
- tracks through ZVM `run_track`
- simple native bridge example: `native_add`

## Status

This is a real bootstrap pipeline and a hardened architecture seed. It is not a full industrial compiler yet, but the files are concrete, valid, and ready for extension.
