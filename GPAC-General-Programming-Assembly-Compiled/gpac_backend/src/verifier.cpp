#include "../include/gpac_backend/backend_api.hpp"
#include <sstream>

namespace gpac::backend {

bool verify_bytecode(const BCModule& module, std::string& error) {
    if (module.name.empty()) {
        error = "module name is empty";
        return false;
    }

    if (module.functions.empty()) {
        error = "module has no functions";
        return false;
    }

    bool has_main = false;

    for (const auto& fn : module.functions) {
        if (fn.name == "main") has_main = true;

        if (fn.code.empty()) {
            error = "function has no bytecode: " + fn.name;
            return false;
        }

        bool has_ret = false;
        std::uint32_t max_written_reg = 0;

        for (std::size_t i = 0; i < fn.code.size(); ++i) {
            const auto& inst = fn.code[i];

            switch (inst.opcode) {
                case Opcode::ConstString:
                    if (inst.a >= module.string_pool.size()) {
                        error = "CONST_STRING references invalid string pool index";
                        return false;
                    }
                    max_written_reg = inst.dst > max_written_reg ? inst.dst : max_written_reg;
                    break;

                case Opcode::ConstInt:
                case Opcode::ConstFloat:
                    max_written_reg = inst.dst > max_written_reg ? inst.dst : max_written_reg;
                    break;

                case Opcode::AddI:
                case Opcode::SubI:
                case Opcode::MulI:
                case Opcode::DivI:
                    if (inst.a > max_written_reg || inst.b > max_written_reg) {
                        error = "arithmetic reads register before definition";
                        return false;
                    }
                    max_written_reg = inst.dst > max_written_reg ? inst.dst : max_written_reg;
                    break;

                case Opcode::Print:
                    if (inst.a > max_written_reg) {
                        error = "PRINT reads register before definition";
                        return false;
                    }
                    break;

                case Opcode::Ret:
                    has_ret = true;
                    break;

                case Opcode::Nop:
                case Opcode::Halt:
                case Opcode::CmpLT:
                case Opcode::CmpGT:
                case Opcode::Call:
                    break;
            }
        }

        if (!has_ret) {
            error = "function does not return: " + fn.name;
            return false;
        }
    }

    if (!has_main) {
        error = "module has no main function";
        return false;
    }

    error.clear();
    return true;
}

} // namespace gpac::backend
