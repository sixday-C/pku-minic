#include "../include/rv_gen.hpp"
#include <map>
#include <cassert>

// 生成整个程序的 RISC-V 内存结构
std::unique_ptr<RVProg> RvGen::generate(const Program& p) {
  auto prog = std::make_unique<RVProg>();
  
  for (const auto& f : p.funcs) {
    auto rv_func = gen_function(*f);
    prog->funcs.push_back(std::move(rv_func));
  }
  
  return prog;
}

// 生成函数的 RISC-V 结构
std::unique_ptr<RVFunc> RvGen::gen_function(const Function& f) {
  auto rv_func = std::make_unique<RVFunc>();
  rv_func->name = f.name;
  
  // 重置寄存器分配状态
  reg_counter_ = 0;
  temp_to_reg_.clear();
  
  for (const auto& bb : f.blocks) {
    auto rv_block = gen_block(*bb);
    rv_func->blocks.push_back(std::move(rv_block));
  }
  
  return rv_func;
}

// 生成基本块的 RISC-V 结构
std::unique_ptr<RVBlock> RvGen::gen_block(const BasicBlock& bb) {
  auto rv_block = std::make_unique<RVBlock>();
  rv_block->label = bb.name;
  
  for (const auto& inst : bb.insts) {
    // 处理二元运算指令
    if (auto* binary = dynamic_cast<const Binary*>(inst.get())) {
      gen_binary(*binary, rv_block->insts);
    }
    // 处理返回指令
    else if (auto* ret = dynamic_cast<const Return*>(inst.get())) {
      gen_return(*ret, rv_block->insts);
    }
  }
  
  return rv_block;
}

// 处理二元运算指令
void RvGen::gen_binary(const Binary& binary, std::vector<std::unique_ptr<RVInst>>& insts) {
  std::string result_reg = get_reg_name(binary.name);
  
  if (binary.op == Binary::EQ) {
    // eq lhs, 0 -> li + xor + seqz 序列
    if (auto* lhs_int = dynamic_cast<const Integer*>(binary.lhs.get())) {
      // li result_reg, value
      auto li = std::make_unique<RVLi>();
      li->rd = RVReg{result_reg};
      li->imm = RVImm{lhs_int->value};
      insts.push_back(std::move(li));
      
      // xor result_reg, result_reg, x0
      auto xor_inst = std::make_unique<RVXor>();
      xor_inst->rd = RVReg{result_reg};
      xor_inst->rs1 = RVReg{result_reg};
      xor_inst->rs2 = RVReg{"x0"};
      insts.push_back(std::move(xor_inst));
      
      // seqz result_reg, result_reg
      auto seqz = std::make_unique<RVSeqz>();
      seqz->rd = RVReg{result_reg};
      seqz->rs = RVReg{result_reg};
      insts.push_back(std::move(seqz));
    }
    else if (auto* lhs_var = dynamic_cast<const Variable*>(binary.lhs.get())) {
      std::string lhs_reg = get_reg_name(lhs_var->name);
      
      // xor result_reg, lhs_reg, x0
      auto xor_inst = std::make_unique<RVXor>();
      xor_inst->rd = RVReg{result_reg};
      xor_inst->rs1 = RVReg{lhs_reg};
      xor_inst->rs2 = RVReg{"x0"};
      insts.push_back(std::move(xor_inst));
      
      // seqz result_reg, result_reg
      auto seqz = std::make_unique<RVSeqz>();
      seqz->rd = RVReg{result_reg};
      seqz->rs = RVReg{result_reg};
      insts.push_back(std::move(seqz));
    }
  }
  else if (binary.op == Binary::SUB) {
    // sub lhs, rhs
    if (auto* lhs_int = dynamic_cast<const Integer*>(binary.lhs.get())) {
      if (auto* rhs_int = dynamic_cast<const Integer*>(binary.rhs.get())) {
        // sub 0, rhs -> li + sub
        if (lhs_int->value == 0) {
          auto li = std::make_unique<RVLi>();
          li->rd = RVReg{result_reg};
          li->imm = RVImm{rhs_int->value};
          insts.push_back(std::move(li));
          
          auto sub = std::make_unique<RVSub>();
          sub->rd = RVReg{result_reg};
          sub->rs1 = RVReg{"x0"};
          sub->rs2 = RVReg{result_reg};
          insts.push_back(std::move(sub));
        }
      }
      else if (auto* rhs_var = dynamic_cast<const Variable*>(binary.rhs.get())) {
        // sub 0, rhs_var
        if (lhs_int->value == 0) {
          std::string rhs_reg = get_reg_name(rhs_var->name);
          auto sub = std::make_unique<RVSub>();
          sub->rd = RVReg{result_reg};
          sub->rs1 = RVReg{"x0"};
          sub->rs2 = RVReg{rhs_reg};
          insts.push_back(std::move(sub));
        }
      }
    }
  }
}

// 处理返回指令
void RvGen::gen_return(const Return& ret, std::vector<std::unique_ptr<RVInst>>& insts) {
  // 将返回值移动到 a0 寄存器
  if (ret.value) {
    if (auto* int_val = dynamic_cast<const Integer*>(ret.value.get())) {
      auto li = std::make_unique<RVLi>();
      li->rd = RVReg{"a0"};
      li->imm = RVImm{int_val->value};
      insts.push_back(std::move(li));
    }
    else if (auto* const_int = dynamic_cast<const ConstInt*>(ret.value.get())) {
      auto li = std::make_unique<RVLi>();
      li->rd = RVReg{"a0"};
      li->imm = RVImm{const_int->v};
      insts.push_back(std::move(li));
    }
  }
  else if (!ret.temp_name.empty()) {
    std::string temp_reg = get_reg_name(ret.temp_name);
    auto mv = std::make_unique<RVMv>();
    mv->rd = RVReg{"a0"};
    mv->rs = RVReg{temp_reg};
    insts.push_back(std::move(mv));
  }
  
  // 添加 ret 指令
  auto ret_inst = std::make_unique<RVRet>();
  insts.push_back(std::move(ret_inst));
}

// 获取临时变量对应的寄存器名
std::string RvGen::get_reg_name(const std::string& temp_var) {
  if (temp_to_reg_.find(temp_var) != temp_to_reg_.end()) {
    return temp_to_reg_[temp_var];
  }
  
  std::string reg_name = "t" + std::to_string(reg_counter_++);
  temp_to_reg_[temp_var] = reg_name;
  return reg_name;
}
