# GPAC Backend Pipeline

## Backend responsibility

The GPAC backend begins after parsing and semantic analysis.

```text
Frontend:
.gpac -> AST -> semantic model

Backend:
semantic model -> GPAC IR -> bytecode -> verifier -> LLVM/native execution
```

## Backend stages

### 1. GPAC IR

GPAC IR is a typed, block-oriented intermediate representation.

Core operations:

```text
const.string
const.int
const.float
add
sub
mul
div
cmp.lt
cmp.gt
print
call
ret
br
cbr
```

### 2. Bytecode assembly

The assembler converts GPAC IR into compact VM-readable instructions.

Example:

```text
CONST_STR 0 "Hello"
PRINT r0
RET
```

### 3. Verification

The verifier checks:

```text
valid opcode
valid register reads
valid constant references
valid function boundary
valid return behavior
valid stack/register flow
```

### 4. LLVM lowering

The backend emits LLVM IR for native execution.

The bootstrap backend emits readable LLVM text. The production backend links directly to LLVM C++ APIs.

### 5. Native compilation

The native driver invokes clang/g++ for the bootstrap path, while the production backend emits object files directly through LLVM TargetMachine.

## ZVM role

The ZVM runtime is the sealed execution boundary. It loads verified modules and executes native-lowered code without interpreter dispatch in hot paths.
