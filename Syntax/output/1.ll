; ModuleID = 'sysy2022_complier'
source_filename = "./input/1.sy"

declare i32 @getinit()
declare i32 @getch()
declare i32 @getarray(i32*)
declare void @putinit(i32)
declare void @putch(i32)
declare void @putarray(i32, i32*)
declare void @starttime()
declare void @stoptime()

define i32 @main() {
main_ENTRY:
    %op1 = alloca i32
    store i32 1, i32* %op1
    %op2 = alloca i32
    store i32 2, i32* %op2
    %op3 = load i32, i32* %op1
    %op4 = load i32, i32* %op2
    %op5 = add i32 %op3, %op4
    %op6 = sub i32 %op5, 1
    store i32 %op6, i32* %op1
    %op7 = load i32, i32* %op1
    %op8 = icmp eq i32 %op7, 2
    br i1 %op8, label %if_then, label %if_else
if_then:
    ret i32 0
if_else:
    %op11 = load i32, i32* %op2
    %op12 = mul i32 %op11, 2
    %op13 = sdiv i32 %op12, 1
    %op14 = srem i32 %op13, 2
    store i32 %op14, i32* %op2
    ret i32 1
}
