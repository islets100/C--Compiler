# LLVM IR 输出对比分析报告

## 一、格式要求（来自 finaltask.md）

根据 finaltask.md 第184-205行的示例，LLVM IR 输出应包含：

1. **模块头部**：
   - `; ModuleID = 'sysy2022_complier'`
   - `source_filename = "./input/XX.sy"`

2. **全局变量声明**（如果有）：
   - `@变量名 = global i32 初始值`

3. **函数声明**（标准库函数）：
   - `declare i32 @getinit()`
   - `declare i32 @getch()`
   - `declare i32 @getarray(i32*)`  ⚠️ 注意：文档示例中使用 `i32^`，但这是错误的，应该是 `i32*`
   - `declare void @putinit(i32)`
   - `declare void @putch(i32)`
   - `declare void @putarray(i32, i32*)`  ⚠️ 同上
   - `declare void @starttime()`
   - `declare void @stoptime()`

4. **函数定义**：
   - `define i32 @main() {` 或 `define void @main() {`
   - 基本块标签：`main_ENTRY:`
   - 指令格式：`%op编号 = 操作码 类型, 操作数...`

---

## 二、各文件详细对比

### 1. 1.ll 文件对比

**源文件**：`lex/test/test1.sy`
```c
int main ( ) {
  int a = 1 , b = 2 ;
  a = a + b - 1 ;
  if ( a == 2 ) {
    return 0 ;
  } else {
    b = b * 2 / 1 % 2 ;
    return 1 ;
  }
}
```

**生成的IR**：✅ **基本正确**

**检查项**：
- ✅ 模块头部格式正确
- ✅ 函数声明顺序和格式正确（使用 `i32*` 而非文档中的 `i32^`，这是正确的）
- ✅ main函数定义为 `define i32 @main()`，正确
- ✅ 基本块标签 `main_ENTRY:` 正确
- ✅ 变量分配 `alloca` 正确
- ✅ 算术运算指令正确（add, sub, mul, sdiv, srem）
- ✅ 比较指令 `icmp eq` 正确
- ✅ 分支指令 `br` 正确
- ✅ return语句正确

**潜在问题**：无

---

### 2. 2.ll 文件对比

**源文件**：`lex/test/test2.sy`
```c
void main ( ) {
  const int N = 10 , M = 2 ;
  float rate = 0.5 , base = 3.14 ;
  int x = 0 , y = 1 ;
  x = x + N - M * 2 / 1 % 2 ;
  if ( ( x >= y && rate != 0.0 ) || ( N < M ) ) {
    y = ( y + 1 ) ;
  } else {
    y = ( y - 1 ) ;
  }
  x = x > y ;
  x = x < y ;
  x = ( x == y ) ;
  x = ( x <= y ) ;
  x = ( x >= y ) ;
  return ;
}
```

**生成的IR**：⚠️ **存在一些问题**

**检查项**：
- ✅ 模块头部格式正确
- ✅ 函数声明正确
- ✅ main函数定义为 `define void @main()`，正确（因为源文件是void main）
- ✅ 基本块标签正确
- ✅ 常量处理：const变量被当作普通局部变量处理（这是合理的，因为const在运行时也是变量）
- ✅ 浮点数处理正确（float类型）
- ✅ 逻辑运算处理：使用 `zext` 将布尔值转换为i32，然后进行整数运算，这是正确的
- ✅ 比较运算处理正确（icmp sge, icmp slt, icmp eq, icmp sle, icmp sge）
- ✅ 浮点数比较 `fcmp one` 正确

**潜在问题**：
1. ⚠️ **const变量处理**：const int N和M被当作普通局部变量，而不是编译时常量。这在语义上是合理的（因为需要支持运行时const），但可能不符合某些实现要求。
2. ✅ **逻辑运算实现**：`&&` 和 `||` 通过整数运算实现（zext + mul/add + icmp ne），这是正确的实现方式。

---

### 3. 3.ll 文件对比

**源文件**：`lex/test/test3.sy`
```c
VoID MaIn ( ) {
  CoNsT InT _X1 = 42 ;
  int y = 0 ;
  FLOAT Pi = 3.14 , rate = 0.5 ;
  int A = 1 , B = 2 ;
  y = y + _X1 * 2 - 5 / 3 % 2 ;
  A = A + B ;
  if ( ( y >= 10 && y != 0 ) || ( A < B ) ) {
      y = y + 1 ;
  } else {
      y = y - 1 ;
  }
  if ( Pi > 1.0 && rate <= 1.0 ) {
      A = ( A + B ) * ( _X1 - 3 ) ;
  } else {
      if ( A == B ) { B = B + 1 ; } else { B = B - 1 ; }
  }
  return ;
}
```

**生成的IR**：⚠️ **存在一些问题**

**检查项**：
- ✅ 模块头部格式正确
- ✅ 函数声明正确
- ✅ main函数定义为 `define void @main()`，正确
- ✅ 基本块标签正确
- ✅ 嵌套if-else处理：存在多个if_then和if_else标签，这是正确的
- ✅ 浮点数比较处理正确

**潜在问题**：
1. ⚠️ **标签重复**：在嵌套的if-else中，出现了多个 `if_then:` 和 `if_else:` 标签。虽然这在LLVM IR中是合法的（每个基本块都有唯一标签），但可能会导致混淆。更好的做法是使用唯一的标签名（如 `if_then1:`, `if_then2:`）。
2. ⚠️ **if_merge标签缺失**：在嵌套的if-else结构中，某些分支可能缺少 `if_merge:` 标签，导致控制流不完整。查看代码：
   - 第89行有 `if_merge:`，但第74行的if-else没有对应的merge标签
   - 第101行有 `if_merge:`，但第88行的if-else没有对应的merge标签
   - 第102行有 `br label %if_merge`，但这是跳转到自己的merge，可能导致无限循环

**严重问题**：
1. ❌ **标签重复**：存在多个同名的 `if_then:` 和 `if_else:` 标签：
   - 第55行：`if_then:`（第一个if-else）
   - 第60行：`if_else:`（第一个if-else）
   - 第75行：`if_then:`（第二个if-else，**重复！**）
   - 第84行：`if_else:`（第二个if-else，**重复！**）
   - 第88行：`br i1 %op57, label %if_then, label %if_else`（会跳转到错误的标签）
   - 第91行：`if_then:`（第三个if-else，**重复！**）
   - 第96行：`if_else:`（第三个if-else，**重复！**）
   
   在LLVM IR中，每个基本块标签必须唯一。重复的标签会导致未定义行为。

2. ❌ **控制流错误**：
   - 第74行：`br i1 %op46, label %if_then, label %if_else` - 这会错误地跳转到第55行的if_then（属于第一个if-else），而不是第75行的if_then（属于第二个if-else）
   - 第88行：`br i1 %op57, label %if_then, label %if_else` - 这会错误地跳转到第75行的if_then（属于第二个if-else），而不是第91行的if_then（属于第三个if-else）
   - 第102行：`br label %if_merge` - 跳转到第101行的if_merge，形成**无限循环**

3. ❌ **结构问题**：
   - 第一个if-else（第54-64行）：✅ 正确
   - 第二个if-else（第74-89行）：❌ 标签重复，控制流错误
   - 第三个if-else（第88-100行）：❌ 嵌套在第二个if-else的else分支中，但标签命名错误
   - 第101-102行：❌ 多余的if_merge标签和无限循环

---

### 4. 4.ll 文件对比

**源文件**：`lex/test/test4.sy`
```c
int a = 10;
int main(){
      a = 10;
return 0;
}
```

**生成的IR**：✅ **基本正确**

**检查项**：
- ✅ 模块头部格式正确
- ✅ 全局变量声明 `@a = global i32 10` 正确
- ✅ 函数声明正确
- ✅ main函数定义为 `define i32 @main()`，正确
- ✅ 基本块标签正确
- ✅ 全局变量赋值 `store i32 10, i32* @a` 正确
- ✅ return语句正确

**潜在问题**：
1. ⚠️ **全局变量初始化**：全局变量 `@a = global i32 10` 在声明时已经初始化为10，但在main函数中又执行了 `store i32 10, i32* @a`。这在语义上是正确的（重新赋值），但可能有些冗余。不过，如果源文件明确写了 `a = 10;`，那么生成store指令是正确的。

---

## 三、与文档示例的差异

### 1. 指针类型表示
- **文档示例**：使用 `i32^`（可能是文档错误）
- **实际生成**：使用 `i32*`（正确的LLVM IR语法）
- **结论**：✅ 生成的代码是正确的，文档可能有误

### 2. store指令格式
- **文档示例**：`store i32 %op0, i32 10`（这是错误的语法）
- **实际生成**：`store i32 10, i32* @a`（正确的语法）
- **结论**：✅ 生成的代码是正确的，文档示例有误

---

## 四、总结

### ✅ 完全正确的文件
1. **1.ll** - 所有指令和格式都正确
2. **4.ll** - 所有指令和格式都正确

### ⚠️ 存在问题的文件
1. **2.ll** - 基本正确，但const变量处理可能需要确认是否符合要求
2. **3.ll** - **存在严重的控制流错误**：
   - 嵌套if-else的标签命名可能不够清晰
   - 第102行的 `br label %if_merge` 可能导致无限循环

### 🔧 需要修复的问题

#### 优先级1（严重）：
1. **3.ll 的控制流错误**：修复嵌套if-else的merge标签跳转逻辑

#### 优先级2（改进）：
1. **3.ll 的标签命名**：为嵌套的if-else使用唯一的标签名
2. **2.ll 的const处理**：确认const变量的处理方式是否符合要求

---

## 五、建议

1. **修复3.ll的控制流问题**：这是最严重的问题，需要立即修复
2. **改进标签命名**：为嵌套结构使用更清晰的标签名
3. **验证const处理**：确认const变量的处理是否符合实验要求
4. **文档问题**：finaltask.md中的示例有语法错误（`i32^` 和 `store` 指令格式），但生成的代码是正确的

