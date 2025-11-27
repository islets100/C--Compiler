### 适配词法定义的 CFG 文法列表 (已展开 EBNF)



#### 0. 起始规则





```
Program -> compUnit
```



#### 1. 编译单元 (compUnit)



 

```
compUnit -> UnitList
UnitList -> UnitList Unit
UnitList -> ε
Unit     -> decl
Unit     -> funcDef
```



#### 2. 声明 (decl)



 

```
decl -> constDecl
decl -> varDecl
```



#### 3. 常量声明 (constDecl)



 

```
constDecl    -> KW_CONST btype ConstDefList SE_SEMICN
ConstDefList -> constDef
ConstDefList -> ConstDefList SE_COMMA constDef
```



#### 4. 基本类型 (btype)



 

```
btype -> KW_INT
btype -> KW_FLOAT
```



#### 5. 常量定义 (constDef)



 

```
constDef -> ID OP_ASSIGN constInitial
```



#### 6. 常量初值 (constInitial)



 

```
constInitial -> constExp
```



#### 7. 变量声明 (varDecl)



 

```
varDecl    -> btype VarDefList SE_SEMICN
VarDefList -> varDef
VarDefList -> VarDefList SE_COMMA varDef
```



#### 8. 变量定义 (varDef)



 

```
varDef -> ID
varDef -> ID OP_ASSIGN initVal
```



#### 9. 变量初值 (initVal)



 

```
initVal -> exp
```



#### 10. 函数定义 (funcDef) 【关键修改】



**注意**：因为 `main` 是关键字 `<KW, 5>`，它不会被识别为 `ID`。这里必须允许函数名是 `ID` **或者** `KW_MAIN`。

 

```
funcDef        -> funcType FuncName SE_LPAREN OptFuncFParams SE_RPAREN block

FuncName       -> ID
FuncName       -> KW_MAIN 

OptFuncFParams -> funcFParams
OptFuncFParams -> ε
```



#### 11. 函数类型 (funcType)



 

```
funcType -> KW_VOID
funcType -> KW_INT
funcType -> KW_FLOAT
```

*(注：原作业文法 funcType 只有 void/int，但词法有 float 且变量可以定义 float，建议加上 float 支持，或者严格遵循作业文法仅 void/int)*



#### 12. 函数形参 (funcFParams)



 

```
funcFParams -> funcFParam
funcFParams -> funcFParams SE_COMMA funcFParam
```



#### 13. 单个形参 (funcFParam)



 

```
funcFParam -> btype ID
```



#### 14. 语句块 (block)



 

```
block         -> SE_LBRACE BlockItemList SE_RBRACE
BlockItemList -> BlockItemList blockItem
BlockItemList -> ε
```



#### 15. 块列表项 (blockItem)



 

```
blockItem -> decl
blockItem -> stmt
```



#### 16. 语句 (stmt)



*包含 If-Else 冲突处理点*

 

```
stmt -> ID OP_ASSIGN exp SE_SEMICN
stmt -> OptExp SE_SEMICN
stmt -> block
stmt -> KW_RETURN OptExp SE_SEMICN
stmt -> KW_IF SE_LPAREN cond SE_RPAREN stmt
stmt -> KW_IF SE_LPAREN cond SE_RPAREN stmt KW_ELSE stmt

OptExp -> exp
OptExp -> ε
```



#### 17. 表达式 (exp) & 条件 (cond)



 

```
exp  -> addExp
cond -> lOrExp
```



#### 18. 基本表达式 (primaryExp)



 

```
primaryExp -> SE_LPAREN exp SE_RPAREN
primaryExp -> ID
primaryExp -> number
```



#### 19. 数值 (number)



 

```
number -> INT_CONST
number -> FLOAT_CONST
```



#### 20. 一元表达式 (unaryExp)



 

```
unaryExp       -> primaryExp
unaryExp       -> ID SE_LPAREN OptFuncRParams SE_RPAREN
unaryExp       -> unaryOp unaryExp
```

*(注：这里调用函数时，普通函数名是 ID。题目没说 main 可以被递归调用，通常 main 不会被显式调用，所以这里只写 ID 即可。如果允许递归调用 main，则需要类似 funcDef 那样修改)*

 

```
OptFuncRParams -> funcRParams
OptFuncRParams -> ε
```



#### 21. 单目运算符 (unaryOp)



 

```
unaryOp -> OP_PLUS
unaryOp -> OP_MINUS
unaryOp -> OP_AND     
```

*(注：如果测试用例有 `!a`，你需要补全词法器的 OP 表。)*



#### 22. 函数实参 (funcRParams)



 

```
funcRParams -> exp
funcRParams -> funcRParams SE_COMMA exp
```



#### 23. 乘除模表达式 (mulExp)



 

```
mulExp -> unaryExp
mulExp -> mulExp OP_MUL unaryExp
mulExp -> mulExp OP_DIV unaryExp
mulExp -> mulExp OP_MOD unaryExp
```



#### 24. 加减表达式 (addExp)



 

```
addExp -> mulExp
addExp -> addExp OP_PLUS mulExp
addExp -> addExp OP_MINUS mulExp
```



#### 25. 关系表达式 (relExp)



 

```
relExp -> addExp
relExp -> relExp OP_LT addExp
relExp -> relExp OP_GT addExp
relExp -> relExp OP_LE addExp
relExp -> relExp OP_GE addExp
```



#### 26. 相等性表达式 (eqExp)



 

```
eqExp -> relExp
eqExp -> eqExp OP_EQ relExp
eqExp -> eqExp OP_NEQ relExp
```



#### 27. 逻辑与 (lAndExp)



 

```
lAndExp -> eqExp
lAndExp -> lAndExp OP_AND eqExp
```



#### 28. 逻辑或 (lOrExp)



 

```
lOrExp -> lAndExp
lOrExp -> lOrExp OP_OR lAndExp
```



#### 29. 常量表达式 (constExp)



```
constExp -> addExp
```

------



