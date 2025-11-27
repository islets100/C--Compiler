#ifndef SIMPLE_PARSER_H
#define SIMPLE_PARSER_H

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <memory>

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
    string name;      // 节点名称
    string value;     // 节点值
    vector<shared_ptr<ASTNode>> children;
    int production;   // 使用的产生式编号
    
    ASTNode(string n, string v = "", int p = -1) : name(n), value(v), production(p) {}
    
    void addChild(shared_ptr<ASTNode> child) {
        children.push_back(child);
    }
};

// 简化的递归下降分析器
class SimpleParser {
private:
    vector<Token> tokens;
    int current_pos;
    int step_count;
    
public:
    SimpleParser();
    
    shared_ptr<ASTNode> parse(vector<Token>& token_list);
    
private:
    Token& peek();
    Token& advance();
    bool match(const string& type, const string& value = "");
    void printStep(const string& action);
    
    // 递归下降分析函数
    shared_ptr<ASTNode> parseProgram();
    shared_ptr<ASTNode> parseCompUnit();
    shared_ptr<ASTNode> parseDecl();
    shared_ptr<ASTNode> parseVarDecl();
    shared_ptr<ASTNode> parseVarDef();
    shared_ptr<ASTNode> parseFuncDef();
    shared_ptr<ASTNode> parseBlock();
    shared_ptr<ASTNode> parseBlockItem();
    shared_ptr<ASTNode> parseStmt();
    shared_ptr<ASTNode> parseExp();
    shared_ptr<ASTNode> parseAddExp();
    shared_ptr<ASTNode> parseMulExp();
    shared_ptr<ASTNode> parseUnaryExp();
    shared_ptr<ASTNode> parsePrimaryExp();
    shared_ptr<ASTNode> parseNumber();
    shared_ptr<ASTNode> parseLVal();
};

#endif
