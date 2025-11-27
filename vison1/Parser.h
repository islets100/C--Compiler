#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <memory>
#include <algorithm>

using namespace std;

// 词法单元结构
struct Token {
    string type;      // KW, OP, SE, IDN, INT, FLOAT
    string value;     // 单词的值
    int code;         // 单词编码
    
    Token() {}
    Token(string t, string v, int c) : type(t), value(v), code(c) {}
};

// 抽象语法树节点
struct ASTNode {
    string name;      // 节点名称（非终结符或终结符）
    string value;     // 节点值（对于终结符）
    vector<shared_ptr<ASTNode>> children;
    int production;   // 使用的产生式编号
    
    ASTNode(string n, string v = "", int p = -1) : name(n), value(v), production(p) {}
    
    void addChild(shared_ptr<ASTNode> child) {
        children.push_back(child);
    }
};

// 文法产生式
struct Production {
    int id;
    string left;
    vector<string> right;
    
    Production(int i, string l, vector<string> r) : id(i), left(l), right(r) {}
};

// LR项目
struct Item {
    int prod_id;      // 产生式ID
    int dot_pos;      // 点的位置
    string lookahead; // 前看符号
    
    Item(int p, int d, string l) : prod_id(p), dot_pos(d), lookahead(l) {}
    
    bool operator<(const Item& other) const {
        if (prod_id != other.prod_id) return prod_id < other.prod_id;
        if (dot_pos != other.dot_pos) return dot_pos < other.dot_pos;
        return lookahead < other.lookahead;
    }
    
    bool operator==(const Item& other) const {
        return prod_id == other.prod_id && dot_pos == other.dot_pos && lookahead == other.lookahead;
    }
};

// SLR分析表项
struct ActionEntry {
    string action;    // "shift", "reduce", "accept", "error"
    int state;        // 对于shift，目标状态；对于reduce，产生式ID
    
    ActionEntry() : action("error"), state(-1) {}
    ActionEntry(string a, int s) : action(a), state(s) {}
};

class SLRParser {
private:
    vector<Production> productions;
    map<string, set<string>> first_set;
    map<string, set<string>> follow_set;
    map<pair<int, string>, ActionEntry> action_table;
    map<pair<int, string>, int> goto_table;
    vector<set<Item>> lr0_items;
    set<string> non_terminals;
    set<string> terminals;
    
    void initProductions();
    void computeFirstSet();
    void computeFollowSet();
    void buildLR0Items();
    void buildSLRTable();
    set<Item> closure(const set<Item>& items);
    set<Item> goTo(const set<Item>& items, const string& symbol);
    
public:
    SLRParser();
    
    shared_ptr<ASTNode> parse(vector<Token>& tokens);
    void printParseTrace(const vector<Token>& tokens);
    void printProductions();
    void printFirstFollow();
};

#endif
