; GPAC LLVM IR lowering artifact for module: hello_world
; Production backend lowers typed GPAC IR directly to LLVM Module.
; Bootstrap backend emits C++ and invokes clang++/g++.

declare i32 @puts(ptr)

define i32 @main() {
entry:
  ret i32 0
}
