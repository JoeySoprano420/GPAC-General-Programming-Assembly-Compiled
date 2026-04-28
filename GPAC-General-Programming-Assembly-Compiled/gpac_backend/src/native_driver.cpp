#include "../include/gpac_backend/backend_api.hpp"
#include <sstream>

namespace gpac::backend {

std::string emit_native_cpp(const BCModule& module) {
    std::ostringstream out;
    out << "#include <iostream>\n";
    out << "#include <string>\n";
    out << "#include <vector>\n\n";

    out << "struct GPAC_ZVM_Runtime {\n";
    out << "    void print(const std::string& s) { std::cout << s << std::endl; }\n";
    out << "    void print(int v) { std::cout << v << std::endl; }\n";
    out << "};\n\n";

    out << "int main() {\n";
    out << "    GPAC_ZVM_Runtime rt;\n";

    for (const auto& s : module.string_pool) {
        (void)s;
    }

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
                out << "    std::string r" << inst.dst << " = ";
                const auto& s = module.string_pool[inst.a];
                out << "\"";
                for (char c : s) {
                    if (c == '\\') out << "\\\\";
                    else if (c == '"') out << "\\\"";
                    else out << c;
                }
                out << "\";\n";
            } else if (inst.opcode == Opcode::Print) {
                out << "    rt.print(r" << inst.a << ");\n";
            }
        }
    }

    out << "    return 0;\n";
    out << "}\n";
    return out.str();
}

} // namespace gpac::backend
