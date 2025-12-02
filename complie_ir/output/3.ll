; ModuleID = 'sysy2022_complier'
source_filename = "./input/3.sy"

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
    store i32 42, i32* %op1
    %op2 = alloca i32
    store i32 0, i32* %op2
    %op3 = alloca float
    store float 3.140000, float* %op3
    %op4 = alloca float
    store float 0.500000, float* %op4
    %op5 = alloca i32
    store i32 1, i32* %op5
    %op6 = alloca i32
    store i32 2, i32* %op6
    %op7 = load i32, i32* %op2
    %op8 = load i32, i32* %op1
    %op9 = mul i32 %op8, 2
    %op10 = add i32 %op7, %op9
    %op11 = sdiv i32 5, 3
    %op12 = srem i32 %op11, 2
    %op13 = sub i32 %op10, %op12
    store i32 %op13, i32* %op2
    %op14 = load i32, i32* %op5
    %op15 = load i32, i32* %op6
    %op16 = add i32 %op14, %op15
    store i32 %op16, i32* %op5
    %op17 = load i32, i32* %op2
    %op18 = icmp sge i32 %op17, 10
    %op19 = load i32, i32* %op2
    %op20 = icmp ne i32 %op19, 0
    %op21 = zext i1 %op18 to i32
    %op22 = zext i1 %op20 to i32
    %op23 = mul i32 %op21, %op22
    %op24 = icmp ne i32 %op23, 0
    %op25 = load i32, i32* %op5
    %op26 = load i32, i32* %op6
    %op27 = icmp slt i32 %op25, %op26
    %op28 = zext i1 %op24 to i32
    %op29 = zext i1 %op27 to i32
    %op30 = add i32 %op28, %op29
    %op31 = icmp ne i32 %op30, 0
    br i1 %op31, label %if_then0, label %if_else0
if_then0:
    %op33 = load i32, i32* %op2
    %op34 = add i32 %op33, 1
    store i32 %op34, i32* %op2
    br label %if_merge0
if_else0:
    %op36 = load i32, i32* %op2
    %op37 = sub i32 %op36, 1
    store i32 %op37, i32* %op2
    br label %if_merge0
if_merge0:
    %op39 = load float, float* %op3
    %op40 = fcmp ogt float %op39, 1.000000
    %op41 = load float, float* %op4
    %op42 = fcmp ole float %op41, 1.000000
    %op43 = zext i1 %op40 to i32
    %op44 = zext i1 %op42 to i32
    %op45 = mul i32 %op43, %op44
    %op46 = icmp ne i32 %op45, 0
    br i1 %op46, label %if_then1, label %if_else1
if_then1:
    %op48 = load i32, i32* %op5
    %op49 = load i32, i32* %op6
    %op50 = add i32 %op48, %op49
    %op51 = load i32, i32* %op1
    %op52 = sub i32 %op51, 3
    %op53 = mul i32 %op50, %op52
    store i32 %op53, i32* %op5
    br label %if_merge1
if_else1:
    %op55 = load i32, i32* %op5
    %op56 = load i32, i32* %op6
    %op57 = icmp eq i32 %op55, %op56
    br i1 %op57, label %if_then2, label %if_else2
if_merge1:
    ret void
if_then2:
    %op60 = load i32, i32* %op6
    %op61 = add i32 %op60, 1
    store i32 %op61, i32* %op6
    br label %if_merge2
if_else2:
    %op63 = load i32, i32* %op6
    %op64 = sub i32 %op63, 1
    store i32 %op64, i32* %op6
    br label %if_merge2
if_merge2:
    br label %if_merge1
}
