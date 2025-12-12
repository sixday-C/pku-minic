#pragma once
#include <unordered_map>
#include <string>
#include <stdexcept>
#include "ir.hpp"
struct SymbolInfo{
    enum Type{
        CONST,
        VAR
    }type;
    int const_value;
    Value* var_alloc;
    static SymbolInfo makeConst(int value){
        return SymbolInfo{CONST,value,nullptr};
    }
    static SymbolInfo makeVar(Value* alloc){
        return SymbolInfo{VAR,0,alloc};
    }
} ;

class SymbolTable{
    private:
    std::unordered_map<std::string,SymbolInfo> table;
    public:
    void insertConst(const std::string& name,int value){
        if(exists(name)){
            throw std::runtime_error("Variable "+name+" already defined in symbol table.");
        }
        table[name]=SymbolInfo::makeConst(value);
    }
    void insertVar(const std::string& name,Value* alloc){
        if(exists(name)){
            throw std::runtime_error("Variable "+name+" already defined in symbol table.");
        }
        table[name]=SymbolInfo::makeVar(alloc);
    }
    bool exists(const std::string& name) const{
        return table.find(name)!=table.end();
    }
    const SymbolInfo& lookup(const std::string& name) const {
        if (!exists(name)) {
            throw std::runtime_error("Variable " + name + " not defined");
        }
        return table.at(name);
    }
    int lookupConst(const std::string& name) const {
        const SymbolInfo& info = lookup(name);
        if (info.type != SymbolInfo::CONST) {
            throw std::runtime_error("Symbol " + name + " is not a constant");
        }
        return info.const_value;
    }
    Value* lookupVar(const std::string& name) const {
        const SymbolInfo& info = lookup(name);
        if (info.type != SymbolInfo::VAR) {
            throw std::runtime_error("Symbol " + name + " is not a variable");
        }
        return info.var_alloc;
    }
};