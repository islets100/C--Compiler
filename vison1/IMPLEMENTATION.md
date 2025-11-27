# C-- 编译器前端实现报告

## 一、项目概述

本项目实现了一个C--语言的编译器前端，包括词法分析、语法分析和中间代码生成三个主要部分。编译器能够将C--源代码转换为LLVM中间代码。

## 二、各模块实现详情

### 1. 词法分析器 (Lexical Analyzer)

#### 实现方式
- **基础理论**: 基于NFA-DFA自动机理论
- **文件**: `lex/Lex_Analysis.cpp`
- **核心算法**:
  - NFA构造: 为各类词法单元定义状态转移
  - NFA确定化: 使用子集构造法将NFA转换为DFA
  - DFA最小化: 合并等价状态以优化性能

#### 识别的词法单元

| 类型 | 编码 | 示例 |
|------|------|------|
| 关键字 (KW) | 1-8 | int, void, return, const, main, float, if, else |
| 运算符 (OP) | 9-22 | +, -, *, /, %, =, >, <, ==, <=, >=, !=, &&, \|\| |
| 界符 (SE) | 23-28 | (, ), {, }, ;, , |
| 标识符 (IDN) | - | 变量名、函数名 |
| 整数 (INT) | - | 数字序列 |
| 浮点数 (FLOAT) | - | 包含小数点的数字 |

#### 输出格式
```
[单词]\t<[类型],[属性]>
```

示例:
```
int	<KW,1>
a	<IDN,a>
=	<OP,14>
10	<INT,10>
;	<SE,27>
```

#### 关键特性
- 支持不区分大小写的关键字识别
- 使用超前搜索处理二字符运算符
- 完整的符号表管理

### 2. 语法分析器 (Syntax Analyzer)

#### 实现方式
- **方法**: 递归下降分析 (Recursive Descent Parsing)
- **文件**: `SimpleParser.h/SimpleParser.cpp`
- **特点**: 
  - 简洁易懂，便于维护
  - 能够处理C--语言的主要语法结构
  - 生成抽象语法树(AST)

#### 支持的文法

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

#### 解析过程
1. 从词法分析器获取词法单元序列
2. 递归调用各个非终结符的解析函数
3. 构建抽象语法树(AST)
4. 返回AST根节点供代码生成使用

#### 抽象语法树 (AST)

AST节点结构:
```cpp
struct ASTNode {
    string name;      // 节点名称
    string value;     // 节点值
    vector<shared_ptr<ASTNode>> children;  // 子节点
    int production;   // 使用的产生式编号
};
```

### 3. 代码生成器 (Code Generator)

#### 实现方式
- **设计模式**: 访问者模式 (Visitor Pattern)
- **文件**: `CodeGenerator.h/CodeGenerator.cpp`
- **目标**: 生成LLVM中间代码

#### 核心功能

##### 访问函数 (Visitor Functions)

| 函数 | 功能 |
|------|------|
| visitProgram | 访问程序根节点 |
| visitCompUnit | 访问编译单元 |
| visitDecl | 访问声明 |
| visitVarDecl | 访问变量声明 |
| visitFuncDef | 访问函数定义 |
| visitBlock | 访问代码块 |
| visitStmt | 访问语句 |
| visitExp | 访问表达式 |
| visitExpressionNode | 处理表达式节点 |

##### 中间代码生成

使用中间代码库 (`complie_ir/`) 提供的API:

- **Module**: 管理全局变量和函数
- **Function**: 函数定义
- **BasicBlock**: 基本块
- **IRBuilder**: 指令构建器

#### 支持的操作

1. **变量声明和初始化**
   ```cpp
   int a = 10;
   ```
   生成:
   ```llvm
   %a = alloca i32
   store i32 10, i32* %a
   ```

2. **二元运算**
   ```cpp
   int c = a + b;
   ```
   生成:
   ```llvm
   %op0 = load i32, i32* %a
   %op1 = load i32, i32* %b
   %op2 = add i32 %op0, %op1
   ```

3. **函数定义**
   ```cpp
   int main() { ... }
   ```
   生成:
   ```llvm
   define i32 @main() {
   main_ENTRY:
       ...
       ret i32 0
   }
   ```

4. **返回语句**
   ```cpp
   return 0;
   ```
   生成:
   ```llvm
   ret i32 0
   ```

#### 符号表管理

- **全局变量表** (`global_vars`): 存储全局变量
- **局部符号表** (`symbol_table`): 存储局部变量和函数参数

## 三、编译流程

```
源代码 (.sy文件)
    ↓
[词法分析器] → 词法单元序列
    ↓
[语法分析器] → 抽象语法树 (AST)
    ↓
[代码生成器] → LLVM中间代码 (.ll文件)
    ↓
LLVM后端 → 目标代码
```

## 四、输出示例

### 输入程序 (test.sy)
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
int	<KW,1>
b	<IDN,b>
=	<OP,14>
20	<INT,20>
;	<SE,27>
int	<KW,1>
c	<IDN,c>
=	<OP,14>
a	<IDN,a>
+	<OP,9>
b	<IDN,b>
;	<SE,27>
return	<KW,3>
0	<INT,0>
;	<SE,27>
}	<SE,26>
```

### 代码生成输出 (output.ll)
```llvm
; ModuleID = 'sysy2022_compiler'
source_filename = "./input.sy"

define i32 @main() {
main_ENTRY:
    %a = alloca i32
    store i32 10, i32* %a
    %b = alloca i32
    store i32 20, i32* %b
    %c = alloca i32
    %op0 = load i32, i32* %a
    %op1 = load i32, i32* %b
    %op2 = add i32 %op0, %op1
    store i32 %op2, i32* %c
    ret i32 0
}
```

## 五、文件结构

```
Finallab/
├── lex/
│   ├── Lex_Analysis.cpp          # 词法分析器实现
│   ├── test/                      # 测试用例
│   └── output/                    # 词法分析输出
├── complie_ir/                    # 中间代码生成库
│   ├── include/                   # 头文件
│   └── src/                       # 源文件
├── SimpleParser.h/cpp             # 语法分析器
├── CodeGenerator.h/cpp            # 代码生成器
├── main.cpp                       # 主程序
├── CMakeLists.txt                 # CMake配置
├── README.md                      # 使用说明
└── IMPLEMENTATION.md              # 本文件
```

## 六、编译和运行

### 编译
```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
cd ..
```

### 运行
```bash
./build/bin/compiler test_simple.sy
```

### 输出文件
- `output_lex.txt`: 词法分析结果
- `output.ll`: LLVM中间代码

## 七、已知限制

1. **不支持的特性**:
   - 数组和指针
   - 结构体和联合体
   - 函数参数
   - 预处理指令 (#include, #define等)
   - 复杂的控制流 (while, for, switch等)

2. **简化处理**:
   - 浮点数转换为整数处理
   - 没有类型检查
   - 没有优化

3. **错误处理**:
   - 基本的语法错误检测
   - 没有详细的错误信息和恢复机制

## 八、扩展建议

1. **完善语法分析**:
   - 实现完整的SLR分析器
   - 添加更详细的错误报告

2. **增强代码生成**:
   - 支持数组和指针
   - 支持函数参数和调用
   - 添加优化pass

3. **改进错误处理**:
   - 实现错误恢复机制
   - 提供更有用的错误信息

4. **添加测试框架**:
   - 单元测试
   - 集成测试
   - 性能测试

## 九、参考资源

- LLVM官方文档: https://llvm.org/docs/
- 编译原理教材
- C--语言规范文档

---

**实现日期**: 2025年11月
**开发者**: 编译原理课程学生
**版本**: 1.0
