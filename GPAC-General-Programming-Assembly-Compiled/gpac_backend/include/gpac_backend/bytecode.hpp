#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace gpac::backend {

enum class Opcode : std::uint16_t {
    Nop = 0,
    ConstInt = 1,
    ConstFloat = 2,
    ConstString = 3,
    AddI = 10,
    SubI = 11,
    MulI = 12,
    DivI = 13,
    CmpLT = 20,
    CmpGT = 21,
    Print = 30,
    Call = 40,
    Ret = 50,
    Halt = 255
};

struct BCInst {
    Opcode opcode = Opcode::Nop;
    std::uint32_t dst = 0;
    std::uint32_t a = 0;
    std::uint32_t b = 0;
    std::string text;
};

struct BCFunction {
    std::string name;
    std::vector<BCInst> code;
};

struct BCModule {
    std::string name;
    std::vector<std::string> string_pool;
    std::vector<BCFunction> functions;
};

} // namespace gpac::backend
