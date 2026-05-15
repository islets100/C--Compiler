
# 编译原理大作业

**Deadline**：2025年12月5日 11:59 PM (GMT+8)

---

## 一、目标

### 1. 背景知识介绍

C-- 语⾔是本实验的源语⾔。是⼀个 c 语⾔的⼦集，C-- 语⾔是单⽂件的，以 .sy 作
为后缀，去除了 C 语⾔中的 include/define/pointer/struct 等较复杂特性。
LLVM 是⼀个模块化的、可重⽤的编译器和⼯具链的集合，⽬的是提供⼀个现代的、基
于 SSA 的、能够⽀持任意静态和动态编译的编程语⾔的编译策略。在最近⼏年已经成为表
现上能够和 gcc 对标的项⽬。
LLVM IR 是 LLVM 项⽬中通⽤的中间代码，作为源语⾔和体系架构的连接部分，是学
⽣需要从源语⾔中编译并翻译到的⽬标语⾔。

- **C--语言**：C 语言的子集，单文件，后缀为 `.sy`，去除了 `include`、`define`、`pointer`、`struct` 等复杂特性。
- **LLVM**：模块化、可重用的编译器和工具链集合，支持现代 SSA 编译策略。
- **LLVM IR**：LLVM 中间代码，连接源语言与目标架构。

### 2. 大作业要求
本次⼤作业要求编写⼀个编译器前端（包括词法分析器【可直接复用平时上机实验课程
内容】、语法分析器【要求 SLR 算法】、语义分析及中间代码⽣成）。
（1）【必做任务】使⽤⾃动机理论编写词法分析器【如平时实验已完成可复⽤】
（2）【必做任务】⾃上⽽下或者⾃下⽽上的语法分析⽅法编写语法分析器。
(3) 【必做任务】补充完成中间代码⽣成部分代码。
1、【必做任务】编写 C--语⾔的词法分析器，理解词法分析器的⼯作原理，熟练掌握基
于⾃动机理论的词法分析器的⼯作流程。编写源代码识别输出单词的⼆元属性，填写符号表。
2、【必做任务】编写 C--语⾔的语法分析器，理解⾃上⽽下/⾃下⽽上的语法分析算法
的⼯作原理；理解词法分析与语法分析之间的关系。语法分析器的输⼊为 C--语⾔源代码，
输出为按扫描顺序进⾏推导或归约的正确/错误的判别结果，以及按照最左推导顺序/规范规
约顺序⽣成语法树所⽤的产⽣式序列。
3、【必做任务】补全给出的中间代码⽣成部分代码。将编译器的前端与我们提供的编译
器中端衔接，该部分需要遍历语法分析器⽣成的语法树，访问语法树结点并调⽤我们所提供
的中端代码(⻅附录 2.2)，最终输出中间代码。


即：
完成一个编译器前端，包括：

1. **词法分析器**（必做）  
   - 基于自动机理论  
   - 输出单词二元属性，填写符号表

2. **语法分析器**（必做）  
   - 使用 SLR 算法  
   - 输出推导/归约序列和语法树产生式

3. **中间代码生成**（必做）  
   - 补全代码，遍历语法树，调用中端代码生成 LLVM IR

---

## 二、软件需求

### 1. 词法分析器

- **输入**：C-- 源代码  
- **输出**：单词符号序列 + 符号表  
- **单词类型**：
  - 关键字（KW，不区分大小写）：`int`, `void`, `return`, `const`, `main`, `float`, `if`, `else`
  - 运算符（OP）：`+`, `-`, `*`, `/`, `%`, `=`, `>`, `<`, `==`, `<=`, `>=`, `!=`, `&&`, `||`
  - 界符（SE）：`{`, `}`, `(`, `)`, `;`, `,`
  - 标识符（IDN）：字母/数字/下划线，不以数字开头
  - 整数（INT）：数字串
  - 浮点数（FLOAT）：带小数点的数字串

- **实现语言**：C/C++/Java/Python（推荐 C++,，因为中间代码生成部分为c++编写）

### 2. 语法分析器

- **输入**：词法分析输出的单词序列  
- **输出**：归约序列、语法树产生式、错误信息  
- **支持语法功能**：
  - 变量声明与初始化
  - 常量定义
  - 函数定义与参数
  - 表达式与运算
  - 控制结构（if-else, return）

- **SLR 要求**：
  - 构造 LR(0) 项目集规范
  - 计算 FIRST/FOLLOW 集
  - 构造 SLR 分析表
  - 输出分析过程与错误信息

- **实现语言**：C/C++/Java/Python（推荐 C++）

### 3. 中间代码生成

中间代码⽣成器代码补全：对给出的中间代码⽣成器部分代码的缺失部分，即各个
结点的 visitor 函数进⾏补全。针对每个结点构造⼀个 visitor 函数来调⽤中端代码类(⻅附录2.2即工作目录的compiler_ir文件夹下内容)，其中 visit 函数对应于语法分析和中间代码⽣成的衔接部分，完成对语法分析⽣成的抽象语法树的遍历，并通过访问语法树结点来调⽤我们提供的中端代码（⻅附录 2.2），⽣成 LLVM IR 中间代码。
- **输出格式**：`.ll` 格式的 LLVM IR
- **任务**：补全 Visitor 函数，遍历语法树，调用中端代码生成 IR
- **实现语言**：C++

---

## 三、输出示例

### 1. 词法分析输出格式要求及示例

#### 词法规则
C-- 语 言 是 本 实 验 的 源 语 言 ， 是 一 个 C 语 言 的 子 集 ， 去 除 了 C 语 言 中 的include/define/pointer/struct 等较复杂特性。
其单词符号的类型包括关键字、标识符、界符、运算符、整数和浮点数。每种单词符号的具体要求如下：
关键字（KW，不区分大小写）包括：(1) int (2) void (3) return (4) const
(5) main (6) float (7) if (8) else
运算符（OP）包括：(9) + (10) - (11) * (12) / (13) % (14) = (15) > (16) <
(17) == (18) <= (19) >= (20) != (21) && (22) ||
界符（SE）包括：(23) ( (24) ) (25) { (26) } (27) ; (28) ,
标识符（IDN）定义与 C 语言保持相同，为字母、数字和下划线（_）组成的不以数字开头的串。
整数（INT）的定义与 C 语言类似，由数字串表示。
浮点数（FLOAT）的定义与 C 语言类似，由包含一个小数点的数字串表示。
#### 输出格式
单词符号输出格式：
```plaintext
[待测代码中的单词符号][TAB]<[单词符号种别],[单词符号内容]>
```
其中，单词符号种别为 KW（关键字）、OP（运算符）、SE（界符）、IDN（标
识符） INT （整形数）；单词符号内容第一个维度为其种别，第二个维度为其属
性。
当用例程序存在非法符号时，输出错误所在行号和列号：
```plaintext
[非法符号][TAB]<ERROR,[行号],[列号]>
```
#### 例子：
对于
```plaintext
int a = 10;
int main(){
a=10;
return 0;
}
```
解析为
```plaintext
int <KW,1>
a <IDN,a>
= <OP,14>
10 <INT,10>
; <SE,27>
int <KW,1>
main <KW,5>
( <SE,23>
) <SE,24>
{ <SE,25>
a <IDN,a>
= <OP,14>
10 <INT,10>
; <SE,27>
return <KW,3>
0 <INT,0>
; <SE,27>
} <SE,26>
```

### 2. 语法分析输出示例
输出规约序列

输出格式：
```plaintext
[序号] [TAB] [栈顶符号]#[⾯临输⼊符号] [TAB] [执⾏动作]
```
其中，选⽤规则序号⻅附件⽂法规则；执⾏动作为“reduction”（SLR 中的归约），“move”
（SLR 分析的移进），“accept”（接受）或“error”（错误）。

```plaintext
1	program#int	reduction
2	compUnit#int	reduction
3	decI#int	reduction
4	valDecI#int	reduction
5	btype#int	reduction
6	int#int	move
7	varDef#a	reduction
8	Ident#a	move
9	-#=	move
10	initVal#10	reduction
```
注：分析栈左端为栈顶，输⼊串过⻓没有在输出实例中进⾏展示，输⼊串即词法分析器⽣
成的单词符号序列。
### 3. 中间代码输出示例

```llvm
; ModuleID = 'sysy2022_complier'
source_filename = "./input/01_var_defn.sy"
@a = global i32 0
declare i32 @getinit()
declare i32 @getch()
declare i32 @getarray(i32^)
declare void @putinit(i32)
declare void @putch(i32)
declare void @putarray(i32, i32^)
declare void @starttime()
declare void @stoptime()

define i32 @main() {
main_ENTRY:
    %op0 = load i32, i32^ @a
    store i32 %op0, i32 10
    ret i32 0
}
```

---

## 四、提交要求

1. **源代码**：词法分析器、语法分析器、中间代码生成部分
2. **开发报告**：
   - 词法/语法分析算法描述
   - SLR 分析表
   - 语法树遍历与中端调用过程
3. **测试报告**：输出截图（按格式要求）

---

## 五、注意事项

- 使用附录中的文法
- 禁止使用 Lex/Yacc、ANTLR 等自动生成工具

---

## 附录

### 1. 文法文件

```plaintext
1. Program -> compUnit
2. compUnit -> (decl | funcDef)* EOF
3. decl -> constDecl | varDecl
4. constDecl -> 'const' btype constDef (',' constDef)* ';'
5. btype -> 'int' | 'float'
6. constDef -> Ident '=' constInitial
7. constInitial -> constExp
8. varDecl -> btype varDef (',' varDef)* ';'
9. varDef -> Ident | Ident '=' initVal
10. initVal -> exp
11. funcDef -> funcType Ident '(' (funcFParams)? ')' block
12. funcType -> 'void' | 'int'
13. funcFParams -> funcFParam (',' funcFParam)*
14. funcFParam -> bType Ident
15. block -> '{' (blockItem)* '}'
16. blockItem -> decl | stmt
17. stmt -> lVal '=' exp ';' | (exp)? ';' | block | 'if' '(' cond ')' stmt ('else' stmt)? | 'return' (exp)? ';'
18. exp -> addExp
19. cond -> lOrExp
20. lVal -> Ident
21. primaryExp -> '(' exp ')' | lVal | number
22. number -> IntConst | floatConst
23. unaryExp -> primaryExp | Ident '(' (funcRParams)? ')' | unaryOp unaryExp
24. unaryOp -> '+' | '-' | '!'
25. funcRParams -> funcRParam (',' funcRParam)*
26. funcRParam -> exp
27. mulExp -> unaryExp | mulExp ('*' | '/' | '%') unaryExp
28. addExp -> mulExp | addExp ('+' | '-') mulExp
29. relExp -> addExp | relExp ('<' | '>' | '<=' | '>=') addExp
30. eqExp -> relExp | eqExp ('==' | '!=') relExp
31. lAndExp -> eqExp | lAndExp '&&' eqExp
32. lOrExp -> lAndExp | lOrExp '||' lAndExp
33. constExp -> addExp
34. IntConst -> [0-9]+
35. Ident -> [a-zA-Z_][a-zA-Z_0-9]*
36. floatConst -> [0-9]+ '.' [0-9]+
```

### 2. LLVM 相关内容

- [LLVM 语法格式介绍](https://llvm.org/docs/LangRef.html)
- [中端代码链接](https://qitee.com/happy-traveller/compiler_ir)

