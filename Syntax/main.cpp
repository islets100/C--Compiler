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
 * @Description    : main part of the compiler
 * @Auther         : Ren Xiaohua
 * @LastEditors    : Ren Xiaohua
 * @Date           : 2025-11-28 19:20:53
 * @LastEditTime   : 2025-11-28 19:20:53
 * Copyright 2025 Ren Xiaohua, All Rights Reserved. 
 */
 #include <iostream>
 #include <fstream>
 #include <string>
 #include <vector>
 #include <algorithm>
 #include <cctype>
 
 #include "GrammarAnalyzer.h"
 #include "SLRTable.h"
 #include "Parser.h" 
 #include "ReductionSequenceLogger.h"
 // (注意：Parser.h 里已经定义了 struct Token)
 
 using namespace std;
 
 // ==========================================
 // 1. 词法文件读取逻辑 (TokenLoader 部分)
 // ==========================================
 
 // 辅助：去除字符串两端空格
 string trim(const string& str) {
     size_t first = str.find_first_not_of(" \t\r\n");
     if (string::npos == first) return "";
     size_t last = str.find_last_not_of(" \t\r\n");
     return str.substr(first, (last - first + 1));
 }
 
 // 核心映射函数：把 txt 里的 <KW, 1> 变成 SYM_KW_INT
 SymbolType mapTokenToSymbol(string category, int val) {
     if (category == "KW") {
         switch(val) {
             case 1: return SYM_KW_INT;
             case 2: return SYM_KW_VOID;
             case 3: return SYM_KW_RETURN;
             case 4: return SYM_KW_CONST;
             case 5: return SYM_KW_MAIN;  // 关键：main 关键字
             case 6: return SYM_KW_FLOAT;
             case 7: return SYM_KW_IF;
             case 8: return SYM_KW_ELSE;
             default: return SYM_ERROR;
         }
     }
     if (category == "OP") {
         switch(val) {
             case 9: return SYM_OP_PLUS;
             case 10: return SYM_OP_MINUS;
             case 11: return SYM_OP_MUL;
             case 12: return SYM_OP_DIV;
             case 13: return SYM_OP_MOD;
             case 14: return SYM_OP_ASSIGN;
             case 15: return SYM_OP_GT;
             case 16: return SYM_OP_LT;
             case 17: return SYM_OP_EQ;
             case 18: return SYM_OP_LE;
             case 19: return SYM_OP_GE;
             case 20: return SYM_OP_NEQ;
             case 21: return SYM_OP_AND;
             case 22: return SYM_OP_OR;
             default: return SYM_ERROR;
         }
     }
     if (category == "SE") {
         switch(val) {
             case 23: return SYM_SE_LPAREN;
             case 24: return SYM_SE_RPAREN;
             case 25: return SYM_SE_LBRACE;
             case 26: return SYM_SE_RBRACE;
             case 27: return SYM_SE_SEMICN;
             case 28: return SYM_SE_COMMA;
             default: return SYM_ERROR;
         }
     }
     if (category == "IDN") return SYM_ID;
     if (category == "INT") return SYM_INT_CONST;
     if (category == "FLOAT") return SYM_FLOAT_CONST;
 
     return SYM_ERROR;
 }
 
 // 读取文件并转换为 Token 列表
 vector<Token> loadTokens(string filename) {
     vector<Token> tokens;
     ifstream file(filename);
     string line;
     int currentLine = 1;
 
     if (!file.is_open()) {
         cerr << "Error: Cannot open " << filename << endl;
         return tokens;
     }
 
     while (getline(file, line)) {
         // 从后往前找最后一个 '<'，因为标记总是在行尾，这样可以避免文本本身包含 '<' 的情况
         // 例如：'< <OP,16>' 或 '<= <OP,18>' 这样的行
         size_t ltPos = line.rfind('<');
         if (ltPos == string::npos) continue;
         
         // 只在尖括号内部查找 ',' 和 '>'
         size_t gtPos = line.find('>', ltPos + 1);
         size_t commaPos = line.find(',', ltPos + 1);
 
         if (gtPos == string::npos || commaPos == string::npos || commaPos > gtPos) {
             continue;
         }
 
         // 处理 标签（文本部分在 '<' 之前）
         string rawTextPart = line.substr(0, ltPos);
         size_t sourceTagPos = rawTextPart.find("]");
         if (sourceTagPos != string::npos) {
             rawTextPart = rawTextPart.substr(sourceTagPos + 1);
         }
         string text = trim(rawTextPart);
         
         // 解析 <Type, Value> 或 <ERROR,行号,列号>
         string typeStr = line.substr(ltPos + 1, commaPos - ltPos - 1); 
         typeStr = trim(typeStr);
         
         // 检查是否是 ERROR 类型（格式为 <ERROR,行号,列号>）
         if (typeStr == "ERROR") {
             // ERROR 格式：<ERROR,行号,列号>
             // 需要找到第二个逗号
             size_t secondCommaPos = line.find(',', commaPos + 1);
             if (secondCommaPos != string::npos && secondCommaPos < gtPos) {
                 string lineNumStr = line.substr(commaPos + 1, secondCommaPos - commaPos - 1);
                 string colNumStr = line.substr(secondCommaPos + 1, gtPos - secondCommaPos - 1);
                 lineNumStr = trim(lineNumStr);
                 colNumStr = trim(colNumStr);
                 
                 int errorLine = 0, errorCol = 0;
                 try { errorLine = stoi(lineNumStr); } catch (...) {}
                 try { errorCol = stoi(colNumStr); } catch (...) {}
                 
                 Token t;
                 t.type = SYM_ERROR;
                 t.text = text;
                 t.line = errorLine;  // 使用错误标记中的行号
                 // 将列号信息编码到category中，格式为 "ERROR:列号"
                 t.category = "ERROR:" + to_string(errorCol);
                 tokens.push_back(t);
                 currentLine++;
                 continue;
             }
         }
         
         // 普通格式：<Type, Value>
         string valStr = line.substr(commaPos + 1, gtPos - commaPos - 1); 
         valStr = trim(valStr);
         
         int val = 0;
         try { val = stoi(valStr); } catch (...) { val = 0; }
 
         Token t;
         t.type = mapTokenToSymbol(typeStr, val);
         t.text = text;
         t.line = currentLine++;
         t.category = typeStr;  // 保存原始类别，用于错误报告
 
         // 调试：如果词法类别无法识别，打印出来看看
         if (t.type == SYM_ERROR) {
             cerr << "[LEX DEBUG] Unknown token at source line " << (currentLine - 1)
                  << ": category='" << typeStr
                  << "', val='" << valStr
                  << "', text='" << text << "'" << endl;
         }
         
         // 简单修正 text 为空的情况
         if (t.text.empty()) {
             if (t.type == SYM_SE_SEMICN) t.text = ";";
             else if (t.type == SYM_SE_LPAREN) t.text = "(";
             else if (t.type == SYM_SE_RPAREN) t.text = ")";
             else if (t.type == SYM_SE_LBRACE) t.text = "{";
             else if (t.type == SYM_SE_RBRACE) t.text = "}";
         }
 
         tokens.push_back(t);
     }
     
     // 必须添加 EOF 符号，否则语法分析器不知道何时结束
     tokens.push_back({SYM_EOF, "$", currentLine, "EOF", "EOF"});
     return tokens;
 }
 
 // ==========================================
 // 2. 主程序
 // ==========================================
 
 int main() {
     // --- 输入与输出文件名配置 ---
     // 从 lex/output/ 目录读取词法分析结果文件
     // 目前默认读取 lex1.txt，你可以手动改成 lex2.txt / lex3.txt 等
     string lexFilename = "../lex/output/lex4.txt";
     // 从文件名中提取数字部分，用于生成类似 "1out.txt" 的输出文件名
     string numPart;
     for (char ch : lexFilename) {
         if (isdigit(static_cast<unsigned char>(ch))) numPart.push_back(ch);
     }
     if (numPart.empty()) numPart = "out";
     string outFilename = "output/" + numPart + "out.txt";
 
     // 将终端输出重定向到对应的 txt 文件
     ofstream outFile(outFilename);
     if (!outFile.is_open()) {
         cerr << "Error: Cannot open output file " << outFilename << endl;
         return 1;
     }
     // 同时重定向 cout / cerr，所有后续终端输出都会写入该文件
     cout.rdbuf(outFile.rdbuf());
     cerr.rdbuf(outFile.rdbuf());
 
     // --- 步骤 1: 准备文法和 SLR 表 ---
     GrammarAnalyzer ga;
     cout << "[1] Initializing Grammar..." << endl;
     ga.initGrammar();
     ga.buildFirst();
     ga.buildFollow();
     // ==========================================
     // 打印集合，FOLLOW 集和FIRST集
     // ==========================================
     cout << "\n[DEBUG] Printing FIRST and FOLLOW sets..." << endl;
     ga.printSets(); 
     cout << "---------------------------------------\n" << endl;
 
     cout << "[3] Building SLR Table..." << endl;
     SLRTable slr(&ga);
     slr.buildTable();
     cout << "    SLR Table generated. Total States: " << slr.states.size() << endl;
 
     // --- 步骤 2: 读取词法分析结果 ---
     // 从 lex/output/ 目录读取词法分析结果文件
     cout << "[3] Loading Tokens from " << lexFilename << "..." << endl;
     vector<Token> tokens = loadTokens(lexFilename); 
     
     if (tokens.size() <= 1) { // 只有 EOF
         cerr << "Error: No valid tokens found in " << lexFilename << ". Please check the file path or content." << endl;
         return 1;
     }
     cout << "    Loaded " << tokens.size() << " tokens." << endl;
 
     // --- 步骤 3: 启动语法分析器 ---
     cout << "[4] Starting Parser..." << endl;
     
     // 创建规约序列记录器（隔离处理，不影响核心逻辑）
     ReductionSequenceLogger sequenceLogger(&ga);
     Parser parser(&slr, tokens);
     parser.sequenceLogger = &sequenceLogger;  // 设置 logger
     
     TreeNode* root = parser.parse();
     
     // 输出规约序列到 syntax1.txt 文件（无论解析成功或失败都要输出）
     string syntaxFilename = "output/syntax" + numPart + ".txt";
     sequenceLogger.writeToFile(syntaxFilename);
     if (root) {
         cout << "    Reduction sequence written to " << syntaxFilename << endl;
     } else {
         cerr << "    Reduction sequence (with error) written to " << syntaxFilename << endl;
     }
 
     // --- 步骤 4: 输出语法树 ---
     if (root) {
         // 创建单独的语法树输出文件
         string treeFilename = "output/tree" + numPart + ".txt";
         ofstream treeFile(treeFilename);
         if (!treeFile.is_open()) {
             cerr << "Error: Cannot open tree output file " << treeFilename << endl;
         }
         
         // Lambda 递归打印树（同时输出到 cout 和 treeFile）
         auto printTree = [&](TreeNode* node, int depth, auto&& self) -> void {
             if (!node) return;
             // 打印缩进
             string indent;
             for(int i=0; i<depth; i++) indent += "  ";
             
             // 构建节点字符串
             string nodeStr;
             if (node->symbolId < 50) {
                 // 终结符：打印符号类型和具体的文本值
                 nodeStr = ga.getSymbolName(node->symbolId) + " (" + node->text + ")";
             } else {
                 // 非终结符：只打印符号类型名
                 nodeStr = ga.getSymbolName(node->symbolId);
             }
             
             // 同时输出到 cout（写入 1out.txt）和 treeFile（写入 tree1.txt）
             cout << indent << nodeStr << endl;
             if (treeFile.is_open()) {
                 treeFile << indent << nodeStr << endl;
             }
             
             for (auto c : node->children) {
                 self(c, depth + 1, self);
             }
         };
         
         // 输出到原文件（包含分隔行）
         cout << "\n================ SYNTAX TREE ================\n";
         
         // tree 文件直接输出树内容，不包含分隔行
         printTree(root, 0, printTree);
         
         cout << "\nAnalysis Success! Syntax Tree generated." << endl;
         if (treeFile.is_open()) {
             treeFile.close();
             cout << "    Syntax tree written to " << treeFilename << endl;
         }
     } else {
         cout << "\nAnalysis Failed." << endl;
     }
 
     return 0;
 }