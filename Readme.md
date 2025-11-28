# 词法-语法分析器使用

## 大概说一下完成情况

项目的前两部分目前是这样，实现了一个完整的编译器前端工具链，包括：
- **词法分析器**：将源程序转换为词法单元（Token）序列
  - ✅正则->NFA
  - ✅DFA数据结构/类设计
  - ✅NFA->DFA, DFA 最小化
  - ✅输出单词值和种别，报错信息

- **中间代码生成器**：遍历抽象语法树，生成 LLVM IR 中间代码
  - ✅**Visitor 模式实现**：为每个语法树节点实现对应的 visitor 函数
  - ✅**符号表管理**：全局变量、局部变量作用域管理
  - ✅**表达式求值**：算术运算、逻辑运算、比较运算
  - ✅**控制流处理**：if-else 分支、return 语句
  - ✅**类型系统**：int、float、bool 类型支持
  - ✅**LLVM IR 输出**：生成符合规范的 `.ll` 格式文件

- **语法分析器**：使用 SLR(1) 分析方法构建抽象语法树（AST）
  - ✅**修改文法**
  - ✅**对词法传来的lex文件进行解析**，**建立 Token 映射**
  - ✅**`productions` (项目集规范族的基础) 和 `first/followSet` 的构建**
  - ❌**构造 LR(0) 项目集与 SLR 分析表**(Action/Goto Table)
  - ✅**语法分析驱动程序 (Parser Driver)**：分析输出规约过程
  - ✅**规约序列提取**：从上一步的里面提取老师要求的格式的规约序列



## 目录结构

```
Finallab/
├── lex/                          # 词法分析器目录
│   ├── Lex_Analysis.cpp         # 词法分析器主程序
│   ├── Lex_Analysis.exe         # 编译后的可执行文件
│   ├── test/                    # 测试用例目录
│   │   ├── test1.sy ~ test4.sy # 测试源文件
│   └── output/                  # 词法分析输出目录
│       └── lex1.txt ~ lex4.txt  # 词法分析结果（作业要求）
│
├── Syntax/                       # 语法分析器目录
│   ├── main.cpp                 # 语法分析器主程序
│   ├── GrammarAnalyzer.cpp/h    # 文法分析器
│   ├── SLRTable.cpp/h           # SLR 表构建
│   ├── Parser.cpp/h             # 语法分析器核心
│   ├── ReductionSequenceLogger.cpp/h  # 规约序列记录器
│   ├── SyntaxTree.h             # 语法树结构
│   ├── IRGenerator.cpp/h        # 中间代码生成器
│   ├── compiler_ir.exe          # 编译后的可执行文件（包含IR生成）
│   └── output/                  # 语法分析输出目录
│       ├── 1out.txt ~ 4out.txt  # 完整分析日志
│       ├── syntax1.txt ~ syntax4.txt  # 规约序列（作业要求）
│       ├── tree1.txt ~ tree4.txt      # 语法树（中间代码生成基于此）
│       └── 1.ll ~ 4.ll                 # LLVM IR 中间代码（作业要求）
│
├── complie_ir/                   # 中端代码库（提供的框架）
│   ├── include/                  # 头文件目录
│   │   ├── Module.h             # 模块类
│   │   ├── Function.h           # 函数类
│   │   ├── BasicBlock.h          # 基本块类
│   │   ├── Instruction.h         # 指令类
│   │   ├── IRbuilder.h           # IR 构建器
│   │   └── ...
│   └── src/                      # 源文件目录
│
├── build.bat                    # Windows 编译脚本
├── run.bat                      # Windows 运行脚本
├── Makefile                     # Linux/Mac 构建文件
└── 词法-语法器.md               # 本说明文档
```

 

## 快速开始

### 1.编译项目

#### Windows 系统

双击运行 `build.bat`，或在finallab目录下命令行中执行：

```batch
./build.bat
```

这将编译：

1. 词法分析器 → `lex/Lex_Analysis.exe`
2. 语法分析器（含IR生成） → `Syntax/compiler_ir.exe`

#### Linux/Mac 系统

在项目根目录执行：

```bash
make all
```

或分别编译：

```bash
make lex        # 仅编译词法分析器
make grammar    # 仅编译语法分析器
```

### 2.运行完整分析流程

#### Windows 系统

双击运行 `run.bat`，或在命令行中执行：

```batch
run.bat
```

这将自动：

1. 运行词法分析器（处理 `lex/test/test*.sy` 文件）
2. 运行语法分析器（读取词法分析结果，生成语法树）
3. 生成中间代码（遍历语法树，生成 LLVM IR）
4. 生成所有输出文件

#### Linux/Mac 系统

```bash
make run-all
```

或分别运行：

```bash
make run-lex      # 仅运行词法分析器
make run-grammar  # 仅运行语法分析器（需要先运行词法分析）
```

#### 单步调试

详情请见下面的配置与自定义部分。

## 输出文件说明

### 1.词法分析输出

**位置**：`lex/output/lex1.txt` ~ `lex4.txt`

**格式**：

```
<token文本>	<类型,编号>
```

**示例**：

```
int	<KW,1>
main	<KW,5>
(	<SE,23>
)	<SE,24>
{	<SE,25>
```

**错误标记**：

```
^	<ERROR,行号,列号>
```

### 2.语法分析输出

语法分析器会生成三类输出文件：

#### 1. 完整分析日志 (`*out.txt`)

**位置**：`Syntax/output/1out.txt` ~ `4out.txt`

**内容**：

- 文法初始化信息

- FIRST 集和 FOLLOW 集

  ```
  ===== FIRST SETS =====
  Program        : { eps int void const float }
  CompUnit       : { eps int void const float }
  UnitList       : { eps int void const float }
  Unit           : { int void const float }
  Decl           : { int const float }
  ConstDecl      : { const }
  VarDecl        : { int float }
  ConstDefList   : { ID }
  BType          : { int float }
  ConstDef       : { ID }
  ConstInit      : { ID INT FLOAT + - ( }
  VarDefList     : { ID }
  VarDef         : { ID }
  InitVal        : { ID INT FLOAT + - ( }
  FuncDef        : { int void float }
  FuncName       : { ID main }
  OptFParams     : { eps int float }
  FuncType       : { void }
  FuncFParams    : { int float }
  FuncFParam     : { int float }
  Block          : { { }
  BlockItemList  : { eps ID INT FLOAT int return const float if + - ( { ; }
  BlockItem      : { ID INT FLOAT int return const float if + - ( { ; }
  Stmt           : { ID INT FLOAT return if + - ( { ; }
  OptExp         : { eps ID INT FLOAT + - ( }
  Exp            : { ID INT FLOAT + - ( }
  Cond           : { ID INT FLOAT + - ( }
  LVal           : { ID }
  PrimaryExp     : { ID INT FLOAT ( }
  Number         : { INT FLOAT }
  UnaryExp       : { ID INT FLOAT + - ( }
  OptRParams     : { eps ID INT FLOAT + - ( }
  UnaryOp        : { + - }
  FuncRParams    : { ID INT FLOAT + - ( }
  MulExp         : { ID INT FLOAT + - ( }
  AddExp         : { ID INT FLOAT + - ( }
  RelExp         : { ID INT FLOAT + - ( }
  EqExp          : { ID INT FLOAT + - ( }
  LAndExp        : { ID INT FLOAT + - ( }
  LOrExp         : { ID INT FLOAT + - ( }
  ConstExp       : { ID INT FLOAT + - ( }
  
  ===== FOLLOW SETS =====
  Program        : { $ }
  CompUnit       : { $ }
  UnitList       : { $ int void const float }
  Unit           : { $ int void const float }
  Decl           : { $ ID INT FLOAT int void return const float if + - ( { } ; }
  ConstDecl      : { $ ID INT FLOAT int void return const float if + - ( { } ; }
  VarDecl        : { $ ID INT FLOAT int void return const float if + - ( { } ; }
  ConstDefList   : { ; }
  BType          : { ID main }
  ConstDef       : { ; , }
  ConstInit      : { ; , }
  VarDefList     : { ; }
  VarDef         : { ; , }
  InitVal        : { ; , }
  FuncDef        : { $ int void const float }
  FuncName       : { ( }
  OptFParams     : { ) }
  FuncType       : { ID main }
  FuncFParams    : { ) , }
  FuncFParam     : { ) , }
  Block          : { $ ID INT FLOAT int void return const float if else + - ( { } ; }
  BlockItemList  : { ID INT FLOAT int return const float if + - ( { } ; }
  BlockItem      : { ID INT FLOAT int return const float if + - ( { } ; }
  Stmt           : { ID INT FLOAT int return const float if else + - ( { } ; }
  OptExp         : { ; }
  Exp            : { ) ; , }
  Cond           : { ) }
  LVal           : { + - * / % = > < == <= >= != && || ) ; , }
  PrimaryExp     : { + - * / % > < == <= >= != && || ) ; , }
  Number         : { + - * / % > < == <= >= != && || ) ; , }
  UnaryExp       : { + - * / % > < == <= >= != && || ) ; , }
  OptRParams     : { ) }
  UnaryOp        : { ID INT FLOAT + - ( }
  FuncRParams    : { ) , }
  MulExp         : { + - * / % > < == <= >= != && || ) ; , }
  AddExp         : { + - > < == <= >= != && || ) ; , }
  RelExp         : { > < == <= >= != && || ) ; , }
  EqExp          : { == != && || ) ; , }
  LAndExp        : { && || ) ; , }
  LOrExp         : { || ) ; , }
  ConstExp       : { ; , }
  ```

- SLR 表构建信息，132个状态，还没实现输出

- 完整的分析过程（PARSING PROCESS 表格）

  如果lex传过来的有不合法的源输入，会有详细提示：

  <img src="pic\image-20251127233806726.png" alt="image-20251127233806726"  />

  对于error的类型，会有输出信息然后跳过，不影响后面语句继续解析：

  <img src="pic\image-20251127233426693.png" alt="image-20251127233426693"  />

  对于语法错误，会输出详细的错误信息，当前状态，以及期望符号集，可以帮助检查语法错误

  <img src="pic\image-20251127233522611.png" alt="image-20251127233522611"  />

  **PARSING PROCESS 表格格式**：

  ```
  Step    State   Input   Action
  1       0       int     Reduce 4 (UnitList -> ...)
  2       2       int     Shift 3
  ...
  ```

- 语法生成树

  ```
  ================ SYNTAX TREE ================
  CompUnit
    UnitList
      UnitList
        UnitList
        Unit
          Decl
            VarDecl
              BType
                int (int)
              VarDefList
                VarDef
                  ID (a)
                  = (=)
                  InitVal
                    Exp
                      LOrExp
                        LAndExp
                          EqExp
                            RelExp
                              AddExp
                                MulExp
                                  UnaryExp
                                    PrimaryExp
                                      Number
                                        INT (10)
              ; (;)
      Unit
        FuncDef
          BType
            int (int)
          FuncName
            main (main)
          ( (()
          OptFParams
          ) ())
          Block
            { ({)
            BlockItemList
              BlockItemList
                BlockItemList
                BlockItem
                  Stmt
                    LVal
                      ID (a)
                    = (=)
                    Exp
                      LOrExp
                        LAndExp
                          EqExp
                            RelExp
                              AddExp
                                MulExp
                                  UnaryExp
                                    PrimaryExp
                                      Number
                                        INT (10)
                    ; (;)
              BlockItem
                Stmt
                  return (return)
                  OptExp
                    Exp
                      LOrExp
                        LAndExp
                          EqExp
                            RelExp
                              AddExp
                                MulExp
                                  UnaryExp
                                    PrimaryExp
                                      Number
                                        INT (0)
                  ; (;)
            } (})
  ```

​	❗ **所以，不管你遇到了什么问题，都可以在日志out文件里先找找答案，因为我也不确定这个语法器是完全正确的，中间代码生成如果需要调整语法器，有问题要要修改可以借助out日志文件**

#### 2. 规约序列 (`syntax*.txt`)

这个是用来交作业的。就是从out里提取的

**位置**：`Syntax/output/syntax1.txt` ~ `syntax4.txt`

**格式**：

```
[序号][TAB][栈顶符号]#[面临输入符号][TAB][执行动作]
```

**执行动作类型**：

- `move` - 移进（Shift）
- `reduction` - 归约（Reduce）
- `accept` - 接受
- `error` - 错误

**示例**：

```
1	int#int	move
2	bType#int	reduction
3	Ident#a	move
4	=#=	move
5	IntConst#10	move
```

#### 3. 语法树 (`tree*.txt`)

中间代码最好基于这个来写
树是忽略错误符号的，也就是说是对于修正过的词法序列生成的，但是对于语法错误，树就没法生成了。

**位置**：`Syntax/output/tree1.txt` ~ `tree4.txt`

**格式**：

```
Program
  CompUnit
    UnitList
      Unit
        FuncDef
          BType
            int (int)
          FuncName
            main (main)
        ...
```

**说明**：

  我们以 `1out.txt` 中解析 `int a = 1, b = 2;` 的这部分为例进行解读：

```
VarDecl                  <-- 顶层：这是一个“变量声明”结构
  BType
    int (int)            <-- 类型是 int
  VarDefList             <-- 接下来是一个变量定义列表
    VarDef               <-- 第一个变量定义
      ID (a)             <-- 变量名 a
      = (=)              <-- 赋值号
      InitVal            <-- 初始值
        Exp...           <-- 经过层层表达式推导
          Number
            INT (1)      <-- 值是 1
    , (,)                <-- 逗号分隔
    VarDefList           <-- 列表的递归部分（第二个变量）
      VarDef
        ID (b)           <-- 变量名 b
        = (=)
        InitVal...
          INT (2)        <-- 值是 2
  ; (;)                  <-- 分号结束声明
```

**解读方法：**

1. **缩进代表层级**：每一级缩进表示“我是上面那个节点的子节点”。这个在code中查看比较清晰：如果遍历不方便的话可以想想怎么处理这个树结构的输入。

   <img src="pic\image-20251128000954953.png" alt="image-20251128000954953" style="zoom:50%;" /><img src="pic\image-20251128001036643.png" alt="image-20251128001036643" style="zoom:50%;" />

2. **非终结符 (Non-Terminal)**：显示符号名称，如 `VarDecl`, `VarDef`, `Exp`。它们代表一种语法结构（语句、表达式等）。

3. **终结符 (Terminal)**：示 `符号名 (文本值)`，如 `int (int)`, `ID (a)`, `INT (1)`。它们是树的**叶子**，对应源代码中实际写出来的字。格式通常是 `Token类型 (Token文本)`。

**这棵树的作用：** 它告诉你程序的**结构化含义**。看到 `VarDecl`，编译器就知道"哦，这里要分配内存了"。看到 `BinaryExp` (在你的树里是 `AddExp` 下有两个子节点)，就知道"这里要做加法运算"。

### 3.中间代码生成输出

中间代码生成器会生成 LLVM IR 格式的中间代码文件。

**位置**：`Syntax/output/1.ll` ~ `4.ll`

**格式**：标准的 LLVM IR 格式

**示例**（`1.ll`）：

```llvm
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
```

**说明**：

1. **模块头部**：
   - `; ModuleID = 'sysy2022_complier'` - 模块标识符
   - `source_filename = "./input/XX.sy"` - 源文件名

2. **函数声明**：标准库函数声明（`getinit`, `getch`, `getarray`, `putinit`, `putch`, `putarray`, `starttime`, `stoptime`）

3. **全局变量**（如果有）：`@变量名 = global i32 初始值`

4. **函数定义**：
   - `define i32 @main()` 或 `define void @main()` - 函数定义
   - `main_ENTRY:` - 入口基本块标签
   - `%op编号 = 操作码 类型, 操作数...` - SSA 格式的指令

5. **指令类型**：
   - `alloca` - 分配局部变量内存
   - `load` / `store` - 内存读写
   - `add`, `sub`, `mul`, `sdiv`, `srem` - 算术运算
   - `icmp`, `fcmp` - 比较运算
   - `br` - 分支跳转
   - `ret` - 返回语句

**验证方法**：

### 方法 1：使用 LLVM 工具链验证（推荐）

如果已安装 LLVM，可以使用以下命令验证：

**Windows (PowerShell/CMD)**：
```powershell
# 验证 IR 语法（如果成功会生成 .bc 文件，无输出表示成功）
llvm-as Syntax\output\1.ll

# 或者使用 clang 编译（需要先安装 LLVM 或 Visual Studio）
clang Syntax\output\1.ll -o test1.exe
```

**安装 LLVM（Windows）**：
1. 访问 [LLVM Releases](https://github.com/llvm/llvm-project/releases) 下载 Windows 安装包
2. 或者使用包管理器安装：
   ```powershell
   # 使用 Chocolatey
   choco install llvm
   
   # 或使用 Scoop
   scoop install llvm
   ```
3. 安装后需要将 LLVM 的 `bin` 目录添加到系统 PATH 环境变量

**检查是否已安装**：
```powershell
# 检查 llvm-as 是否可用
where llvm-as

# 检查 clang 是否可用
where clang
```

### 方法 2：手动检查格式（无需安装工具）

如果未安装 LLVM 工具，可以通过以下方式手动验证：

1. **检查基本格式**：
   - 文件应以 `; ModuleID` 开头
   - 包含 `source_filename` 声明
   - 包含标准库函数声明（`declare` 语句）
   - 函数定义以 `define` 开头，以 `}` 结尾

2. **检查语法结构**：
   - 每个基本块以标签开头（如 `main_ENTRY:`, `if_then:`）
   - 指令格式：`%op编号 = 操作码 类型, 操作数...`
   - 所有括号、大括号应匹配
   - 每个函数应以 `ret` 指令结束

3. **检查常见错误**：
   - 未定义的变量引用
   - 类型不匹配（如 `i32` 和 `float` 混用）
   - 跳转标签未定义
   - 缺少返回语句

4. **对比参考输出**：
   - 查看 `Syntax/output/comparison_report.md` 中的格式检查报告
   - 对比生成的 IR 文件与标准格式的差异

## 详细说明

- 词法部分

  传递给到语法器的是lex-n.txt文件，类似这样：

  ```
  VoID	<KW,2>
  MaIn	<KW,5>
  (	<SE,23>
  )	<SE,24>
  {	<SE,25>
  @	<ERROR,1,28>//错误信息是行号和列号
  CoNsT	<KW,4>
  InT	<KW,1>
  _X1	<IDN,_X1>
  =	<OP,14>
  42	<INT,42>
  ,	<SE,28>
  y	<IDN,y>
  ```

- 语法部分

  - ✅**修改文法**：EBNF 转 CFG，作业附录给出的文法使用了 EBNF 符号（`*`, `?`, `|`, `()`），例如规则 2 和规则 17。标准的 SLR 算法只能处理 **上下文无关文法（CFG）**。需要将所有 EBNF 写法展开为递归产生式。**更改完的文法我会整合一下发在`c--产生式.md`文件里**。最后采用的详细的产生式以注释的形式放在了`Syntax\GrammarAnalyzer.cpp`文件的`initGrammar`内，有几条做了更改，注释也写了更改的原因和方式，第3和7是由于对`，`的识别，10和11是由于main这个KW的定义导致了冲突，17是由于原来的规则不支持括号内的逻辑/关系运算。

    > 相关文件
    >
    > 1. c--产生式.md
    >
    > 2. GrammarAnalyzer.cpp

  - ✅**对词法传来的lex文件进行解析**，**建立 Token 映射**：将词法分析器输出的 `<种别, 属性>` 映射为语法分析器能识别的终结符。这部分在`Syntax\main.cpp`的第**16-181**行，包含了对`<ERROR,行号,列号>`这种特殊情况的处理，对于输入的不符合规则的情况会另外打印解析错误信息，比如这样：`<hello, 88>`，会在`*out.txt`日志类文件里打印出来这一行的debug信息的。

    > 相关文件
    >
    > 1.  main.cpp    映射和解析逻辑在这里
    > 2.  Parser.cpp   对ERROR的信息输出逻辑在这里

    | **终结符名称**  | **对应词法输出 (种别, 属性值)** | **说明**                |
    | --------------- | ------------------------------- | ----------------------- |
    | **ID**          | `<IDN, 字符串>`                 | 变量名、函数名          |
    | **INT_CONST**   | `<INT, 数值>`                   | 整数常量                |
    | **FLOAT_CONST** | `<FLOAT, 数值>`                 | 浮点常量                |
    | **KW_INT**      | `<KW, 1>`                       | `int`                   |
    | **KW_VOID**     | `<KW, 2>`                       | `void`                  |
    | **KW_RETURN**   | `<KW, 3>`                       | `return`                |
    | **KW_CONST**    | `<KW, 4>`                       | `const`                 |
    | **KW_MAIN**     | `<KW, 5>`                       | **注意：main 是关键字** |
    | **KW_FLOAT**    | `<KW, 6>`                       | `float`                 |
    | **KW_IF**       | `<KW, 7>`                       | `if`                    |
    | **KW_ELSE**     | `<KW, 8>`                       | `else`                  |
    | **OP_PLUS**     | `<OP, 9>`                       | `+`                     |
    | **OP_MINUS**    | `<OP, 10>`                      | `-`                     |
    | **OP_MUL**      | `<OP, 11>`                      | `*`                     |
    | **OP_DIV**      | `<OP, 12>`                      | `/`                     |
    | **OP_MOD**      | `<OP, 13>`                      | `%`                     |
    | **OP_ASSIGN**   | `<OP, 14>`                      | `=`                     |
    | **OP_GT**       | `<OP, 15>`                      | `>`                     |
    | **OP_LT**       | `<OP, 16>`                      | `<`                     |
    | **OP_EQ**       | `<OP, 17>`                      | `==`                    |
    | **OP_LE**       | `<OP, 18>`                      | `<=`                    |
    | **OP_GE**       | `<OP, 19>`                      | `>=`                    |
    | **OP_NEQ**      | `<OP, 20>`                      | `!=`                    |
    | **OP_AND**      | `<OP, 21>`                      | `&&`                    |
    | **OP_OR**       | `<OP, 22>`                      | `||`                    |
    | **SE_LPAREN**   | `<SE, 23>`                      | `(`                     |
    | **SE_RPAREN**   | `<SE, 24>`                      | `)`                     |
    | **SE_LBRACE**   | `<SE, 25>`                      | `{`                     |
    | **SE_RBRACE**   | `<SE, 26>`                      | `}`                     |
    | **SE_SEMICN**   | `<SE, 27>`                      | `;`                     |
    | **SE_COMMA**    | `<SE, 28>`                      | `,`                     |

  -  ✅**`productions` (项目集规范族的基础) 和 `first/followSet` 的构建**：这部分实现在`Syntax\GrammarAnalyzer.cpp`中的，`main`会调用`GrammarAnalyzer`类里的`printSets`函数在out文件里打印出来所有的`first/followSet`，可以有个debug的参考，如果后面做中间代码生成发现树有问题需要改语法器的话，可以找找是不是我实现的**两个set和action还是goto表**（表也有打印信息，但是暂时还没有像follow集这样完整的打印）有问题。

    > 相关文件：
    >
    > 1. 头文件与数据结构 (`GrammarAnalyzer.h`)
    >
    > 这里我们扩展了之前的枚举，加入非终结符（对应我们之前约定的纯 CFG 文法），并定义了计算所需的集合容器。
    >
    > 2. 算法实现 (`GrammarAnalyzer.cpp`)
    >
    > 这里使用了不动点迭代法。这是实现 FIRST/FOLLOW 最稳健的方法，特别是在处理左递归文法（如 `addExp -> addExp + mulExp`）时，比纯递归更容易避免死循环。

  - ❌**构造 LR(0) 项目集与 SLR 分析表**(Action/Goto Table)：

    这是最复杂的一步，在 `SLRTable` 类里。它需要完成以下任务：

    1. ✅**构造项目集 (Items)**：比如 `Stmt -> if . ( Cond )`。
    2. ✅**计算闭包 (Closure)** 和 **转换 (GoTo)**。
    3. ✅**生成分析表**：这里需要提一下遇到 **移进-归约冲突 (Shift-Reduce Conflict)** 时（特指 If-Else），这里强制选择 **移进 (Shift)**。
    4. ❌**输出结果**：将分析表保存为 CSV 或打印出来

    > 相关文件：
    >
    > SLRTable.h     SLRTable.cpp
    >
    > 构造规范族主要依赖两个函数：
    >
    > 1. **`getClosure(I)` (闭包计算)**：
    >    - **原理**：如果我期待读 `A` (即 `... -> . A ...`)，而 `A` 又能展开成 `B C`，那么我实际上也在期待读 `B` (即 `A -> . B C`)。
    >    - **代码体现**：`SLRTable::getClosure` 函数。
    > 2. **`getGoto(I, X)` (状态转换)**：
    >    - **原理**：如果我在状态 I，读入符号 X，圆点就会后移，形成新的状态。
    >    - **代码体现**：`SLRTable::getGoto` 函数。
    > 3. **`buildTable()` (构建主循环)**：
    >    - 这个函数通过不断循环 `Closure` 和 `GoTo`，把 State 0, State 1... 一个个找出来，放入 `states` 数组中。

  - ✅**语法分析驱动程序 (Parser Driver)**：分析输出规约过程

    它的任务是：

    1. 读取 Token 流。
    2. 维护 **状态栈** 和 **符号栈**（用于构建语法树）。
    3. 查表执行 Shift（移进）或 Reduce（归约）。
    4. **构建语法树 (AST)** 并输出归约序列。

    负责输出PARSING PROCESS表，为了改bug我们在日志输出的时候选择建立了一个特别详细好读的表，出现问题可以查这个 PARSING PROCESS表，包含Step，State，Input，Action四个信息，以及如果是规约，应该被规约为什么（reduction后面括号里的内容）

    > 相关文件：
    >
    > 1. `Parser.h` 和 `Parser.cpp`  分析器驱动 
    > 2. `SyntaxTree.h `   用于生成语法分析树的

  - ✅**规约序列提取**：从上一步的里面提取老师要求的格式的规约序列

    > 相关文件：
    >
    > 1.  ReductionSequenceLogger.cpp 
    > 2.  ReductionSequenceLogger.h

- 中间代码生成部分

  - ✅**IRGenerator 类设计**：采用 Visitor 模式遍历语法树，为每个语法节点实现对应的 visitor 函数。IRGenerator 负责管理符号表、生成 IR 指令、处理控制流等。

    > 相关文件：
    >
    > 1. IRGenerator.h / IRGenerator.cpp - 中间代码生成器核心实现
    >
    > 2. complie_ir/ - 提供的中端代码框架（Module, Function, BasicBlock, Instruction 等类）

  - ✅**符号表管理**：
    
    1. **全局变量表** (`globalVars_`)：存储全局变量信息（地址、类型、是否const）
    2. **作用域栈** (`scopeStack_`)：使用栈结构管理局部变量的作用域，支持嵌套作用域
    3. **函数表** (`functions_`)：存储函数信息（函数对象、返回类型、参数类型列表）
    
    变量查找遵循作用域规则：先在当前作用域查找，再向上层作用域查找，最后查找全局变量。

    > 相关函数：
    >
    > - `pushScope()` / `popScope()` - 作用域管理
    > - `lookupVar()` - 变量查找
    > - `defineLocal()` - 定义局部变量

  - ✅**表达式求值**：
    
    实现了完整的表达式求值系统，支持：
    
    1. **算术表达式**：`evalAdd()`, `evalMul()`, `evalUnary()` - 处理 `+`, `-`, `*`, `/`, `%` 运算
    2. **关系表达式**：`evalRel()`, `evalEq()` - 处理 `>`, `<`, `>=`, `<=`, `==`, `!=` 比较
    3. **逻辑表达式**：`evalLAnd()`, `evalLOr()` - 处理 `&&`, `||` 逻辑运算
    4. **类型转换**：`ensureInt()`, `ensureBool()`, `castBoolToInt()` - 处理类型转换和提升
    
    表达式求值采用递归下降的方式，按照运算符优先级从低到高处理（LOr → LAnd → Eq → Rel → Add → Mul → Unary → Primary）。

    > 相关函数：
    >
    > - `evalExp()` - 表达式求值入口
    > - `evalLOr()`, `evalLAnd()`, `evalEq()`, `evalRel()`, `evalAdd()`, `evalMul()`, `evalUnary()`, `evalPrimary()` - 各层表达式求值
    > - `evalLVal()` - 左值求值（变量读取）

  - ✅**控制流处理**：
    
    1. **if-else 语句**：
       - 使用 `ifLabelCounter_` 为嵌套的 if-else 生成唯一标签（`if_then1`, `if_else1`, `if_merge1` 等）
       - 通过 `buildCondValue()` 构建条件值，生成条件分支指令
       - 正确处理 then 和 else 分支，并在最后合并到 `if_merge` 标签
    
    2. **return 语句**：
       - 根据函数返回类型（`currentReturnType_`）生成相应的 return 指令
       - void 函数使用 `ret void`，有返回值函数使用 `ret i32` 或 `ret float`
    
    3. **基本块管理**：
       - 使用 IRBuilder 管理当前基本块
       - 自动处理基本块的创建和跳转

    > 相关函数：
    >
    > - `visitStmt()` - 语句处理入口
    > - `buildCondValue()` - 构建条件值
    > - `finalizeCurrentFunction()` - 完成函数生成

  - ✅**类型系统**：
    
    支持三种基本类型：
    
    - `ValueKind::INT` - 整数类型（对应 LLVM `i32`）
    - `ValueKind::FLOAT` - 浮点类型（对应 LLVM `float`）
    - `ValueKind::BOOL` - 布尔类型（对应 LLVM `i1`，用于条件判断）
    - `ValueKind::VOID` - 空类型（用于 void 函数）
    
    类型转换规则：
    - 布尔值可以通过 `zext` 转换为整数（用于逻辑运算的结果）
    - 整数和浮点数之间需要显式转换（当前实现中主要处理整数和浮点数的独立运算）

  - ✅**全局变量处理**：
    
    1. **全局变量声明**：在 `visitDecl()` 中识别全局作用域的变量声明，创建全局变量（`GlobalVariable`）
    2. **全局变量访问**：在 `evalLVal()` 中识别全局变量，直接使用全局变量地址
    3. **全局变量赋值**：在 `handleAssignment()` 中处理全局变量的赋值操作

  - ✅**函数处理**：
    
    1. **函数注册**：`registerFunctions()` 先遍历语法树，注册所有函数（包括参数类型）
    2. **函数定义**：`visitFuncDef()` 创建函数对象，设置参数，生成函数体
    3. **函数调用**：支持标准库函数调用（当前实现主要处理 main 函数，函数调用功能可扩展）

  - ✅**LLVM IR 输出**：
    
    使用提供的 `Module` 类生成符合 LLVM IR 规范的代码：
    
    1. **模块初始化**：创建 Module 对象，设置 ModuleID 和 source_filename
    2. **标准库声明**：自动添加所有标准库函数声明
    3. **IR 生成**：通过 IRBuilder 生成指令，Module 自动管理指令编号（`%op编号`）
    4. **格式化输出**：调用 `module_->print()` 生成格式化的 LLVM IR 文本

    > 相关函数：
    >
    > - `IRGenerator::generate()` - IR 生成入口
    > - `Module::print()` - IR 格式化输出

  **实现细节**：
  
  - **Visitor 模式**：每个语法树节点类型（如 `VarDecl`, `FuncDef`, `Stmt`, `Exp` 等）都有对应的 `visit*()` 函数
  - **错误处理**：使用异常处理机制，在生成过程中遇到错误会抛出异常并记录
  - **调试支持**：在关键步骤输出调试信息，便于排查问题（可通过注释关闭）

  > 相关文件：
  >
  > 1. IRGenerator.h / IRGenerator.cpp - 完整的实现
  >
  > 2. complie_ir/include/ - 中端代码框架的头文件
  >
  > 3. Syntax/output/*.ll - 生成的 LLVM IR 文件
  >
  > 4. Syntax/output/comparison_report.md - IR 输出对比分析报告（包含格式检查和问题分析）

## 配置与自定义

### 修改语法分析器处理的文件

**语法分析器每次只能处理一个文件**

编辑 `Syntax/main.cpp`，找到以下行：

```cpp
string lexFilename = "../lex/output/lex1.txt";
```

修改为你需要的文件名，例如：
```cpp
string lexFilename = "../lex/output/lex2.txt";
```

### 添加新的测试用例

1. **添加源文件**：
   - 在 `lex/test/` 目录下创建新的 `.sy` 文件（如 `test5.sy`）

2. **修改词法分析器**：
   - 编辑 `lex/Lex_Analysis.cpp` 中的 `lexical_analysis()` 函数
   
   - 添加新文件的处理：`lexcial("test/test5.sy", minDFA, "5");`
   
   - 进行词法分析：
   
     ```powershell
     ##命令
     1.cd lex  
     
     2.g++ Lex_Analysis.cpp -o Lex_Analysis  
     
     3../Lex_Analysis   
     ```
   
     或者干脆用bat程序批处理就行。
   
3. **修改语法分析器**：
   - 编辑 `Syntax/main.cpp` 中的 `lexFilename` 变量
   
   - 进行语法分析和IR生成：
   
     ```
     1.cd Syntax
     
     2.g++ main.cpp GrammarAnalyzer.cpp SLRTable.cpp Parser.cpp ReductionSequenceLogger.cpp IRGenerator.cpp -I../complie_ir/include -L../complie_ir -o compiler_ir
     
     3../compiler_ir
     ```
   
     会生成对应的lex*的syntax，out，tree，以及.ll文件。**一次只能跑一个测试用例**。

## 清理构建产物

### Windows

```batch
cd lex
del Lex_Analysis.exe
cd ..\Syntax
del compiler_ir.exe
del compiler_test.exe
cd ..
```

### Linux/Mac

```bash
make clean
```

## 联系与支持

如有问题，请检查：
1. 编译错误信息
2. 输出日志文件（`*out.txt`）
3. 源代码中的注释和文档

---

**最后更新**：2025年12月（已补全中间代码生成部分）



