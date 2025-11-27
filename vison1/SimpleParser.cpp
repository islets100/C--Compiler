#include "SimpleParser.h"
#include <iostream>

SimpleParser::SimpleParser() : current_pos(0), step_count(0) {}

Token& SimpleParser::peek() {
    static Token eof_token("EOF", "EOF", -1);
    if (current_pos >= tokens.size()) {
        return eof_token;
    }
    return tokens[current_pos];
}

Token& SimpleParser::advance() {
    if (current_pos < tokens.size()) {
        return tokens[current_pos++];
    }
    static Token eof_token("EOF", "EOF", -1);
    return eof_token;
}

bool SimpleParser::match(const string& type, const string& value) {
    Token& t = peek();
    if (t.type == type && (value.empty() || t.value == value)) {
        advance();
        return true;
    }
    return false;
}

void SimpleParser::printStep(const string& action) {
    cout << ++step_count << "\t" << peek().value << "#" << peek().type << "\t" << action << endl;
}

shared_ptr<ASTNode> SimpleParser::parse(vector<Token>& token_list) {
    tokens = token_list;
    current_pos = 0;
    step_count = 0;
    
    return parseProgram();
}

shared_ptr<ASTNode> SimpleParser::parseProgram() {
    auto node = make_shared<ASTNode>("Program");
    auto comp_unit = parseCompUnit();
    if (comp_unit) {
        node->addChild(comp_unit);
    }
    return node;
}

shared_ptr<ASTNode> SimpleParser::parseCompUnit() {
    auto node = make_shared<ASTNode>("compUnit");
    
    while (peek().type != "EOF") {
        if (peek().value == "const" || peek().value == "int" || peek().value == "float") {
            // 可能是声明或函数定义
            int save_pos = current_pos;
            
            // 尝试解析声明
            auto decl = parseDecl();
            if (decl) {
                node->addChild(decl);
                continue;
            }
            
            // 回溯并尝试函数定义
            current_pos = save_pos;
            auto func = parseFuncDef();
            if (func) {
                node->addChild(func);
                continue;
            }
            
            // 都失败了，跳过这个token
            current_pos = save_pos;
            advance();
        } else {
            advance();
        }
    }
    
    return node;
}

shared_ptr<ASTNode> SimpleParser::parseDecl() {
    auto node = make_shared<ASTNode>("decl");
    
    if (peek().value == "const") {
        // constDecl
        advance(); // const
        
        // btype
        if (peek().value == "int" || peek().value == "float") {
            advance();
        }
        
        // constDef
        if (peek().type == "IDN") {
            advance(); // Ident
        }
        
        if (match("OP", "=")) {
            // constInitial
            parseExp();
        }
        
        if (match("SE", ";")) {
            return node;
        }
    } else if (peek().value == "int" || peek().value == "float") {
        // varDecl
        return parseVarDecl();
    }
    
    return nullptr;
}

shared_ptr<ASTNode> SimpleParser::parseVarDecl() {
    auto node = make_shared<ASTNode>("varDecl");
    
    // btype
    if (peek().value == "int" || peek().value == "float") {
        advance();
    }
    
    // varDef
    auto var_def = parseVarDef();
    if (var_def) {
        node->addChild(var_def);
    }
    
    // ;
    if (match("SE", ";")) {
        return node;
    }
    
    return nullptr;
}

shared_ptr<ASTNode> SimpleParser::parseVarDef() {
    auto node = make_shared<ASTNode>("varDef");
    
    // Ident
    if (peek().type == "IDN") {
        auto ident = make_shared<ASTNode>("Ident", peek().value);
        node->addChild(ident);
        advance();
    }
    
    // = initVal
    if (match("OP", "=")) {
        auto init_val = parseExp();
        if (init_val) {
            node->addChild(init_val);
        }
    }
    
    return node;
}

shared_ptr<ASTNode> SimpleParser::parseFuncDef() {
    auto node = make_shared<ASTNode>("funcDef");
    
    // funcType
    if (peek().value == "int" || peek().value == "float" || peek().value == "void") {
        advance();
    }
    
    // Ident
    if (peek().type == "IDN") {
        advance();
    }
    
    // (
    if (match("SE", "(")) {
        // )
        if (match("SE", ")")) {
            // block
            auto block = parseBlock();
            if (block) {
                node->addChild(block);
                return node;
            }
        }
    }
    
    return nullptr;
}

shared_ptr<ASTNode> SimpleParser::parseBlock() {
    auto node = make_shared<ASTNode>("block");
    
    // {
    if (!match("SE", "{")) {
        return nullptr;
    }
    
    // blockItems
    while (peek().value != "}" && peek().type != "EOF") {
        auto item = parseBlockItem();
        if (item) {
            node->addChild(item);
        } else {
            advance();
        }
    }
    
    // }
    if (match("SE", "}")) {
        return node;
    }
    
    return nullptr;
}

shared_ptr<ASTNode> SimpleParser::parseBlockItem() {
    if (peek().value == "const" || peek().value == "int" || peek().value == "float") {
        return parseDecl();
    } else {
        return parseStmt();
    }
}

shared_ptr<ASTNode> SimpleParser::parseStmt() {
    auto node = make_shared<ASTNode>("stmt");
    
    if (peek().value == "return") {
        advance(); // return
        
        // exp?
        if (peek().value != ";") {
            auto exp = parseExp();
            if (exp) {
                node->addChild(exp);
            }
        }
        
        if (match("SE", ";")) {
            return node;
        }
    } else if (peek().value == "{") {
        return parseBlock();
    } else if (peek().value == ";") {
        advance();
        return node;
    } else {
        // exp ;
        auto exp = parseExp();
        if (exp) {
            node->addChild(exp);
        }
        
        if (match("SE", ";")) {
            return node;
        }
    }
    
    return nullptr;
}

shared_ptr<ASTNode> SimpleParser::parseExp() {
    return parseAddExp();
}

shared_ptr<ASTNode> SimpleParser::parseAddExp() {
    auto left = parseMulExp();
    
    while (peek().value == "+" || peek().value == "-") {
        auto node = make_shared<ASTNode>("addExp");
        node->addChild(left);
        
        auto op = make_shared<ASTNode>("OP", peek().value);
        node->addChild(op);
        advance();
        
        auto right = parseMulExp();
        if (right) {
            node->addChild(right);
        }
        
        left = node;
    }
    
    return left;
}

shared_ptr<ASTNode> SimpleParser::parseMulExp() {
    auto left = parseUnaryExp();
    
    while (peek().value == "*" || peek().value == "/" || peek().value == "%") {
        auto node = make_shared<ASTNode>("mulExp");
        node->addChild(left);
        
        auto op = make_shared<ASTNode>("OP", peek().value);
        node->addChild(op);
        advance();
        
        auto right = parseUnaryExp();
        if (right) {
            node->addChild(right);
        }
        
        left = node;
    }
    
    return left;
}

shared_ptr<ASTNode> SimpleParser::parseUnaryExp() {
    return parsePrimaryExp();
}

shared_ptr<ASTNode> SimpleParser::parsePrimaryExp() {
    if (peek().value == "(") {
        advance(); // (
        auto exp = parseExp();
        if (match("SE", ")")) {
            return exp;
        }
    } else if (peek().type == "IDN") {
        return parseLVal();
    } else if (peek().type == "INT" || peek().type == "FLOAT") {
        return parseNumber();
    }
    
    return nullptr;
}

shared_ptr<ASTNode> SimpleParser::parseNumber() {
    auto node = make_shared<ASTNode>("number");
    
    if (peek().type == "INT") {
        auto int_node = make_shared<ASTNode>("INT", peek().value);
        node->addChild(int_node);
        advance();
    } else if (peek().type == "FLOAT") {
        auto float_node = make_shared<ASTNode>("FLOAT", peek().value);
        node->addChild(float_node);
        advance();
    }
    
    return node;
}

shared_ptr<ASTNode> SimpleParser::parseLVal() {
    auto node = make_shared<ASTNode>("lVal");
    
    if (peek().type == "IDN") {
        auto ident = make_shared<ASTNode>("Ident", peek().value);
        node->addChild(ident);
        advance();
        return node;
    }
    
    return nullptr;
}
