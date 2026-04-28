#pragma once
#include <memory>
#include <string>
#include <vector>
#include <variant>

namespace gpac {

struct Expr;
struct Stmt;
using ExprPtr = std::shared_ptr<Expr>;
using StmtPtr = std::shared_ptr<Stmt>;

struct Expr {
    std::string text;
};

struct Stmt {
    std::string kind;
    std::string text;
};

struct Proc {
    std::string name;
    std::vector<std::string> params;
    std::vector<StmtPtr> body;
};

struct Program {
    std::string name;
    std::vector<Proc> procs;
};

}
