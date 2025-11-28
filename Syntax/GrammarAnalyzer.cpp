/*
 *                                |~~~~~~~|
 *                                |       |
 *                                |       |
 *                                |       |
 *                                |       |
 *                                |       |
 *     |~.\\\_\~~~~~~~~~~~~~~xx~~~         ~~~~~~~~~~~~~~~~~~~~~/_//;~|
 *     |  \  o \_         ,XXXXX),                         _..-~ o /  |
 *     |    ~~\  ~-.     XXXXX`)))),                 _.--~~   .-~~~   |
 *      ~~~~~~~`\   ~\~~~XXX' _/ ';))     |~~~~~~..-~     _.-~ ~~~~~~~
 *               `\   ~~--`_\~\, ;;;\)__.---.~~~      _.-~
 *                 ~-.       `:;;/;; \          _..-~~
 *                    ~-._      `''        /-~-~
 *                        `\              /  /
 *                          |         ,   | |
 *                           |  '        /  |
 *                            \/;          |
 *                             ;;          |
 *                             `;   .       |
 *                             |~~~-----.....|
 *                            | \             \
 *                           | /\~~--...__    |
 *                           (|  `\       __-\|
 *                           ||    \_   /~    |
 *                           |)     \~-'      |
 *                            |      | \      '
 *                            |      |  \    :
 *                             \     |  |    |
 *                              |    )  (    )
 *                               \  /;  /\  |
 *                               |    |/   |
 *                               |    |   |
 *                                \  .'  ||
 *                                |  |  | |
 *                                (  | |  |
 *                                |   \ \ |
 *                                || o `.)|
 *                                |`\\) |
 *                                |       |
 *                                |       |
 * 
 * @Description    : grammar analyzer part of the compiler  
 * @Auther         : Ren Xiaohua
 * @LastEditors    : Ren Xiaohua
 * @Date           : 2025-11-28 19:20:53
 * @LastEditTime   : 2025-11-28 19:20:53
 * Copyright 2025 Ren Xiaohua, All Rights Reserved. 
 */
#include "GrammarAnalyzer.h"
#include <iomanip>

// ==========================================
// 辅助函数
// ==========================================
bool GrammarAnalyzer::isTerminal(int sym) {
    return sym < 50; 
}

string GrammarAnalyzer::getSymbolName(int id) {
    if (id == SYM_EPSILON) return "eps";
    if (id == SYM_EOF) return "$";
    if (id == SYM_ERROR) return "ERROR";
    
    switch(id) {
        case SYM_ID: return "ID";
        case SYM_INT_CONST: return "INT";
        case SYM_FLOAT_CONST: return "FLOAT";
        case SYM_KW_INT: return "int";
        case SYM_KW_VOID: return "void";
        case SYM_KW_RETURN: return "return";
        case SYM_KW_CONST: return "const";
        case SYM_KW_MAIN: return "main";
        case SYM_KW_FLOAT: return "float";
        case SYM_KW_IF: return "if";
        case SYM_KW_ELSE: return "else";
        case SYM_OP_PLUS: return "+";
        case SYM_OP_MINUS: return "-";
        case SYM_OP_MUL: return "*";
        case SYM_OP_DIV: return "/";
        case SYM_OP_MOD: return "%";
        case SYM_OP_ASSIGN: return "=";
        case SYM_OP_GT: return ">";
        case SYM_OP_LT: return "<";
        case SYM_OP_EQ: return "==";
        case SYM_OP_LE: return "<=";
        case SYM_OP_GE: return ">=";
        case SYM_OP_NEQ: return "!=";
        case SYM_OP_AND: return "&&";
        case SYM_OP_OR: return "||";
        case SYM_SE_LPAREN: return "(";
        case SYM_SE_RPAREN: return ")";
        case SYM_SE_LBRACE: return "{";
        case SYM_SE_RBRACE: return "}";
        case SYM_SE_SEMICN: return ";";
        case SYM_SE_COMMA: return ",";
    }
    
    // 非终结符名称
    if (id >= 50) {
        static map<int, string> names = {
            {NON_PROGRAM, "Program"}, {NON_COMPUNIT, "CompUnit"}, {NON_UNITLIST, "UnitList"}, {NON_UNIT, "Unit"},
            {NON_DECL, "Decl"}, {NON_CONSTDECL, "ConstDecl"}, {NON_VARDECL, "VarDecl"},
            {NON_CONSTDEFLIST, "ConstDefList"}, {NON_BTYPE, "BType"}, {NON_CONSTDEF, "ConstDef"}, {NON_CONSTINITIAL, "ConstInit"},
            {NON_VARDEFLIST, "VarDefList"}, {NON_VARDEF, "VarDef"}, {NON_INITVAL, "InitVal"},
            {NON_FUNCDEF, "FuncDef"}, {NON_FUNCNAME, "FuncName"}, {NON_OPTFUNCFPARAMS, "OptFParams"}, {NON_FUNCTYPE, "FuncType"},
            {NON_FUNCFPARAMS, "FuncFParams"}, {NON_FUNCFPARAM, "FuncFParam"},
            {NON_BLOCK, "Block"}, {NON_BLOCKITEMLIST, "BlockItemList"}, {NON_BLOCKITEM, "BlockItem"},
            {NON_STMT, "Stmt"}, {NON_OPTEXP, "OptExp"},
            {NON_EXP, "Exp"}, {NON_COND, "Cond"}, {NON_LVAL, "LVal"}, {NON_PRIMARYEXP, "PrimaryExp"}, {NON_NUMBER, "Number"},
            {NON_UNARYEXP, "UnaryExp"}, {NON_OPTFUNCRPARAMS, "OptRParams"}, {NON_UNARYOP, "UnaryOp"},
            {NON_FUNCRPARAMS, "FuncRParams"},
            {NON_MULEXP, "MulExp"}, {NON_ADDEXP, "AddExp"},
            {NON_RELEXP, "RelExp"}, {NON_EQEXP, "EqExp"}, {NON_LANDEXP, "LAndExp"}, {NON_LOREXP, "LOrExp"},
            {NON_CONSTEXP, "ConstExp"}
        };
        if (names.count(id)) return names[id];
        return "NT_" + to_string(id);
    }
    return "?";
}

bool GrammarAnalyzer::mergeSet(set<int>& target, const set<int>& source, bool excludeEpsilon) {
    int originalSize = target.size();
    for (int s : source) {
        if (excludeEpsilon && s == SYM_EPSILON) continue;
        target.insert(s);
    }
    return target.size() > originalSize;
}

// ==========================================
// 核心算法: FIRST 和 FOLLOW
// ==========================================
void GrammarAnalyzer::buildFirst() {
    for (int i = 0; i < 100; i++) {
        if (isTerminal(i)) firstSet[i].insert(i);
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : productions) {
            int X = prod.left;
            bool allDeriveEpsilon = true;
            
            // 空产生式
            if (prod.right.empty() || (prod.right.size() == 1 && prod.right[0] == SYM_EPSILON)) {
                 if (firstSet[X].find(SYM_EPSILON) == firstSet[X].end()) {
                    firstSet[X].insert(SYM_EPSILON);
                    changed = true;
                }
                continue;
            }

            for (int Y : prod.right) {
                if (mergeSet(firstSet[X], firstSet[Y], true)) changed = true;
                if (firstSet[Y].find(SYM_EPSILON) == firstSet[Y].end()) {
                    allDeriveEpsilon = false;
                    break;
                }
            }
            if (allDeriveEpsilon) {
                if (firstSet[X].find(SYM_EPSILON) == firstSet[X].end()) {
                    firstSet[X].insert(SYM_EPSILON);
                    changed = true;
                }
            }
        }
    }
}

// GrammarAnalyzer.cpp

void GrammarAnalyzer::buildFollow() {
    // 1. 标准算法 (计算基础 FOLLOW 集)
    followSet[NON_PROGRAM].insert(SYM_EOF);

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& prod : productions) {
            int A = prod.left;
            const vector<int>& right = prod.right;

            for (size_t i = 0; i < right.size(); i++) {
                int B = right[i];
                if (isTerminal(B) || B == SYM_EPSILON) continue;

                bool betaIsNullable = true;
                for (size_t j = i + 1; j < right.size(); j++) {
                    int beta = right[j];
                    if (mergeSet(followSet[B], firstSet[beta], true)) changed = true;
                    if (firstSet[beta].find(SYM_EPSILON) == firstSet[beta].end()) {
                        betaIsNullable = false;
                        break;
                    }
                }
                if (betaIsNullable) {
                    if (mergeSet(followSet[B], followSet[A], false)) changed = true;
                }
            }
        }
    }

    // =========================================================
    // 【终极强制修复】手动注入逗号和分号
    // SLR(1) 有时无法穿透多层单产生式 (Unit Production) 传播 FOLLOW
    // 这里我们强制规定：所有表达式结尾都可以跟逗号或分号
    // =========================================================
    // vector<int> targets = {
    //     NON_INITVAL, 
    //     NON_EXP, 
    //     NON_ADDEXP, 
    //     NON_MULEXP, 
    //     NON_UNARYEXP, 
    //     NON_PRIMARYEXP, 
    //     NON_NUMBER,
    //     NON_CONSTINITIAL,
    //     NON_CONSTEXP,
    //     NON_VARDEF,   // 必须告诉编译器：变量定义完成后，后面可以跟逗号！
    //     NON_CONSTDEF  // 常量定义同理
    // };

    // for (int nt : targets) {
    //     followSet[nt].insert(SYM_SE_COMMA);  // 允许逗号
    //     followSet[nt].insert(SYM_SE_SEMICN); // 允许分号
    //     followSet[nt].insert(SYM_SE_RPAREN); // 允许右括号 (用于函数参数)
    // }
}

void GrammarAnalyzer::printSets() {
    cout << "\n===== FIRST SETS =====\n";
    for (auto const& [key, val] : firstSet) {
        if (isTerminal(key)) continue; 
        cout << left << setw(15) << getSymbolName(key) << ": { ";
        for (int s : val) cout << getSymbolName(s) << " ";
        cout << "}" << endl;
    }

    cout << "\n===== FOLLOW SETS =====\n";
    for (auto const& [key, val] : followSet) {
        if (isTerminal(key)) continue;
        cout << left << setw(15) << getSymbolName(key) << ": { ";
        for (int s : val) cout << getSymbolName(s) << " ";
        cout << "}" << endl;
    }
}

// ==========================================
// 文法初始化 (完整版)
// ==========================================
void GrammarAnalyzer::initGrammar() {
    int pid = 1;
    // 简化添加函数
    auto add = [&](int left, vector<int> right) {
        productions.push_back({pid++, left, right});
    };

    // 0. Program -> compUnit
    add(NON_PROGRAM, {NON_COMPUNIT});

    // 1. compUnit -> UnitList (处理 Kleene *)
    add(NON_COMPUNIT, {NON_UNITLIST});
    add(NON_UNITLIST, {NON_UNITLIST, NON_UNIT});
    add(NON_UNITLIST, {SYM_EPSILON}); // 空

    // Unit -> decl | funcDef
    add(NON_UNIT, {NON_DECL});
    add(NON_UNIT, {NON_FUNCDEF});

    // 2. decl -> constDecl | varDecl
    add(NON_DECL, {NON_CONSTDECL});
    add(NON_DECL, {NON_VARDECL});

    // 3. constDecl
    add(NON_CONSTDECL, {SYM_KW_CONST, NON_BTYPE, NON_CONSTDEFLIST, SYM_SE_SEMICN});
    
    // ConstDefList -> ConstDef
    add(NON_CONSTDEFLIST, {NON_CONSTDEF});

    // ConstDefList -> ConstDef , ConstDefList  <--- 修改这里
    // 让 ConstDef 后面直接跟逗号，这样 FOLLOW 集计算更准确
    add(NON_CONSTDEFLIST, {NON_CONSTDEF, SYM_SE_COMMA, NON_CONSTDEFLIST});

    // 4. btype
    add(NON_BTYPE, {SYM_KW_INT});
    add(NON_BTYPE, {SYM_KW_FLOAT});

    // 5. constDef -> Ident = constInit
    add(NON_CONSTDEF, {SYM_ID, SYM_OP_ASSIGN, NON_CONSTINITIAL});

    // 6. constInitial
    add(NON_CONSTINITIAL, {NON_CONSTEXP});

    // 7. varDecl
    add(NON_VARDECL, {NON_BTYPE, NON_VARDEFLIST, SYM_SE_SEMICN});
    
    // VarDefList -> VarDef
    add(NON_VARDEFLIST, {NON_VARDEF});

    // VarDefList -> VarDef , VarDefList  <--- 修改这里
    add(NON_VARDEFLIST, {NON_VARDEF, SYM_SE_COMMA, NON_VARDEFLIST});

    // 8. varDef
    add(NON_VARDEF, {SYM_ID});
    add(NON_VARDEF, {SYM_ID, SYM_OP_ASSIGN, NON_INITVAL});

    // 9. initVal
    add(NON_INITVAL, {NON_EXP});

    // 10. funcDef (解决 main 是关键字的问题)
    //add(NON_FUNCDEF, {NON_FUNCTYPE, NON_FUNCNAME, SYM_SE_LPAREN, NON_OPTFUNCFPARAMS, SYM_SE_RPAREN, NON_BLOCK});
    // 情况 1: void 类型的函数 (使用 FuncType)
    add(NON_FUNCDEF, {NON_FUNCTYPE, NON_FUNCNAME, SYM_SE_LPAREN, NON_OPTFUNCFPARAMS, SYM_SE_RPAREN, NON_BLOCK});
    // 情况 2: int/float 类型的函数 (直接使用 BTYPE，避免归约冲突)
    add(NON_FUNCDEF, {NON_BTYPE, NON_FUNCNAME, SYM_SE_LPAREN, NON_OPTFUNCFPARAMS, SYM_SE_RPAREN, NON_BLOCK});

    // FuncName -> ID | main
    add(NON_FUNCNAME, {SYM_ID});
    add(NON_FUNCNAME, {SYM_KW_MAIN});

    // OptParams
    add(NON_OPTFUNCFPARAMS, {NON_FUNCFPARAMS});
    add(NON_OPTFUNCFPARAMS, {SYM_EPSILON});

    // 11. funcType
    add(NON_FUNCTYPE, {SYM_KW_VOID});
    //add(NON_FUNCTYPE, {SYM_KW_INT});
    //add(NON_FUNCTYPE, {SYM_KW_FLOAT});

    // 12. funcFParams
    add(NON_FUNCFPARAMS, {NON_FUNCFPARAM});
    add(NON_FUNCFPARAMS, {NON_FUNCFPARAMS, SYM_SE_COMMA, NON_FUNCFPARAM});
    
    // 13. funcFParam
    add(NON_FUNCFPARAM, {NON_BTYPE, SYM_ID});

    // 14. block
    add(NON_BLOCK, {SYM_SE_LBRACE, NON_BLOCKITEMLIST, SYM_SE_RBRACE});
    add(NON_BLOCKITEMLIST, {NON_BLOCKITEMLIST, NON_BLOCKITEM});
    add(NON_BLOCKITEMLIST, {SYM_EPSILON});

    // 15. blockItem
    add(NON_BLOCKITEM, {NON_DECL});
    add(NON_BLOCKITEM, {NON_STMT});

    // 16. stmt (包含所有语句类型)
    // lVal = exp ;
    add(NON_STMT, {NON_LVAL, SYM_OP_ASSIGN, NON_EXP, SYM_SE_SEMICN});
    // (exp)? ;
    add(NON_STMT, {NON_OPTEXP, SYM_SE_SEMICN});
    // block
    add(NON_STMT, {NON_BLOCK});
    // return (exp)? ;
    add(NON_STMT, {SYM_KW_RETURN, NON_OPTEXP, SYM_SE_SEMICN});
    // if (cond) stmt
    add(NON_STMT, {SYM_KW_IF, SYM_SE_LPAREN, NON_COND, SYM_SE_RPAREN, NON_STMT});
    // if (cond) stmt else stmt
    add(NON_STMT, {SYM_KW_IF, SYM_SE_LPAREN, NON_COND, SYM_SE_RPAREN, NON_STMT, SYM_KW_ELSE, NON_STMT});

    // OptExp
    add(NON_OPTEXP, {NON_EXP});
    add(NON_OPTEXP, {SYM_EPSILON});

    // ==========================================
    // 表达式部分 (完整补全)
    // ==========================================

    // // 17. exp -> addExp (注意: 只有算术)
    // add(NON_EXP, {NON_ADDEXP});
    // 17. exp -> lOrExp (修改为顶层表达式，从而支持括号内的逻辑/关系运算)
    add(NON_EXP, {NON_LOREXP});
    
    // 19. cond -> lOrExp (用于 if 条件，包含逻辑运算)
    add(NON_COND, {NON_LOREXP});

    // 20. lVal
    add(NON_LVAL, {SYM_ID});

    // 21. primaryExp
    add(NON_PRIMARYEXP, {SYM_SE_LPAREN, NON_EXP, SYM_SE_RPAREN});
    add(NON_PRIMARYEXP, {NON_LVAL});
    add(NON_PRIMARYEXP, {NON_NUMBER});

    // 22. number
    add(NON_NUMBER, {SYM_INT_CONST});
    add(NON_NUMBER, {SYM_FLOAT_CONST});

    // 23. unaryExp
    add(NON_UNARYEXP, {NON_PRIMARYEXP});
    add(NON_UNARYEXP, {SYM_ID, SYM_SE_LPAREN, NON_OPTFUNCRPARAMS, SYM_SE_RPAREN}); // 函数调用
    add(NON_UNARYEXP, {NON_UNARYOP, NON_UNARYEXP});

    // OptFuncRParams
    add(NON_OPTFUNCRPARAMS, {NON_FUNCRPARAMS});
    add(NON_OPTFUNCRPARAMS, {SYM_EPSILON});

    // 24. unaryOp (+, -) 
    // 注意: 暂时没有 '!'
    // 如果需要支持非，需添加 SYM_OP_NOT
    add(NON_UNARYOP, {SYM_OP_PLUS});
    add(NON_UNARYOP, {SYM_OP_MINUS});

    // 25/26. funcRParams
    add(NON_FUNCRPARAMS, {NON_EXP});
    add(NON_FUNCRPARAMS, {NON_FUNCRPARAMS, SYM_SE_COMMA, NON_EXP});

    // 27. mulExp (乘除模)
    add(NON_MULEXP, {NON_UNARYEXP});
    add(NON_MULEXP, {NON_MULEXP, SYM_OP_MUL, NON_UNARYEXP});
    add(NON_MULEXP, {NON_MULEXP, SYM_OP_DIV, NON_UNARYEXP});
    add(NON_MULEXP, {NON_MULEXP, SYM_OP_MOD, NON_UNARYEXP});

    // 28. addExp (加减)
    add(NON_ADDEXP, {NON_MULEXP});
    add(NON_ADDEXP, {NON_ADDEXP, SYM_OP_PLUS, NON_MULEXP});
    add(NON_ADDEXP, {NON_ADDEXP, SYM_OP_MINUS, NON_MULEXP});

    // 29. relExp (关系: <, >, <=, >=) - 之前缺失的
    add(NON_RELEXP, {NON_ADDEXP});
    add(NON_RELEXP, {NON_RELEXP, SYM_OP_LT, NON_ADDEXP});
    add(NON_RELEXP, {NON_RELEXP, SYM_OP_GT, NON_ADDEXP});
    add(NON_RELEXP, {NON_RELEXP, SYM_OP_LE, NON_ADDEXP});
    add(NON_RELEXP, {NON_RELEXP, SYM_OP_GE, NON_ADDEXP});

    // 30. eqExp (相等: ==, !=) - 之前缺失的
    add(NON_EQEXP, {NON_RELEXP});
    add(NON_EQEXP, {NON_EQEXP, SYM_OP_EQ, NON_RELEXP});
    add(NON_EQEXP, {NON_EQEXP, SYM_OP_NEQ, NON_RELEXP});

    // 31. lAndExp (逻辑与: &&) - 之前缺失的
    add(NON_LANDEXP, {NON_EQEXP});
    add(NON_LANDEXP, {NON_LANDEXP, SYM_OP_AND, NON_EQEXP});

    // 32. lOrExp (逻辑或: ||) - 之前缺失的
    add(NON_LOREXP, {NON_LANDEXP});
    add(NON_LOREXP, {NON_LOREXP, SYM_OP_OR, NON_LANDEXP});

    // 33. constExp
    add(NON_CONSTEXP, {NON_ADDEXP});
}