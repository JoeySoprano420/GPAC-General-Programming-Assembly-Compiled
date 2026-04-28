#include "../include/gpac/ast.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace gpac {

// This is the C++ frontend integration point.
// Production builds generate GPACLexer/GPACParser from grammar/GPAC.g4 using ANTLR4.
// The Python bootstrap compiler included in this package allows immediate .gpac -> .cpp/.ll/.exe use.

Program parse_with_antlr_frontend_placeholder(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("cannot open GPAC source");
    }
    Program p;
    p.name = "antlr_frontend_pending_generation";
    return p;
}

}
