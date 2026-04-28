// gpac_assemblypp_runtime.cpp
// GPAC Virtual Runtime — Assembly++ Intrinsic Bytecode Engine
// Build: g++ -std=c++20 -O3 -pthread gpac_assemblypp_runtime.cpp -o gpac_vm

#include <atomic>
#include <cstdint>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace gpac::assemblypp {

enum class TypeTag { Void, Int, Float, Bool, String, Array, Object, Pointer };
struct Value;
using ArrayValue = std::vector<Value>;
using ObjectValue = std::unordered_map<std::string, Value>;
struct PointerValue { std::string region; std::size_t slot{}; };

struct Value {
    TypeTag type = TypeTag::Void;
    std::variant<std::monostate, std::int64_t, double, bool, std::string, ArrayValue, ObjectValue, PointerValue> data;
    static Value Void(){ return {}; }
    static Value Int(std::int64_t v){ Value x; x.type=TypeTag::Int; x.data=v; return x; }
    static Value Float(double v){ Value x; x.type=TypeTag::Float; x.data=v; return x; }
    static Value Bool(bool v){ Value x; x.type=TypeTag::Bool; x.data=v; return x; }
    static Value String(std::string v){ Value x; x.type=TypeTag::String; x.data=std::move(v); return x; }
    static Value Array(ArrayValue v){ Value x; x.type=TypeTag::Array; x.data=std::move(v); return x; }
    static Value Object(ObjectValue v){ Value x; x.type=TypeTag::Object; x.data=std::move(v); return x; }
    static Value Pointer(std::string r, std::size_t s){ Value x; x.type=TypeTag::Pointer; x.data=PointerValue{std::move(r),s}; return x; }
    std::int64_t as_int() const { if(type==TypeTag::Int) return std::get<std::int64_t>(data); if(type==TypeTag::Bool) return std::get<bool>(data); throw std::runtime_error("not int"); }
    double as_float() const { if(type==TypeTag::Float) return std::get<double>(data); if(type==TypeTag::Int) return (double)std::get<std::int64_t>(data); throw std::runtime_error("not numeric"); }
    bool as_bool() const { if(type==TypeTag::Bool) return std::get<bool>(data); if(type==TypeTag::Int) return std::get<std::int64_t>(data)!=0; if(type==TypeTag::Float) return std::get<double>(data)!=0.0; if(type==TypeTag::String) return !std::get<std::string>(data).empty(); return false; }
    std::string to_string() const {
        switch(type){
            case TypeTag::Void: return "void";
            case TypeTag::Int: return std::to_string(std::get<std::int64_t>(data));
            case TypeTag::Float:{ std::ostringstream ss; ss<<std::get<double>(data); return ss.str(); }
            case TypeTag::Bool: return std::get<bool>(data)?"true":"false";
            case TypeTag::String: return std::get<std::string>(data);
            case TypeTag::Array:{ const auto& a=std::get<ArrayValue>(data); std::string s="["; for(size_t i=0;i<a.size();++i){ if(i)s+=", "; s+=a[i].to_string(); } return s+"]"; }
            case TypeTag::Object: return "{object}";
            case TypeTag::Pointer:{ const auto& p=std::get<PointerValue>(data); return "&"+p.region+"["+std::to_string(p.slot)+"]"; }
        } return "<invalid>";
    }
};

enum class Op { NOP, CONST_INT, CONST_FLOAT, CONST_BOOL, CONST_STRING, MOV, ADD, SUB, MUL, DIV, MOD, NEG, EQ, NEQ, LT, LTE, GT, GTE, AND, OR, NOT, ARRAY_NEW, ARRAY_PUSH, ARRAY_GET, ARRAY_SET, OBJECT_NEW, FIELD_GET, FIELD_SET, REGION_NEW, REGION_STORE, REGION_LOAD, PTR, LABEL, JMP, JMP_TRUE, JMP_FALSE, CALL, RET, PRINT, INPUT, NATIVE_CALL, TRACK_CALL, HALT };

struct Inst {
    Op op=Op::NOP; int dst=-1,a=-1,b=-1,c=-1; std::int64_t i64=0; double f64=0; bool boolean=false; std::string text; std::vector<int> regs;
    static Inst ConstInt(int d,std::int64_t v){Inst x; x.op=Op::CONST_INT; x.dst=d; x.i64=v; return x;}
    static Inst ConstFloat(int d,double v){Inst x; x.op=Op::CONST_FLOAT; x.dst=d; x.f64=v; return x;}
    static Inst ConstBool(int d,bool v){Inst x; x.op=Op::CONST_BOOL; x.dst=d; x.boolean=v; return x;}
    static Inst ConstString(int d,std::string v){Inst x; x.op=Op::CONST_STRING; x.dst=d; x.text=std::move(v); return x;}
    static Inst Mov(int d,int s){Inst x; x.op=Op::MOV; x.dst=d; x.a=s; return x;}
    static Inst Bin(Op op,int d,int a,int b){Inst x; x.op=op; x.dst=d; x.a=a; x.b=b; return x;}
    static Inst Un(Op op,int d,int a){Inst x; x.op=op; x.dst=d; x.a=a; return x;}
    static Inst Label(std::string n){Inst x; x.op=Op::LABEL; x.text=std::move(n); return x;}
    static Inst Jmp(std::string n){Inst x; x.op=Op::JMP; x.text=std::move(n); return x;}
    static Inst JmpTrue(int r,std::string n){Inst x; x.op=Op::JMP_TRUE; x.a=r; x.text=std::move(n); return x;}
    static Inst JmpFalse(int r,std::string n){Inst x; x.op=Op::JMP_FALSE; x.a=r; x.text=std::move(n); return x;}
    static Inst Call(std::string fn,std::vector<int> args={},int d=-1){Inst x; x.op=Op::CALL; x.text=std::move(fn); x.regs=std::move(args); x.dst=d; return x;}
    static Inst NativeCall(std::string fn,std::vector<int> args={},int d=-1){Inst x; x.op=Op::NATIVE_CALL; x.text=std::move(fn); x.regs=std::move(args); x.dst=d; return x;}
    static Inst TrackCall(std::string fn,std::vector<int> args={}){Inst x; x.op=Op::TRACK_CALL; x.text=std::move(fn); x.regs=std::move(args); return x;}
    static Inst Ret(int s=-1){Inst x; x.op=Op::RET; x.a=s; return x;}
    static Inst Print(int s){Inst x; x.op=Op::PRINT; x.a=s; return x;}
    static Inst Input(int d){Inst x; x.op=Op::INPUT; x.dst=d; return x;}
    static Inst Halt(){Inst x; x.op=Op::HALT; return x;}
};

struct Function { std::string name; int register_count=64; std::vector<Inst> code; std::unordered_map<std::string,std::size_t> labels; };
struct Module { std::string name; std::unordered_map<std::string,Function> functions; };

class MemoryRegion {
    std::string name; mutable std::mutex mu; std::vector<Value> slots;
public:
    explicit MemoryRegion(std::string n):name(std::move(n)){}
    std::size_t store(Value v){ std::lock_guard<std::mutex> l(mu); slots.push_back(std::move(v)); return slots.size()-1; }
    Value load(std::size_t s) const { std::lock_guard<std::mutex> l(mu); if(s>=slots.size()) throw std::runtime_error("memory load OOB: "+name); return slots[s]; }
    void write(std::size_t s,Value v){ std::lock_guard<std::mutex> l(mu); if(s>=slots.size()) throw std::runtime_error("memory write OOB: "+name); slots[s]=std::move(v); }
};

class Runtime {
    std::mutex io_mu;
    std::unordered_map<std::string,std::function<Value(const std::vector<Value>&)>> natives;
    std::unordered_map<std::string,std::unique_ptr<MemoryRegion>> regions;
public:
    Runtime(){
        regions.emplace("CoreHeap",std::make_unique<MemoryRegion>("CoreHeap"));
        regions.emplace("StackBurst",std::make_unique<MemoryRegion>("StackBurst"));
        regions.emplace("LocalArena",std::make_unique<MemoryRegion>("LocalArena"));
        regions.emplace("Scratch",std::make_unique<MemoryRegion>("Scratch"));
        register_native("native_add",[](const auto& a){ if(a.size()!=2) throw std::runtime_error("native_add needs 2 args"); return Value::Int(a[0].as_int()+a[1].as_int()); });
        register_native("native_clock_tick",[](const auto&){ static std::atomic<std::int64_t> t{0}; return Value::Int(++t); });
    }
    void print(const Value& v){ std::lock_guard<std::mutex> l(io_mu); std::cout<<v.to_string()<<'\n'; }
    Value input(){ std::lock_guard<std::mutex> l(io_mu); std::string s; std::getline(std::cin,s); return Value::String(s); }
    void register_native(const std::string& n, std::function<Value(const std::vector<Value>&)> f){ natives[n]=std::move(f); }
    Value native_call(const std::string& n,const std::vector<Value>& args){ auto it=natives.find(n); if(it==natives.end()) throw std::runtime_error("unknown native: "+n); return it->second(args); }
    MemoryRegion& region(const std::string& n){ auto it=regions.find(n); if(it==regions.end()) it=regions.emplace(n,std::make_unique<MemoryRegion>(n)).first; return *it->second; }
};

class Verifier {
public:
    static void verify(Module& m){
        if(m.name.empty()) throw std::runtime_error("module name empty");
        if(!m.functions.contains("main")) throw std::runtime_error("missing main");
        for(auto& [_,f]:m.functions){
            f.labels.clear();
            for(size_t i=0;i<f.code.size();++i) if(f.code[i].op==Op::LABEL) f.labels[f.code[i].text]=i;
            for(auto& ins:f.code){
                auto ck=[&](int r){ if(r>=f.register_count) throw std::runtime_error("register out of range in "+f.name); };
                ck(ins.dst); ck(ins.a); ck(ins.b); ck(ins.c); for(int r:ins.regs) ck(r);
                if((ins.op==Op::JMP||ins.op==Op::JMP_TRUE||ins.op==Op::JMP_FALSE)&&!f.labels.contains(ins.text)) throw std::runtime_error("unknown label "+ins.text);
            }
            if(f.code.empty()) throw std::runtime_error("empty function: "+f.name);
        }
    }
};

class VM {
    Module module;
    static std::size_t label_ip(const Function& f,const std::string& n){ auto it=f.labels.find(n); if(it==f.labels.end()) throw std::runtime_error("bad label: "+n); return it->second+1; }
    static Value add(const Value& a,const Value& b){ if(a.type==TypeTag::String||b.type==TypeTag::String) return Value::String(a.to_string()+b.to_string()); if(a.type==TypeTag::Float||b.type==TypeTag::Float) return Value::Float(a.as_float()+b.as_float()); return Value::Int(a.as_int()+b.as_int()); }
    static Value num(const Value& a,const Value& b,char op){ if(a.type==TypeTag::Float||b.type==TypeTag::Float){ double x=a.as_float(),y=b.as_float(); if(op=='-') return Value::Float(x-y); if(op=='*') return Value::Float(x*y); if(op=='/'){ if(y==0) throw std::runtime_error("divide by zero"); return Value::Float(x/y);} } auto x=a.as_int(), y=b.as_int(); if(op=='-') return Value::Int(x-y); if(op=='*') return Value::Int(x*y); if(op=='/'){ if(y==0) throw std::runtime_error("divide by zero"); return Value::Int(x/y);} throw std::runtime_error("bad numeric op"); }
    static bool cmp(const Value& a,const Value& b,Op op){ double x=a.as_float(),y=b.as_float(); if(op==Op::LT)return x<y; if(op==Op::LTE)return x<=y; if(op==Op::GT)return x>y; if(op==Op::GTE)return x>=y; return false; }
public:
    explicit VM(Module m):module(std::move(m)){ Verifier::verify(module); }
    Value run(Runtime& rt){ return call(rt,"main",{}); }
    Value call(Runtime& rt,const std::string& name,const std::vector<Value>& args){
        auto it=module.functions.find(name); if(it==module.functions.end()) throw std::runtime_error("unknown function: "+name);
        const Function& f=it->second; std::vector<Value> r(f.register_count); for(size_t i=0;i<args.size()&&i<r.size();++i) r[i]=args[i]; size_t ip=0;
        while(ip<f.code.size()){
            const auto& in=f.code[ip];
            switch(in.op){
                case Op::NOP: case Op::LABEL: ++ip; break;
                case Op::CONST_INT: r[in.dst]=Value::Int(in.i64); ++ip; break;
                case Op::CONST_FLOAT: r[in.dst]=Value::Float(in.f64); ++ip; break;
                case Op::CONST_BOOL: r[in.dst]=Value::Bool(in.boolean); ++ip; break;
                case Op::CONST_STRING: r[in.dst]=Value::String(in.text); ++ip; break;
                case Op::MOV: r[in.dst]=r[in.a]; ++ip; break;
                case Op::ADD: r[in.dst]=add(r[in.a],r[in.b]); ++ip; break;
                case Op::SUB: r[in.dst]=num(r[in.a],r[in.b],'-'); ++ip; break;
                case Op::MUL: r[in.dst]=num(r[in.a],r[in.b],'*'); ++ip; break;
                case Op::DIV: r[in.dst]=num(r[in.a],r[in.b],'/'); ++ip; break;
                case Op::MOD: r[in.dst]=Value::Int(r[in.a].as_int()%r[in.b].as_int()); ++ip; break;
                case Op::NEG: r[in.dst]=(r[in.a].type==TypeTag::Float)?Value::Float(-r[in.a].as_float()):Value::Int(-r[in.a].as_int()); ++ip; break;
                case Op::EQ: r[in.dst]=Value::Bool(r[in.a].to_string()==r[in.b].to_string()); ++ip; break;
                case Op::NEQ: r[in.dst]=Value::Bool(r[in.a].to_string()!=r[in.b].to_string()); ++ip; break;
                case Op::LT: case Op::LTE: case Op::GT: case Op::GTE: r[in.dst]=Value::Bool(cmp(r[in.a],r[in.b],in.op)); ++ip; break;
                case Op::AND: r[in.dst]=Value::Bool(r[in.a].as_bool()&&r[in.b].as_bool()); ++ip; break;
                case Op::OR: r[in.dst]=Value::Bool(r[in.a].as_bool()||r[in.b].as_bool()); ++ip; break;
                case Op::NOT: r[in.dst]=Value::Bool(!r[in.a].as_bool()); ++ip; break;
                case Op::ARRAY_NEW: r[in.dst]=Value::Array({}); ++ip; break;
                case Op::ARRAY_PUSH:{ auto a=std::get<ArrayValue>(r[in.dst].data); a.push_back(r[in.a]); r[in.dst]=Value::Array(std::move(a)); ++ip; break; }
                case Op::ARRAY_GET:{ auto& a=std::get<ArrayValue>(r[in.a].data); size_t idx=(size_t)r[in.b].as_int(); if(idx>=a.size()) throw std::runtime_error("array OOB"); r[in.dst]=a[idx]; ++ip; break; }
                case Op::ARRAY_SET:{ auto a=std::get<ArrayValue>(r[in.dst].data); size_t idx=(size_t)r[in.a].as_int(); if(idx>=a.size()) throw std::runtime_error("array OOB"); a[idx]=r[in.b]; r[in.dst]=Value::Array(std::move(a)); ++ip; break; }
                case Op::OBJECT_NEW: r[in.dst]=Value::Object({}); ++ip; break;
                case Op::FIELD_SET:{ auto o=std::get<ObjectValue>(r[in.dst].data); o[in.text]=r[in.a]; r[in.dst]=Value::Object(std::move(o)); ++ip; break; }
                case Op::FIELD_GET:{ const auto& o=std::get<ObjectValue>(r[in.a].data); auto fit=o.find(in.text); if(fit==o.end()) throw std::runtime_error("missing field: "+in.text); r[in.dst]=fit->second; ++ip; break; }
                case Op::REGION_NEW: rt.region(in.text); ++ip; break;
                case Op::REGION_STORE:{ size_t slot=rt.region(in.text).store(r[in.a]); r[in.dst]=Value::Pointer(in.text,slot); ++ip; break; }
                case Op::REGION_LOAD:{ auto p=std::get<PointerValue>(r[in.a].data); r[in.dst]=rt.region(p.region).load(p.slot); ++ip; break; }
                case Op::PTR:{ size_t slot=rt.region(in.text).store(r[in.a]); r[in.dst]=Value::Pointer(in.text,slot); ++ip; break; }
                case Op::JMP: ip=label_ip(f,in.text); break;
                case Op::JMP_TRUE: ip=r[in.a].as_bool()?label_ip(f,in.text):ip+1; break;
                case Op::JMP_FALSE: ip=!r[in.a].as_bool()?label_ip(f,in.text):ip+1; break;
                case Op::CALL:{ std::vector<Value> a; for(int x:in.regs)a.push_back(r[x]); Value ret=call(rt,in.text,a); if(in.dst>=0) r[in.dst]=ret; ++ip; break; }
                case Op::NATIVE_CALL:{ std::vector<Value> a; for(int x:in.regs)a.push_back(r[x]); Value ret=rt.native_call(in.text,a); if(in.dst>=0) r[in.dst]=ret; ++ip; break; }
                case Op::TRACK_CALL:{ std::vector<Value> a; for(int x:in.regs)a.push_back(r[x]); auto fn=in.text; std::async(std::launch::async,[this,&rt,fn,a]{ return call(rt,fn,a); }).get(); ++ip; break; }
                case Op::PRINT: rt.print(r[in.a]); ++ip; break;
                case Op::INPUT: r[in.dst]=rt.input(); ++ip; break;
                case Op::RET: return in.a>=0?r[in.a]:Value::Void();
                case Op::HALT: return Value::Void();
            }
        }
        return Value::Void();
    }
};

Module build_demo_module(){
    Module m; m.name="GPAC_AssemblyPP_Demo";
    Function add{"add",8,{ Inst::Bin(Op::ADD,2,0,1), Inst::Ret(2) }}; m.functions[add.name]=add;
    Function worker{"worker",8,{ Inst::ConstString(0,"Running track worker"), Inst::Print(0), Inst::Ret() }}; m.functions[worker.name]=worker;
    Function main; main.name="main"; main.register_count=32; main.code={
        Inst::ConstString(0,"Hello from GPAC Assembly++ VM"), Inst::Print(0),
        Inst::ConstInt(1,10), Inst::ConstInt(2,32), Inst::Call("add",{1,2},3), Inst::Print(3),
        Inst::ConstInt(4,5), Inst::ConstInt(5,7), Inst::NativeCall("native_add",{4,5},6), Inst::Print(6),
        Inst::TrackCall("worker",{}),
        Inst::ConstString(7,"Stored in LocalArena"), []{Inst x; x.op=Op::REGION_STORE; x.text="LocalArena"; x.a=7; x.dst=8; return x;}(), []{Inst x; x.op=Op::REGION_LOAD; x.a=8; x.dst=9; return x;}(), Inst::Print(9),
        []{Inst x; x.op=Op::OBJECT_NEW; x.dst=10; return x;}(), Inst::ConstString(11,"GPAC Object"), []{Inst x; x.op=Op::FIELD_SET; x.dst=10; x.a=11; x.text="name"; return x;}(), []{Inst x; x.op=Op::FIELD_GET; x.dst=12; x.a=10; x.text="name"; return x;}(), Inst::Print(12),
        []{Inst x; x.op=Op::ARRAY_NEW; x.dst=13; return x;}(), Inst::ConstInt(14,100), []{Inst x; x.op=Op::ARRAY_PUSH; x.dst=13; x.a=14; return x;}(), Inst::ConstInt(15,0), []{Inst x; x.op=Op::ARRAY_GET; x.dst=16; x.a=13; x.b=15; return x;}(), Inst::Print(16),
        Inst::ConstInt(17,0), Inst::ConstInt(18,3), Inst::ConstInt(19,1), Inst::Label("loop_start"), Inst::Bin(Op::LT,20,17,18), Inst::JmpFalse(20,"loop_end"), Inst::Print(17), Inst::Bin(Op::ADD,17,17,19), Inst::Jmp("loop_start"), Inst::Label("loop_end"),
        Inst::ConstString(21,"GPAC VM complete."), Inst::Print(21), Inst::Halt()
    }; m.functions[main.name]=main; return m;
}

} // namespace gpac::assemblypp

int main(){
    try { using namespace gpac::assemblypp; Runtime rt; VM vm(build_demo_module()); vm.run(rt); return 0; }
    catch(const std::exception& e){ std::cerr<<"[GPAC VM ERROR] "<<e.what()<<'\n'; return 1; }
}
