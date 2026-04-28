#pragma once
#include "../include/gpac_backend/bytecode.hpp"
#include <stdexcept>
#include <iostream>
#include <vector>
#include <variant>

namespace gpac::zvm {

using Register = std::variant<int, double, bool, std::string>;

class Loader {
public:
    explicit Loader(gpac::backend::BCModule module)
        : module_(std::move(module)) {}

    void verify_or_throw() const {
        if (module_.functions.empty()) {
            throw std::runtime_error("ZVM loader rejected empty module");
        }
    }

    void debug_dump() const {
        std::cout << "ZVM module: " << module_.name << "\n";
        for (const auto& fn : module_.functions) {
            std::cout << " function " << fn.name << " instructions=" << fn.code.size() << "\n";
        }
    }

private:
    gpac::backend::BCModule module_;
};

} // namespace gpac::zvm
