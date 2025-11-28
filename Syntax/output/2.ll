; ModuleID = 'sysy2022_complier'
source_filename = "./input/2.sy"

declare i32 @getinit()
declare i32 @getch()
declare i32 @getarray(i32*)
declare void @putinit(i32)
declare void @putch(i32)
declare void @putarray(i32, i32*)
declare void @starttime()
declare void @stoptime()

define void @main() {
main_ENTRY:
    %op1 = alloca i32
    store i32 10, i32* %op1
    %op2 = alloca i32
    store i32 2, i32* %op2
    %op3 = alloca float
    store float 0.500000, float* %op3
    %op4 = alloca float
    store float 3.140000, float* %op4
    %op5 = alloca i32
    store i32 0, i32* %op5
    %op6 = alloca i32
    store i32 1, i32* %op6
    %op7 = load i32, i32* %op5
    %op8 = load i32, i32* %op1
    %op9 = add i32 %op7, %op8
    %op10 = load i32, i32* %op2
    %op11 = mul i32 %op10, 2
    %op12 = sdiv i32 %op11, 1
    %op13 = srem i32 %op12, 2
    %op14 = sub i32 %op9, %op13
    store i32 %op14, i32* %op5
    %op15 = load i32, i32* %op5
    %op16 = load i32, i32* %op6
    %op17 = icmp sge i32 %op15, %op16
    %op18 = load float, float* %op3
    %op19 = fcmp one float %op18, 0.000000
    %op20 = zext i1 %op17 to i32
    %op21 = zext i1 %op19 to i32
    %op22 = mul i32 %op20, %op21
    %op23 = icmp ne i32 %op22, 0
    %op24 = load i32, i32* %op1
    %op25 = load i32, i32* %op2
    %op26 = icmp slt i32 %op24, %op25
    %op27 = zext i1 %op23 to i32
    %op28 = zext i1 %op26 to i32
    %op29 = add i32 %op27, %op28
    %op30 = icmp ne i32 %op29, 0
    br i1 %op30, label %if_then, label %if_else
if_then:
    %op32 = load i32, i32* %op6
    %op33 = add i32 %op32, 1
    store i32 %op33, i32* %op6
    br label %if_merge
if_else:
    %op35 = load i32, i32* %op6
    %op36 = sub i32 %op35, 1
    store i32 %op36, i32* %op6
    br label %if_merge
if_merge:
    %op38 = load i32, i32* %op6
    %op39 = add i32 %op38, 1
    store i32 %op39, i32* %op6
    %op40 = load i32, i32* %op6
    %op41 = sub i32 %op40, 1
    store i32 %op41, i32* %op6
    %op42 = load i32, i32* %op5
    %op43 = load i32, i32* %op6
    %op44 = icmp sgt i32 %op42, %op43
    %op45 = zext i1 %op44 to i32
    store i32 %op45, i32* %op5
    %op46 = load i32, i32* %op5
    %op47 = load i32, i32* %op6
    %op48 = icmp slt i32 %op46, %op47
    %op49 = zext i1 %op48 to i32
    store i32 %op49, i32* %op5
    %op50 = load i32, i32* %op5
    %op51 = load i32, i32* %op6
    %op52 = icmp eq i32 %op50, %op51
    %op53 = zext i1 %op52 to i32
    store i32 %op53, i32* %op5
    %op54 = load i32, i32* %op5
    %op55 = load i32, i32* %op6
    %op56 = icmp sle i32 %op54, %op55
    %op57 = zext i1 %op56 to i32
    store i32 %op57, i32* %op5
    %op58 = load i32, i32* %op5
    %op59 = load i32, i32* %op6
    %op60 = icmp sge i32 %op58, %op59
    %op61 = zext i1 %op60 to i32
    store i32 %op61, i32* %op5
    ret void
}
