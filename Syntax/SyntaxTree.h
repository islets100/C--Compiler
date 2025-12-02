#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include <string>
#include <vector>

using namespace std;

struct TreeNode {
    int id;               // 节点ID (唯一，用于调试)
    int symbolId;         // 符号 ID (GrammarAnalyzer 中的 SymbolType)
    string text;          // 文本内容 (对于终结符，如 "main", "10", "+")
    int line;             // 行号
    
    vector<TreeNode*> children; // 子节点列表

    // 构造函数
    TreeNode(int sId, string txt, int ln) 
        : symbolId(sId), text(txt), line(ln) {
        static int globalId = 0;
        id = globalId++;
    }

    // 析构函数 (简单处理，实际项目建议用智能指针)
    ~TreeNode() {
        for (auto c : children) delete c;
    }
};

#endif