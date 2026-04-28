#include "../runtime/gpac_zvm_runtime.hpp"
#include <string>
#include <vector>
#include <iostream>
using namespace gpac::zvm;
Runtime __rt;
using gpac::zvm::native_add;

void main();

void main() {
    __rt.print("Hello, World!");
}

int main() { main(); return 0; }