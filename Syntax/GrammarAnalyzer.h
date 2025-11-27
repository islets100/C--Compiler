#ifndef GRAMMAR_ANALYZER_H
#define GRAMMAR_ANALYZER_H

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <algorithm>

using namespace std;

// ==========================================
// 1. 符号定义 (包含终结符和非终结符)
// ==========================================
enum SymbolType {
    // === 0-49: 终结符 (Terminals) ===
    SYM_EPSILON = 0, // ε
    SYM_EOF,         // $
    SYM_ERROR,
    
    // 标识符与常量
    SYM_ID, SYM_INT_CONST, SYM_FLOAT_CONST,

    // 关键字
    SYM_KW_INT, SYM_KW_VOID, SYM_KW_RETURN, SYM_KW_CONST, 
    SYM_KW_MAIN, // 你的词法中 main 是关键字 <KW, 5>
    SYM_KW_FLOAT, SYM_KW_IF, SYM_KW_ELSE,

    // 运算符
    SYM_OP_PLUS,    // +
    SYM_OP_MINUS,   // -
    SYM_OP_MUL,     // *
    SYM_OP_DIV,     // /
    SYM_OP_MOD,     // %
    SYM_OP_ASSIGN,  // =
    SYM_OP_GT,      // >
    SYM_OP_LT,      // <
    SYM_OP_EQ,      // ==
    SYM_OP_LE,      // <=
    SYM_OP_GE,      // >=
    SYM_OP_NEQ,     // !=
    SYM_OP_AND,     // &&
    SYM_OP_OR,      // ||

    // 界符
    SYM_SE_LPAREN,  // (
    SYM_SE_RPAREN,  // )
    SYM_SE_LBRACE,  // {
    SYM_SE_RBRACE,  // }
    SYM_SE_SEMICN,  // ;
    SYM_SE_COMMA,   // ,

    // === 50+: 非终结符 (Non-Terminals) ===
    NON_PROGRAM = 50,
    NON_COMPUNIT, NON_UNITLIST, NON_UNIT,
    NON_DECL, NON_CONSTDECL, NON_VARDECL,
    NON_CONSTDEFLIST, NON_BTYPE, NON_CONSTDEF, NON_CONSTINITIAL,
    NON_VARDEFLIST, NON_VARDEF, NON_INITVAL,
    NON_FUNCDEF, NON_FUNCNAME, NON_OPTFUNCFPARAMS, NON_FUNCTYPE,
    NON_FUNCFPARAMS, NON_FUNCFPARAM,
    NON_BLOCK, NON_BLOCKITEMLIST, NON_BLOCKITEM,
    NON_STMT, NON_OPTEXP, 
    NON_EXP, NON_COND, NON_LVAL, NON_PRIMARYEXP, NON_NUMBER,
    NON_UNARYEXP, NON_OPTFUNCRPARAMS, NON_UNARYOP,
    NON_FUNCRPARAMS, // 注意：funcRParam 和 funcRParams 合并处理
    NON_MULEXP, NON_ADDEXP, 
    NON_RELEXP, NON_EQEXP, NON_LANDEXP, NON_LOREXP, 
    NON_CONSTEXP
};

// 产生式结构: A -> B C D
struct Production {
    int id;             
    int left;           
    vector<int> right;  
};

class GrammarAnalyzer {
public:
    vector<Production> productions;
    map<int, set<int>> firstSet;  
    map<int, set<int>> followSet; 

    // 初始化所有文法规则 (对应 finaltask.md)
    void initGrammar(); 
    
    // 计算集合
    void buildFirst();  
    void buildFollow(); 
    
    // 打印结果
    void printSets();
    
    // 辅助工具
    bool isTerminal(int sym);
    string getSymbolName(int id);

private:
    bool mergeSet(set<int>& target, const set<int>& source, bool excludeEpsilon);
};

#endif