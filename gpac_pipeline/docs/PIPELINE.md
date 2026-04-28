# GPAC Pipeline

## Permanent production architecture

```text
.gpac source
  -> ANTLR lexer/parser from grammar/GPAC.g4
  -> C++ AST
  -> semantic validation
  -> GPAC IR
  -> GPAC bytecode (.gpbc)
  -> ZVM verifier
  -> LLVM lowering
  -> LLVM optimization pipeline
  -> native object / executable
```

## Included bootstrap path

```text
.gpac source
  -> Python bootstrap parser
  -> C++ source
  -> O3 native executable through clang++/g++
  -> .ll lowering artifact
```

This gives the language a working execution lane while the ANTLR-generated C++ frontend is completed.
