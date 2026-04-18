#pragma once
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <vector>
#include <list>
#include "ir.hpp"
struct SymbolInfo {
    enum Kind { CONST, VAR, FUNC } kind;


    Value* var_alloc = nullptr;//变量的起始位置

    ::Type func_ret_type = ::Type::Void; 
    std::vector<::Type> param_types;   
    
    bool is_array = false;           
    bool is_param_array = false;     // 标记是否为数组参数
    int array_size = 0;              
    int const_value = 0; 
    std::vector<int> array_dims;   // 新增：记录每一维长度  


    static SymbolInfo makeConst(int value) {
        SymbolInfo info;
        info.kind = CONST;
        info.const_value = value;
        return info;
    }

    static SymbolInfo makeConstArray(Value* alloc, int size, const std::vector<int>& dims) {
        SymbolInfo info;
        info.kind = CONST;
        info.is_array = true;
        info.array_size = size;
        info.array_dims = dims;
        info.var_alloc = alloc;
        return info;
    }

    static SymbolInfo makeVar(Value* alloc, bool is_arr = false, int size = 0,
                          const std::vector<int>& dims = {}, bool is_param = false) {
    SymbolInfo info;
    info.kind = VAR;
    info.var_alloc = alloc;
    info.is_array = is_arr;
    info.is_param_array = is_param;
    info.array_size = size;
    info.array_dims = dims;
    return info;
}
    static SymbolInfo makeFunc(::Type ret_type, std::vector<::Type> params = {}) {
        SymbolInfo info;
        info.kind = FUNC;
        info.func_ret_type = ret_type;
        info.param_types = std::move(params);
        return info;
    }
};

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

    //【数组】变量
    void insertVar(const std::string& name, Value* alloc, bool is_arr = false,
               int size = 0, const std::vector<int>& dims = {}, bool is_param = false) {
    auto &current = scopes.back();
    if(current.find(name) != current.end()) {
        throw std::runtime_error("Variable " + name + " already defined.");
    }
    current[name] = SymbolInfo::makeVar(alloc, is_arr, size, dims, is_param);
}

    //数组常量
    void insertConstArray(const std::string& name, Value* alloc, int size,
                      const std::vector<int>& dims) {
    auto &current = scopes.back();
    if(current.find(name) != current.end()) {
        throw std::runtime_error("Constant array " + name + " already defined.");
    }
    current[name] = SymbolInfo::makeConstArray(alloc, size, dims);
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

    int lookupConst(const std::string& name) const {
        const SymbolInfo& info = lookup(name);
        if (info.kind != SymbolInfo::CONST || info.is_array) {
            throw std::runtime_error("Symbol " + name + " is not a scalar constant");
        }
        return info.const_value;
    }
    
    Value* lookupVar(const std::string& name) const { //返回变量的alloc指令，检查找到的是否是变量
        const SymbolInfo& info = lookup(name);
        if (info.kind != SymbolInfo::VAR) {
            throw std::runtime_error("Symbol " + name + " is not a variable");
        }
        return info.var_alloc;
    }

    Value* lookupAddr(const std::string& name) const {
    const SymbolInfo& info = lookup(name);
    if (info.kind == SymbolInfo::FUNC) {
        throw std::runtime_error("Symbol " + name + " is a function, not an addressable entity.");
    }
    return info.var_alloc;
}
void insertFunc(const std::string& name, ::Type ret_type, std::vector<::Type> params = {}) {
    scopes.front()[name] = SymbolInfo::makeFunc(ret_type, std::move(params));
}
    ::Type lookupFunc(const std::string& name) const {
        const SymbolInfo& info = lookup(name);
        if (info.kind != SymbolInfo::FUNC) {
            throw std::runtime_error("Symbol " + name + " is not a function");
        }
        return info.func_ret_type;
    }
    
};