; ModuleID = 'sysy2022_complier'
source_filename = "./input/5.sy"

declare i32 @getinit()
declare i32 @getch()
declare i32 @getarray(i32*)
declare void @putinit(i32)
declare void @putch(i32)
declare void @putarray(i32, i32*)
declare void @starttime()
declare void @stoptime()

define i32 @fib(i32 %arg0) {
fib_ENTRY:
    %op2 = alloca i32
    store i32 %arg0, i32* %op2
    %op3 = load i32, i32* %op2
    %op4 = icmp sle i32 %op3, 1
    br i1 %op4, label %if_then0, label %if_merge0
if_then0:
    %op6 = load i32, i32* %op2
    ret i32 %op6
if_merge0:
    %op8 = load i32, i32* %op2
    %op9 = sub i32 %op8, 1
    %op10 = call i32 @fib(i32 %op9)
    %op11 = load i32, i32* %op2
    %op12 = sub i32 %op11, 2
    %op13 = call i32 @fib(i32 %op12)
    %op14 = add i32 %op10, %op13
    ret i32 %op14
}
define i32 @main() {
main_ENTRY:
    %op1 = alloca i32
    store i32 10, i32* %op1
    %op2 = alloca i32
    store i32 0, i32* %op2
    %op3 = load i32, i32* %op1
    %op4 = call i32 @fib(i32 %op3)
    store i32 %op4, i32* %op2
    %op5 = load i32, i32* %op2
    ret i32 %op5
}
