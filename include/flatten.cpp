#include "flatten.hpp"
#include "SymbolTable.hpp"
#include <stdexcept>
#include <vector>

using namespace std;

// 计算每个维度的块大小，例如对于 dims = [2, 3, 4]，返回 [24, 12, 4】
static vector<int> build_blocks(const vector<int>& dims) {
    int n = (int)dims.size();
    vector<int> blocks(n);
    int prod = 1;
    for (int i = n - 1; i >= 0; --i) {
        prod *= dims[i];
        blocks[i] = prod;
    }
    return blocks;
}

//决定{} 应该对应哪一维
static int find_aligned_dim(int cur_dim, int used, const vector<int>& blocks) {
    for (int next = cur_dim + 1; next < (int)blocks.size(); ++next) {
        if (used % blocks[next] == 0) {
            return next;
        }
    }
    return -1;
}

//填充一维数组，递归
static void flatten_impl(
    InitValAST* init,
    int dim,
    int& pos,
    const vector<int>& dims,
    const vector<int>& blocks,
    vector<int>& data,
    SymbolTable& sym
) {
    int n = (int)dims.size();

    // 到最底层（标量）
    if (dim == n) {
        if (init->is_list) {
            throw runtime_error("brace initializer for scalar");
        }
        if (pos >= (int)data.size()) {
            throw runtime_error("too many initializers");
        }
        data[pos++] = init->exp->evalConst(sym);
        return;
    }

    // 当前是一个单表达式
    if (!init->is_list) {
        if (pos >= (int)data.size()) {
            throw runtime_error("too many initializers");
        }
        data[pos++] = init->exp->evalConst(sym);
        return;
    }

    int begin = pos;
    int block_size = blocks[dim];
    int end = begin + block_size;

    for (auto& elem_ptr : init->init_list) {
        auto* elem = dynamic_cast<InitValAST*>(elem_ptr.get());
        if (!elem) {
            throw runtime_error("invalid InitValAST node");
        }

        if (pos >= end) {
            throw runtime_error("too many initializers");
        }

        if (!elem->is_list) {
            data[pos++] = elem->exp->evalConst(sym);
        } else {
            int used = pos - begin;
            int next_dim = find_aligned_dim(dim, used, blocks);
            if (next_dim == -1) {
                throw runtime_error("initializer not aligned");
            }
            flatten_impl(elem, next_dim, pos, dims, blocks, data, sym);
        }
    }

    // 剩余部分补 0
    pos = end;
}

//总入口函数
vector<int> flatten_init(
    InitValAST* init,
    const vector<int>& dims,
    SymbolTable& sym
) {
    if (!init) {
        throw runtime_error("null initializer");
    }
    if (dims.empty()) {
        throw runtime_error("array dims should not be empty");
    }

    vector<int> blocks = build_blocks(dims);
    vector<int> data(blocks[0], 0);

    int pos = 0;
    flatten_impl(init, 0, pos, dims, blocks, data, sym);

    return data;
}

static void flatten_const_impl(
    ConstInitValAST* init,
    int dim,
    int& pos,
    const vector<int>& dims,
    const vector<int>& blocks,
    vector<int>& data,
    SymbolTable& sym
) {
    int n = (int)dims.size();

    if (dim == n) {
        if (init->is_list) {
            throw runtime_error("brace initializer for scalar");
        }
        if (pos >= (int)data.size()) {
            throw runtime_error("too many initializers");
        }
        data[pos++] = init->const_exp->evalConst(sym);
        return;
    }

    if (!init->is_list) {
        if (pos >= (int)data.size()) {
            throw runtime_error("too many initializers");
        }
        data[pos++] = init->const_exp->evalConst(sym);
        return;
    }

    int begin = pos;
    int end = begin + blocks[dim];

    for (auto& elem_ptr : init->init_list) {
        auto* elem = dynamic_cast<ConstInitValAST*>(elem_ptr.get());
        if (!elem) {
            throw runtime_error("invalid ConstInitValAST node");
        }

        if (pos >= end) {
            throw runtime_error("too many initializers");
        }

        if (!elem->is_list) {
            data[pos++] = elem->const_exp->evalConst(sym);
        } else {
            int used = pos - begin;
            int next_dim = find_aligned_dim(dim, used, blocks);
            if (next_dim == -1) {
                throw runtime_error("initializer not aligned");
            }
            flatten_const_impl(elem, next_dim, pos, dims, blocks, data, sym);
        }
    }

    pos = end;
}

vector<int> flatten_const_init(
    ConstInitValAST* init, 
    const vector<int>& dims, 
    SymbolTable& sym
) {
    if (!init) {
        throw runtime_error("null initializer");
    }
    if (dims.empty()) {
        throw runtime_error("array dims should not be empty");
    }

    vector<int> blocks = build_blocks(dims);
    vector<int> data(blocks[0], 0);

    int pos = 0;
    flatten_const_impl(init, 0, pos, dims, blocks, data, sym);

    return data;
}