#include "../include/gpac_backend/backend_api.hpp"
#include <unordered_map>

namespace gpac::backend {

static std::uint32_t reg_id(const std::string& name, std::unordered_map<std::string, std::uint32_t>& regs) {
    auto it = regs.find(name);
    if (it != regs.end()) return it->second;
    std::uint32_t id = static_cast<std::uint32_t>(regs.size());
    regs[name] = id;
    return id;
}

BCModule assemble_to_bytecode(const Module& module) {
    BCModule out;
    out.name = module.name;

    for (const auto& fn : module.functions) {
        BCFunction bf;
        bf.name = fn.name;
        std::unordered_map<std::string, std::uint32_t> regs;

        for (const auto& block : fn.blocks) {
            for (const auto& inst : block.instructions) {
                BCInst bi;

                switch (inst.op) {
                    case IROp::ConstInt:
                        bi.opcode = Opcode::ConstInt;
                        bi.dst = reg_id(inst.dst, regs);
                        bi.text = inst.literal;
                        break;

                    case IROp::ConstFloat:
                        bi.opcode = Opcode::ConstFloat;
                        bi.dst = reg_id(inst.dst, regs);
                        bi.text = inst.literal;
                        break;

                    case IROp::ConstString:
                        bi.opcode = Opcode::ConstString;
                        bi.dst = reg_id(inst.dst, regs);
                        out.string_pool.push_back(inst.literal);
                        bi.a = static_cast<std::uint32_t>(out.string_pool.size() - 1);
                        break;

                    case IROp::Add:
                        bi.opcode = Opcode::AddI;
                        bi.dst = reg_id(inst.dst, regs);
                        bi.a = reg_id(inst.args.at(0), regs);
                        bi.b = reg_id(inst.args.at(1), regs);
                        break;

                    case IROp::Print:
                        bi.opcode = Opcode::Print;
                        bi.a = reg_id(inst.args.at(0), regs);
                        break;

                    case IROp::Ret:
                        bi.opcode = Opcode::Ret;
                        if (!inst.args.empty()) bi.a = reg_id(inst.args.at(0), regs);
                        break;

                    default:
                        bi.opcode = Opcode::Nop;
                        break;
                }

                bf.code.push_back(bi);
            }
        }

        if (bf.code.empty() || bf.code.back().opcode != Opcode::Ret) {
            bf.code.push_back({Opcode::Ret, 0, 0, 0, ""});
        }

        out.functions.push_back(bf);
    }

    return out;
}

} // namespace gpac::backend
