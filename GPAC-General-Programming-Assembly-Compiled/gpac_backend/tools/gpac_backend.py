#!/usr/bin/env python3
from __future__ import annotations
import argparse
import json
import os
import re
import shutil
import subprocess
from pathlib import Path
from dataclasses import dataclass, field
from typing import List, Dict, Optional

@dataclass
class Inst:
    op: str
    dst: str = ""
    args: List[str] = field(default_factory=list)
    literal: str = ""

@dataclass
class Function:
    name: str
    ret: str
    insts: List[Inst] = field(default_factory=list)

@dataclass
class Module:
    name: str
    funcs: List[Function] = field(default_factory=list)

def parse_gpir(text: str) -> Module:
    lines = []
    for raw in text.splitlines():
        raw = re.sub(r"--.*$", "", raw).strip()
        if raw:
            lines.append(raw)
    if not lines or not lines[0].startswith("module "):
        raise SyntaxError("GPIR must begin with: module <name>")

    mod = Module(lines[0].split()[1])
    i = 1

    while i < len(lines):
        line = lines[i]
        m = re.match(r"func\s+(\w+)\s*->\s*(\w+)", line)
        if not m:
            raise SyntaxError(f"expected function declaration, got: {line}")
        fn = Function(m.group(1), m.group(2))
        i += 1

        if i >= len(lines) or not lines[i].startswith("block "):
            raise SyntaxError("function must begin with a block")
        i += 1

        while i < len(lines):
            cur = lines[i]
            if cur == "end":
                i += 1
                break

            m_const = re.match(r"(%\w+)\s*=\s*const\.string\s+\"(.*)\"", cur)
            if m_const:
                fn.insts.append(Inst("CONST_STR", dst=m_const.group(1), literal=m_const.group(2)))
                i += 1
                continue

            m_int = re.match(r"(%\w+)\s*=\s*const\.int\s+(-?\d+)", cur)
            if m_int:
                fn.insts.append(Inst("CONST_INT", dst=m_int.group(1), literal=m_int.group(2)))
                i += 1
                continue

            m_print = re.match(r"print\s+(%\w+)", cur)
            if m_print:
                fn.insts.append(Inst("PRINT", args=[m_print.group(1)]))
                i += 1
                continue

            if cur == "ret":
                fn.insts.append(Inst("RET"))
                i += 1
                continue

            raise SyntaxError(f"unknown GPIR instruction: {cur}")

        if i < len(lines) and lines[i] == "end":
            i += 1

        mod.funcs.append(fn)

    return mod

def assemble(mod: Module) -> dict:
    string_pool = []
    funcs = []
    for fn in mod.funcs:
        regs = {}
        code = []

        def reg(name):
            if name not in regs:
                regs[name] = len(regs)
            return regs[name]

        for inst in fn.insts:
            if inst.op == "CONST_STR":
                idx = len(string_pool)
                string_pool.append(inst.literal)
                code.append({"op": "CONST_STR", "dst": reg(inst.dst), "pool": idx})
            elif inst.op == "CONST_INT":
                code.append({"op": "CONST_INT", "dst": reg(inst.dst), "value": int(inst.literal)})
            elif inst.op == "PRINT":
                code.append({"op": "PRINT", "src": reg(inst.args[0])})
            elif inst.op == "RET":
                code.append({"op": "RET"})
        if not code or code[-1]["op"] != "RET":
            code.append({"op": "RET"})
        funcs.append({"name": fn.name, "ret": fn.ret, "registers": len(regs), "code": code})
    return {"name": mod.name, "strings": string_pool, "functions": funcs}

def verify(bc: dict) -> None:
    if not bc.get("name"):
        raise ValueError("bytecode module missing name")
    if not bc.get("functions"):
        raise ValueError("bytecode module has no functions")
    if not any(f["name"] == "main" for f in bc["functions"]):
        raise ValueError("bytecode module has no main function")

    for f in bc["functions"]:
        defined = set()
        for ins in f["code"]:
            op = ins["op"]
            if op in ("CONST_STR", "CONST_INT"):
                defined.add(ins["dst"])
                if op == "CONST_STR" and ins["pool"] >= len(bc["strings"]):
                    raise ValueError("invalid string pool reference")
            elif op == "PRINT":
                if ins["src"] not in defined:
                    raise ValueError("PRINT reads undefined register")
            elif op == "RET":
                pass
            else:
                raise ValueError(f"unknown opcode: {op}")

def emit_ll(bc: dict) -> str:
    out = []
    out.append("; GPAC backend emitted LLVM IR")
    out.append(f"; module: {bc['name']}")
    out.append("declare i32 @puts(ptr)")
    out.append("")
    for i, s in enumerate(bc["strings"]):
        esc = s.replace("\\", "\\5C").replace('"', "\\22") + "\\00"
        out.append(f'@.gpac.str.{i} = private unnamed_addr constant [{len(s)+1} x i8] c"{esc}"')
    out.append("")
    out.append("define i32 @main() {")
    out.append("entry:")
    main = next(f for f in bc["functions"] if f["name"] == "main")
    for ins in main["code"]:
        if ins["op"] == "CONST_STR":
            s = bc["strings"][ins["pool"]]
            out.append(f'  %r{ins["dst"]} = getelementptr [{len(s)+1} x i8], ptr @.gpac.str.{ins["pool"]}, i32 0, i32 0')
        elif ins["op"] == "PRINT":
            out.append(f'  call i32 @puts(ptr %r{ins["src"]})')
    out.append("  ret i32 0")
    out.append("}")
    return "\n".join(out)

def c_escape(s: str) -> str:
    return s.replace("\\", "\\\\").replace('"', '\\"')

def emit_cpp(bc: dict) -> str:
    out = []
    out.append("#include <iostream>")
    out.append("#include <string>")
    out.append("")
    out.append("struct GPAC_ZVM_Runtime {")
    out.append("    void print(const std::string& s) { std::cout << s << std::endl; }")
    out.append("    void print(int v) { std::cout << v << std::endl; }")
    out.append("};")
    out.append("")
    out.append("int main() {")
    out.append("    GPAC_ZVM_Runtime rt;")
    main = next(f for f in bc["functions"] if f["name"] == "main")
    for ins in main["code"]:
        if ins["op"] == "CONST_STR":
            out.append(f'    std::string r{ins["dst"]} = "{c_escape(bc["strings"][ins["pool"]])}";')
        elif ins["op"] == "CONST_INT":
            out.append(f'    int r{ins["dst"]} = {ins["value"]};')
        elif ins["op"] == "PRINT":
            out.append(f'    rt.print(r{ins["src"]});')
    out.append("    return 0;")
    out.append("}")
    return "\n".join(out)

def build_exe(cpp: Path, exe: Path):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        raise RuntimeError("No clang++ or g++ found; emitted backend C++ but could not build executable.")
    exe.parent.mkdir(parents=True, exist_ok=True)
    subprocess.check_call([compiler, "-std=c++20", "-O3", str(cpp), "-o", str(exe)])

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("source", help=".gpir file")
    ap.add_argument("--emit-all", action="store_true")
    ap.add_argument("--exe", default=None)
    args = ap.parse_args()

    src = Path(args.source)
    mod = parse_gpir(src.read_text(encoding="utf-8"))
    bc = assemble(mod)
    verify(bc)

    build = src.parent / "build"
    build.mkdir(exist_ok=True)

    bc_path = build / f"{src.stem}.gpbc.json"
    ll_path = build / f"{src.stem}.ll"
    cpp_path = build / f"{src.stem}.cpp"

    bc_path.write_text(json.dumps(bc, indent=2), encoding="utf-8")
    ll_path.write_text(emit_ll(bc), encoding="utf-8")
    cpp_path.write_text(emit_cpp(bc), encoding="utf-8")

    print(f"emitted: {bc_path}")
    print(f"emitted: {ll_path}")
    print(f"emitted: {cpp_path}")

    if args.exe:
        build_exe(cpp_path, Path(args.exe))
        print(f"built: {args.exe}")

if __name__ == "__main__":
    main()
