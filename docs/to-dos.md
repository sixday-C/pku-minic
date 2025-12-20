
"Premature optimization is the root of all evil"
（过早优化是万恶之源）

int main() {
  int a = 1;        // 外层的 a
  {
    a = 2;          // 修改外层的 a
    int a = 3;      // 内层的新 a（遮蔽了外层的 a）
  }
  return a;         // 返回外层的 a（值为 2）
}
