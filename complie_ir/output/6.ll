; ModuleID = 'sysy2022_complier'
source_filename = "./input/6.sy"

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
    store i32 10, i32* %op1
    %op2 = alloca i32
    store i32 20, i32* %op2
    %op3 = alloca i32
    store i32 30, i32* %op3
    %op4 = alloca i32
    store i32 0, i32* %op4
    %op5 = load i32, i32* %op1
    %op6 = load i32, i32* %op2
    %op7 = icmp slt i32 %op5, %op6
    br i1 %op7, label %if_then0, label %if_merge0
if_then0:
    %op9 = load i32, i32* %op2
    %op10 = load i32, i32* %op3
    %op11 = icmp slt i32 %op9, %op10
    br i1 %op11, label %if_then1, label %if_else1
if_merge0:
    %op13 = load i32, i32* %op1
    %op14 = load i32, i32* %op2
    %op15 = icmp slt i32 %op13, %op14
    %op16 = load i32, i32* %op2
    %op17 = load i32, i32* %op3
    %op18 = icmp sgt i32 %op16, %op17
    %op19 = load i32, i32* %op1
    %op20 = icmp eq i32 %op19, 10
    %op21 = zext i1 %op18 to i32
    %op22 = zext i1 %op20 to i32
    %op23 = mul i32 %op21, %op22
    %op24 = icmp ne i32 %op23, 0
    %op25 = zext i1 %op15 to i32
    %op26 = zext i1 %op24 to i32
    %op27 = add i32 %op25, %op26
    %op28 = icmp ne i32 %op27, 0
    br i1 %op28, label %if_then2, label %if_merge2
if_then1:
    store i32 1, i32* %op4
    br label %if_merge1
if_else1:
    store i32 2, i32* %op4
    br label %if_merge1
if_merge1:
    br label %if_merge0
if_then2:
    %op33 = load i32, i32* %op4
    %op34 = add i32 %op33, 100
    store i32 %op34, i32* %op4
    br label %if_merge2
if_merge2:
    %op36 = load i32, i32* %op4
    %op37 = icmp sgt i32 %op36, 0
    br i1 %op37, label %if_then3, label %if_else3
if_then3:
    %op39 = alloca i32
    store i32 5, i32* %op39
    %op40 = load i32, i32* %op39
    %op41 = icmp ne i32 %op40, 0
    br i1 %op41, label %if_then4, label %if_merge4
if_else3:
    store i32 0, i32* %op4
    br label %if_merge3
if_then4:
    %op44 = load i32, i32* %op4
    %op45 = mul i32 %op44, 2
    store i32 %op45, i32* %op4
    br label %if_merge4
if_merge4:
    br label %if_merge3
if_merge3:
    %op48 = load i32, i32* %op4
    ret i32 %op48
}
