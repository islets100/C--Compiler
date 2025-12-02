; ModuleID = 'sysy2022_complier'
source_filename = "./input/8.sy"

declare i32 @getinit()
declare i32 @getch()
declare i32 @getarray(i32*)
declare void @putinit(i32)
declare void @putch(i32)
declare void @putarray(i32, i32*)
declare void @starttime()
declare void @stoptime()

define i32 @foo(i32 %arg0) {
foo_ENTRY:
    %op2 = alloca i32
    store i32 %arg0, i32* %op2
    %op3 = load i32, i32* %op2
    %op4 = mul i32 %op3, 2
    ret i32 %op4
}
define i32 @main() {
main_ENTRY:
    %op1 = alloca i32
    store i32 5, i32* %op1
    %op2 = alloca i32
    store i32 10, i32* %op2
    %op3 = alloca i32
    store i32 2, i32* %op3
    %op4 = alloca i32
    store i32 0, i32* %op4
    %op5 = load i32, i32* %op1
    %op6 = load i32, i32* %op2
    %op7 = load i32, i32* %op3
    %op8 = mul i32 %op6, %op7
    %op9 = load i32, i32* %op1
    %op10 = sub i32 %op9, 2
    %op11 = sdiv i32 %op8, %op10
    %op12 = add i32 %op5, %op11
    %op13 = load i32, i32* %op1
    %op14 = call i32 @foo(i32 %op13)
    %op15 = srem i32 %op14, 3
    %op16 = load i32, i32* %op2
    %op17 = mul i32 %op15, %op16
    %op18 = sub i32 %op12, %op17
    store i32 %op18, i32* %op4
    %op19 = alloca i32
    %op20 = load i32, i32* %op1
    %op21 = load i32, i32* %op2
    %op22 = icmp sgt i32 %op20, %op21
    %op23 = zext i1 %op22 to i32
    %op24 = icmp eq i32 %op23, 0
    %op25 = zext i1 %op24 to i32
    store i32 %op25, i32* %op19
    %op26 = load i32, i32* %op4
    ret i32 %op26
}
