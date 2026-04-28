#include <sstream>
#include <string>

namespace gpac {

std::string emit_minimal_llvm_module(const std::string& module_name) {
    std::ostringstream out;
    out << "; GPAC LLVM lowering module: " << module_name << "\n";
    out << "declare i32 @puts(ptr)\n\n";
    out << "define i32 @main() {\n";
    out << "entry:\n";
    out << "  ret i32 0\n";
    out << "}\n";
    return out.str();
}

}
