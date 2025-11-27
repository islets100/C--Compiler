# C-- 编译器前端

这是一个C--语言的编译器前端实现，包括词法分析、语法分析和中间代码生成三个部分。

## 项目结构

### 词法分析器 (Lexical Analyzer)
- **文件**: `lex/Lex_Analysis.cpp`
- **功能**: 基于NFA-DFA自动机理论，将源代码转换为词法单元序列
- **输出格式**: `[单词]\t<[类型],[属性]>`
  - 类型: KW(关键字), OP(运算符), SE(界符), IDN(标识符), INT(整数), FLOAT(浮点数)
  - 属性: 对于关键字/运算符/界符为编码，对于标识符/数字为值本身

### 语法分析器 (Syntax Analyzer)
- **文件**: `SimpleParser.h/SimpleParser.cpp`
- **方法**: 递归下降分析
- **功能**: 将词法单元序列转换为抽象语法树(AST)
- **输出**: AST结构，用于后续的代码生成

### 代码生成器 (Code Generator)
- **文件**: `CodeGenerator.h/CodeGenerator.cpp`
- **功能**: 遍历AST，调用中间代码生成库生成LLVM IR
- **输出**: `.ll`格式的LLVM中间代码

### 中间代码生成库 (IR Library)
- **位置**: `complie_ir/`
- **功能**: 提供LLVM IR的构建和输出接口
- **主要类**:
  - `Module`: 模块，包含全局变量和函数
  - `Function`: 函数定义
  - `BasicBlock`: 基本块
  - `IRBuilder`: IR构建器，用于创建各种指令
  - `Instruction`: 各种指令类型

## 编译方法

### 使用CMake (推荐)
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
cd ..
```

### 使用build.bat (Windows)
```bash
build.bat
```

## 使用方法

```bash
./build/bin/compiler <source_file.sy>
```

### 输出文件
- `output_lex.txt`: 词法分析结果
- `output.ll`: LLVM中间代码

## 支持的语言特性

### 数据类型
- `int`: 32位整数
- `float`: 浮点数
- `void`: 无返回值

### 关键字
- `int`, `void`, `return`, `const`, `main`, `float`, `if`, `else`

### 运算符
- 算术: `+`, `-`, `*`, `/`, `%`
- 比较: `>`, `<`, `==`, `<=`, `>=`, `!=`
- 逻辑: `&&`, `||`
- 赋值: `=`

### 界符
- `(`, `)`, `{`, `}`, `;`, `,`

### 语句
- 变量声明和初始化
- 常量声明
- 函数定义
- 赋值语句
- 表达式语句
- 返回语句
- 块语句
- if-else语句

## 示例

### 输入 (test.sy)
```c
int main() {
    int a = 10;
    int b = 20;
    int c = a + b;
    return 0;
}
```

### 词法分析输出 (output_lex.txt)
```
int	<KW,1>
main	<KW,5>
(	<SE,23>
)	<SE,24>
{	<SE,25>
int	<KW,1>
a	<IDN,a>
=	<OP,14>
10	<INT,10>
;	<SE,27>
...
```

### 中间代码输出 (output.ll)
```llvm
; ModuleID = 'sysy2022_compiler'
source_filename = "./test.sy"

define i32 @main() {
main_ENTRY:
    %op0 = alloca i32
    store i32 10, i32* %op0
    ...
    ret i32 0
}
```

## 文法规则

编译器支持以下简化的C--文法:

```
Program -> compUnit
compUnit -> ε | decl compUnit | funcDef compUnit
decl -> constDecl | varDecl
constDecl -> const btype constDef ;
constDef -> Ident = constInitial
constInitial -> constExp
btype -> int | float
varDecl -> btype varDef ;
varDef -> Ident | Ident = initVal
initVal -> exp
funcDef -> funcType Ident ( ) block
funcType -> void | int
block -> { blockItems }
blockItems -> ε | blockItem blockItems
blockItem -> decl | stmt
stmt -> lVal = exp ; | exp ; | ; | block | return ; | return exp ;
exp -> addExp
addExp -> mulExp | addExp + mulExp | addExp - mulExp
mulExp -> unaryExp | mulExp * unaryExp | mulExp / unaryExp | mulExp % unaryExp
unaryExp -> primaryExp
primaryExp -> ( exp ) | lVal | number
number -> IntConst | floatConst
lVal -> Ident
```

## 注意事项

1. 编译器目前支持的是C--语言的一个子集
2. 不支持数组、指针、结构体等复杂特性
3. 函数参数暂不支持
4. 浮点数处理目前转换为整数

## 开发报告

### 词法分析器
- 基于NFA-DFA自动机理论实现
- 使用状态转移表进行词法识别
- 支持关键字、运算符、界符、标识符、整数、浮点数的识别
- 输出格式符合任务要求

### 语法分析器
- 采用递归下降分析方法
- 实现了对C--语言主要语法结构的解析
- 生成抽象语法树(AST)供代码生成使用

### 代码生成器
- 使用访问者模式遍历AST
- 调用中间代码生成库生成LLVM IR
- 支持变量声明、表达式计算、函数定义、语句执行等

## 测试

在`test/`目录下有测试用例:
- `test1.sy`: 基本的变量声明和赋值
- `test2.sy`: 函数定义和调用
- `test3.sy`: 复杂的表达式和控制流

## 许可证

MIT License
