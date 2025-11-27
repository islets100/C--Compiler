#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cctype>
#include <algorithm>
#include "SimpleParser.h"
#include "CodeGenerator.h"

using namespace std;

// 词法分析器
class LexicalAnalyzer {
private:
    map<string, int> keyword_code = {
        {"int", 1}, {"void", 2}, {"return", 3}, {"const", 4}, 
        {"main", 5}, {"float", 6}, {"if", 7}, {"else", 8}
    };
    
    map<string, int> operator_code = {
        {"+", 9}, {"-", 10}, {"*", 11}, {"/", 12}, {"%", 13}, 
        {"=", 14}, {">", 15}, {"<", 16}, {"==", 17}, {"<=", 18}, 
        {">=", 19}, {"!=", 20}, {"&&", 21}, {"||", 22}
    };
    
    map<string, int> separator_code = {
        {"(", 23}, {")", 24}, {"{", 25}, {"}", 26}, {";", 27}, {",", 28}
    };
    
public:
    vector<Token> analyze(const string& source_code) {
        vector<Token> tokens;
        int i = 0;
        
        while (i < source_code.length()) {
            // 跳过空白符
            while (i < source_code.length() && isspace(source_code[i])) {
                i++;
            }
            if (i >= source_code.length()) break;
            
            char ch = source_code[i];
            
            // 处理数字
            if (isdigit(ch)) {
                string num;
                while (i < source_code.length() && isdigit(source_code[i])) {
                    num += source_code[i];
                    i++;
                }
                
                // 检查是否是浮点数
                if (i < source_code.length() && source_code[i] == '.') {
                    num += '.';
                    i++;
                    while (i < source_code.length() && isdigit(source_code[i])) {
                        num += source_code[i];
                        i++;
                    }
                    tokens.push_back(Token("FLOAT", num, -1));
                } else {
                    tokens.push_back(Token("INT", num, -1));
                }
                continue;
            }
            
            // 处理标识符和关键字
            if (isalpha(ch) || ch == '_') {
                string ident;
                while (i < source_code.length() && (isalnum(source_code[i]) || source_code[i] == '_')) {
                    ident += source_code[i];
                    i++;
                }
                
                string lower_ident = ident;
                transform(lower_ident.begin(), lower_ident.end(), lower_ident.begin(), ::tolower);
                
                if (keyword_code.find(lower_ident) != keyword_code.end()) {
                    tokens.push_back(Token("KW", lower_ident, keyword_code[lower_ident]));
                } else {
                    tokens.push_back(Token("IDN", ident, -1));
                }
                continue;
            }
            
            // 处理运算符和分隔符
            if (i + 1 < source_code.length()) {
                string two_char = source_code.substr(i, 2);
                if (operator_code.find(two_char) != operator_code.end()) {
                    tokens.push_back(Token("OP", two_char, operator_code[two_char]));
                    i += 2;
                    continue;
                }
            }
            
            string one_char(1, ch);
            
            if (operator_code.find(one_char) != operator_code.end()) {
                tokens.push_back(Token("OP", one_char, operator_code[one_char]));
            } else if (separator_code.find(one_char) != separator_code.end()) {
                tokens.push_back(Token("SE", one_char, separator_code[one_char]));
            } else {
                cerr << "Unknown character: " << ch << endl;
            }
            
            i++;
        }
        
        // 添加EOF标记
        tokens.push_back(Token("EOF", "EOF", -1));
        
        return tokens;
    }
    
    void printTokens(const vector<Token>& tokens, const string& output_file) {
        ofstream out(output_file);
        for (const auto& token : tokens) {
            if (token.type == "KW") {
                out << token.value << "\t<KW," << keyword_code[token.value] << ">" << endl;
            } else if (token.type == "OP") {
                out << token.value << "\t<OP," << operator_code[token.value] << ">" << endl;
            } else if (token.type == "SE") {
                out << token.value << "\t<SE," << separator_code[token.value] << ">" << endl;
            } else if (token.type == "IDN") {
                out << token.value << "\t<IDN," << token.value << ">" << endl;
            } else if (token.type == "INT") {
                out << token.value << "\t<INT," << token.value << ">" << endl;
            } else if (token.type == "FLOAT") {
                out << token.value << "\t<FLOAT," << token.value << ">" << endl;
            }
        }
        out.close();
    }
};

// 读取源文件
string readSourceFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Cannot open file: " << filename << endl;
        return "";
    }
    
    stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return buffer.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <source_file>" << endl;
        return 1;
    }
    
    string source_file = argv[1];
    string source_code = readSourceFile(source_file);
    
    if (source_code.empty()) {
        return 1;
    }
    
    cout << "=== Lexical Analysis ===" << endl;
    LexicalAnalyzer lexer;
    vector<Token> tokens = lexer.analyze(source_code);
    
    // 输出词法分析结果
    string lex_output = "output_lex.txt";
    lexer.printTokens(tokens, lex_output);
    cout << "Lexical analysis completed. Output: " << lex_output << endl;
    
    cout << "\n=== Syntax Analysis ===" << endl;
    SimpleParser parser;
    
    // 进行语法分析
    shared_ptr<ASTNode> ast = parser.parse(tokens);
    
    if (!ast) {
        cerr << "Syntax analysis failed!" << endl;
        return 1;
    }
    
    cout << "Syntax analysis completed." << endl;
    
    cout << "\n=== Code Generation ===" << endl;
    CodeGenerator codegen;
    codegen.generate(ast);
    codegen.printIR("output.ll");
    cout << "Code generation completed. Output: output.ll" << endl;
    
    return 0;
}
