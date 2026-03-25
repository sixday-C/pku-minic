#pragma once
#include <unordered_map>
#include <string>
#include <stdexcept>
#include "ir.hpp"
struct SymbolInfo{
    enum Type{
        CONST,
        VAR,
        FUNC
    }type;
    int const_value;
    Value* var_alloc;
    static SymbolInfo makeConst(int value){
        return SymbolInfo{CONST,value,nullptr,::Type::Void};
    }
    static SymbolInfo makeVar(Value* alloc){
        return SymbolInfo{VAR,0,alloc,::Type::Void};
    }
    ::Type func_ret_type;
    static SymbolInfo makeFunc(::Type ret_type){
        return SymbolInfo{FUNC,0,nullptr,ret_type};
    }
} ;

class SymbolTable{
    private:
    std::list<std::unordered_map<std::string,SymbolInfo>> scopes;
    std::unordered_map<std::string,int> nameCount;
    public:
    SymbolTable(){
        //全局作用域
        enterScope();
    }
    void enterScope(){
        scopes.push_back(std::unordered_map<std::string,SymbolInfo>{});
    }
    void exitScope(){
        if(scopes.empty()){
            throw std::runtime_error("No scope to exit.");
        }
        scopes.pop_back();
    }
    std::string makeUniqueName(const std::string& name){
        int count =nameCount[name]++;
        return "@"+name+"_"+std::to_string(count);
    }
    void insertConst(const std::string& name,int value){
        auto &current = scopes.back();
        if(current.find(name)!=current.end()){//找不到
            throw std::runtime_error("Constant "+name+" already defined in symbol table.");
        }
        current[name]=SymbolInfo::makeConst(value);
    }
    void insertVar(const std::string& name,Value* alloc){
        auto &current = scopes.back();
        if(current.find(name)!=current.end()){//找不到
            throw std::runtime_error("Variable "+name+" already defined in symbol table.");
        }
        current[name]=SymbolInfo::makeVar(alloc);
    }
    const SymbolInfo& lookup(const std::string& name) const {
        for(auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }
        throw std::runtime_error("Symbol " + name + " not found in symbol table.");
}
    int lookupConst(const std::string& name) const { //返回常量值，检查找到的是否是常量
        const SymbolInfo& info = lookup(name);
        if (info.type != SymbolInfo::CONST) {
            throw std::runtime_error("Symbol " + name + " is not a constant");
        }
        return info.const_value;
    }
    Value* lookupVar(const std::string& name) const { //返回变量的alloc指令，检查找到的是否是变量
        const SymbolInfo& info = lookup(name);
        if (info.type != SymbolInfo::VAR) {
            throw std::runtime_error("Symbol " + name + " is not a variable");
        }
        return info.var_alloc;
    }
    void insertFunc(const std::string& name,::Type ret_type){
        scopes.front()[name] = SymbolInfo::makeFunc(ret_type);
    }
    ::Type lookupFunc(const std::string& name) const {
        const SymbolInfo& info = lookup(name);
        if (info.type != SymbolInfo::FUNC) {
            throw std::runtime_error("Symbol " + name + " is not a function");
        }
        return info.func_ret_type;
    }
};