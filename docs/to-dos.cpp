
//"Premature optimization is the root of all evil"
//（过早优化是万恶之源）

//支持全局变量
int var;

const int one = 1;

int main() {
  return var + one;
}

/*

global @var = alloc i32, zeroinit  //全局变量 var，初始值为 0

fun @main(): i32 {
%entry:
  %0 = load @var
  %1 = add %0, 1
  ret %1
}

  */