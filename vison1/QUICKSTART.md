# 快速开始指南

## 5分钟快速上手

### 1. 编译编译器

#### Windows
```bash
build.bat
```

#### Linux/Mac
```bash
mkdir build && cd build && cmake .. && make && cd ..
```

### 2. 运行编译器

```bash
./build/bin/compiler test_simple.sy
```

### 3. 查看输出

- **词法分析结果**: `output_lex.txt`
- **LLVM中间代码**: `output.ll`

## 示例

### 创建测试文件 (hello.sy)
```c
int main() {
    int x = 5;
    int y = 10;
    int z = x + y;
    return 0;
}
```

### 编译
```bash
./build/bin/compiler hello.sy
```

### 查看词法分析结果
```bash
cat output_lex.txt
```

输出:
```
int	<KW,1>
main	<KW,5>
(	<SE,23>
)	<SE,24>
{	<SE,25>
int	<KW,1>
x	<IDN,x>
=	<OP,14>
5	<INT,5>
;	<SE,27>
...
```

### 查看生成的LLVM IR
```bash
cat output.ll
```

输出:
```llvm
; ModuleID = 'sysy2022_compiler'
source_filename = "./input.sy"

define i32 @main() {
main_ENTRY:
    %x = alloca i32
    store i32 5, i32* %x
    %y = alloca i32
    store i32 10, i32* %y
    %z = alloca i32
    %op0 = load i32, i32* %x
    %op1 = load i32, i32* %y
    %op2 = add i32 %op0, %op1
    store i32 %op2, i32* %z
    ret i32 0
}
```

## 支持的语法

### 变量声明
```c
int a = 10;
float b = 3.14;
```

### 函数定义
```c
int main() {
    // 函数体
    return 0;
}
```

### 表达式
```c
int c = a + b;
int d = a * b - c;
int e = (a + b) * c;
```

### 语句
```c
a = 10;
return 0;
{
    int x = 5;
}
```

## 常见问题

### Q: 如何编译?
A: 使用 `build.bat` (Windows) 或 `mkdir build && cd build && cmake .. && make` (Linux/Mac)

### Q: 如何运行?
A: `./build/bin/compiler <source_file.sy>`

### Q: 输出文件在哪里?
A: 在当前目录下，`output_lex.txt` 和 `output.ll`

### Q: 支持哪些语言特性?
A: 基本的变量声明、函数定义、表达式、语句等。详见 README.md

### Q: 不支持什么?
A: 数组、指针、结构体、函数参数、复杂控制流等。详见 README.md

## 文件说明

| 文件 | 说明 |
|------|------|
| `SimpleParser.h/cpp` | 语法分析器 |
| `CodeGenerator.h/cpp` | 代码生成器 |
| `main.cpp` | 主程序 |
| `CMakeLists.txt` | 构建配置 |
| `build.bat` | Windows编译脚本 |

## 更多信息

- 详细使用说明: 见 `README.md`
- 实现细节: 见 `IMPLEMENTATION.md`
- 完成总结: 见 `SUMMARY.md`

---

**提示**: 所有命令都应在项目根目录执行
