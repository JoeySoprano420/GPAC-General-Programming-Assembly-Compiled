#pragma once
#include "ir.hpp"
#include "bytecode.hpp"
#include <string>

namespace gpac::backend {

BCModule assemble_to_bytecode(const Module& module);
bool verify_bytecode(const BCModule& module, std::string& error);
std::string emit_llvm_text(const BCModule& module);
std::string emit_native_cpp(const BCModule& module);

} // namespace gpac::backend
