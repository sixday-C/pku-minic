#pragma once
#include <unordered_map>
#include <string>
#include <stdexcept>

class SymbolTable{
    private:
    std::unordered_map<std::string,int> table;
    public:
    void insert(const std::string& name,int value){
        table[name]=value;
    }
    bool exists(const std::string& name) const{
        return table.find(name)!=table.end();
    }
    int lookup(const std::string& name) const{
        if(!exists(name)){
            throw std::runtime_error("Variable "+name+" not found in symbol table.");
        }
        return table.at(name);
    }
};
