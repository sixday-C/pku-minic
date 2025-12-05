
fun @main(): i32 {
%entry:
  %0 = eq 6, 0
  %1 = sub 0, %0
  %2 = sub 0, %1
  ret %2
}

"Premature optimization is the root of all evil"
（过早优化是万恶之源）