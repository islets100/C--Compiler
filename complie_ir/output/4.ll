; ModuleID = 'sysy2022_complier'
source_filename = "./input/4.sy"

@a = global i32 10
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
    store i32 10, i32* @a
    ret i32 0
}
