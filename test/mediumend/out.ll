declare i32 @getint()
declare i32 @getch()
declare float @getfloat()
declare void @putint(i32)
declare void @putch(i32)
declare void @putfloat(float)

define i32 @main() {
B1:
%0 = add i32 0 , 1
call void @putint(i32 %52)
ret i32 %0
}

