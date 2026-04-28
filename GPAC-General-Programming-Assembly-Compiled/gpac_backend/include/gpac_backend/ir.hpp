#pragma once
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <unordered_map>

namespace gpac::backend {

enum class ValueType {
    Void,
    Int,
    Float,
    Bool,
    String,
    Ptr
};

struct Value {
    std::string name;
    ValueType type = ValueType::Void;
};

enum class IROp {
    ConstInt,
    ConstFloat,
    ConstString,
    Add,
    Sub,
    Mul,
    Div,
    CmpLT,
    CmpGT,
    Print,
    Call,
    Ret,
    Br,
    CBr
};

struct IRInst {
    IROp op;
    std::string dst;
    std::vector<std::string> args;
    std::string literal;
    ValueType type = ValueType::Void;
};

struct BasicBlock {
    std::string name;
    std::vector<IRInst> instructions;
};

struct Function {
    std::string name;
    ValueType return_type = ValueType::Void;
    std::vector<Value> params;
    std::vector<BasicBlock> blocks;
};

struct Module {
    std::string name;
    std::vector<Function> functions;
};

} // namespace gpac::backend
