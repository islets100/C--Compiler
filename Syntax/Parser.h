#ifndef PARSER_H
#define PARSER_H

#include "SLRTable.h"
#include "SyntaxTree.h" // 确保你有这个文件，如果没有，见下一步
#include <vector>
#include <stack>
#include <string>

// 前向声明
class ReductionSequenceLogger;

using namespace std;

// 1. 在这里定义 Token 结构体，供 main.cpp 和 Parser.cpp 共同使用
struct Token {
    SymbolType type;   // 对应的终结符 ID (如 SYM_KW_INT)
    string text;       // 原始文本 (如 "int", "main")
    int line;          // 行号
    string typeName;   // 调试用的类型名
    string category;   // 原始类别 (如 "KW", "OP", "IDN") - 用于错误报告
};

// 前向声明
class ReductionSequenceLogger;

class Parser {
public:
    SLRTable* slrTable;
    vector<Token> tokens;
    
    // 状态栈
    stack<int> stateStack;
    // 节点栈 (用于构建语法树)
    stack<TreeNode*> nodeStack;
    
    // 可选的规约序列记录器（隔离处理，不影响核心逻辑）
    ReductionSequenceLogger* sequenceLogger;

    Parser(SLRTable* table, vector<Token> inputTokens) 
        : slrTable(table), tokens(inputTokens), sequenceLogger(nullptr) {}

    // 执行分析
    TreeNode* parse();

    // 辅助：打印语法树
    void printTree(TreeNode* root, int depth = 0);
};

#endif