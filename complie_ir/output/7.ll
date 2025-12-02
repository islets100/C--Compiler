; ModuleID = 'sysy2022_complier'
source_filename = "./input/7.sy"

@PI = global float 3.141590
declare i32 @getinit()
declare i32 @getch()
declare i32 @getarray(i32*)
declare void @putinit(i32)
declare void @putch(i32)
declare void @putarray(i32, i32*)
declare void @starttime()
declare void @stoptime()

define i32 @gcd(i32 %arg0, i32 %arg1) {
gcd_ENTRY:
    %op3 = alloca i32
    store i32 %arg0, i32* %op3
    %op4 = alloca i32
    store i32 %arg1, i32* %op4
    %op5 = load i32, i32* %op4
    %op6 = icmp eq i32 %op5, 0
    br i1 %op6, label %if_then0, label %if_merge0
if_then0:
    %op8 = load i32, i32* %op3
    ret i32 %op8
if_merge0:
    %op10 = load i32, i32* %op4
    %op11 = load i32, i32* %op3
    %op12 = load i32, i32* %op4
    %op13 = srem i32 %op11, %op12
    %op14 = call i32 @gcd(i32 %op10, i32 %op13)
    ret i32 %op14
}
define i32 @main() {
main_ENTRY:
    %op1 = alloca i32
    store i32 24, i32* %op1
    %op2 = alloca i32
    store i32 18, i32* %op2
    %op3 = alloca i32
    %op4 = load i32, i32* %op1
    %op5 = load i32, i32* %op2
    %op6 = call i32 @gcd(i32 %op4, i32 %op5)
    store i32 %op6, i32* %op3
    %op7 = alloca float
    store float 5.000000, float* %op7
    %op8 = alloca float
    store float 0.000000, float* %op8
    %op9 = load i32, i32* %op3
    %op10 = icmp sgt i32 %op9, 1
    br i1 %op10, label %if_then1, label %if_merge1
if_then1:
    %op12 = load float, float* @PI
    %op13 = load float, float* %op7
    %op14 = fmul float %op12, %op13
    %op15 = load float, float* %op7
    %op16 = fmul float %op14, %op15
    store float %op16, float* %op8
    br label %if_merge1
if_merge1:
    ret i32 0
}
