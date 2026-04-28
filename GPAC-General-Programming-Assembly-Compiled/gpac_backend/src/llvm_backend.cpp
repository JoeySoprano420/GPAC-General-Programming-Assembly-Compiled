#include "../include/gpac_backend/backend_api.hpp"
#include <sstream>

namespace gpac::backend {

std::string emit_llvm_text(const BCModule& module) {
    std::ostringstream out;
    out << "; GPAC LLVM text backend\n";
    out << "; module: " << module.name << "\n\n";
    out << "declare i32 @puts(ptr)\n\n";

    for (std::size_t i = 0; i < module.string_pool.size(); ++i) {
        const auto& s = module.string_pool[i];
        out << "@.gpac.str." << i << " = private unnamed_addr constant ["
            << (s.size() + 1) << " x i8] c\"";
        for (char c : s) {
            if (c == '\\') out << "\\5C";
            else if (c == '"') out << "\\22";
            else if (c == '\n') out << "\\0A";
            else out << c;
        }
        out << "\\00\"\n";
    }

    out << "\n";
    out << "define i32 @main() {\n";
    out << "entry:\n";

    const BCFunction* main_fn = nullptr;
    for (const auto& fn : module.functions) {
        if (fn.name == "main") {
            main_fn = &fn;
            break;
        }
    }

    if (main_fn) {
        for (const auto& inst : main_fn->code) {
            if (inst.opcode == Opcode::ConstString) {
                out << "  %r" << inst.dst << " = getelementptr ["
                    << (module.string_pool[inst.a].size() + 1)
                    << " x i8], ptr @.gpac.str." << inst.a << ", i32 0, i32 0\n";
            } else if (inst.opcode == Opcode::Print) {
                out << "  call i32 @puts(ptr %r" << inst.a << ")\n";
            }
        }
    }

    out << "  ret i32 0\n";
    out << "}\n";
    return out.str();
}

} // namespace gpac::backend
