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
#include <cctype>
#include <fstream>
 #include <iostream>
 #include <string>
#include <unordered_map>
#include <utility>
 #include <vector>

#include "../../Syntax/GrammarAnalyzer.h"
#include "../../Syntax/SyntaxTree.h"
#include "../include/IRGenerator.h"
 
 using namespace std;
 
 // ==========================================
// 1. 辅助工具
 // ==========================================
 
string trim(const string &str) {
     size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return "";
    }
     size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

unordered_map<string, int> buildSymbolIdMap(GrammarAnalyzer &ga) {
    unordered_map<string, int> mapping;
    for (int id = SYM_EPSILON; id <= NON_CONSTEXP; ++id) {
        string name = ga.getSymbolName(id);
        if (!name.empty() && name != "?") {
            mapping[name] = id;
        }
    }
    // 兜底：允许直接使用 epsilon 表示
    mapping["eps"] = SYM_EPSILON;
    return mapping;
}

TreeNode *loadSyntaxTree(const string &filename, GrammarAnalyzer &ga) {
     ifstream file(filename);
     if (!file.is_open()) {
        cerr << "Error: Cannot open syntax tree file " << filename << endl;
        return nullptr;
    }

    auto symbolMap = buildSymbolIdMap(ga);
    vector<pair<TreeNode *, int>> nodeStack;
    TreeNode *root = nullptr;
    string line;
    int lineNo = 0;
 
     while (getline(file, line)) {
        lineNo++;
        string raw = trim(line);
        if (raw.empty()) {
             continue;
         }
 
        int indentSpaces = 0;
        while (indentSpaces < static_cast<int>(line.size()) &&
               line[indentSpaces] == ' ') {
            indentSpaces++;
        }
        int depth = indentSpaces / 2;

        string symbolName = raw;
        string nodeText;
        size_t delimPos = raw.find(" (");
        if (delimPos != string::npos && raw.back() == ')') {
            symbolName = raw.substr(0, delimPos);
            nodeText = raw.substr(delimPos + 2, raw.size() - delimPos - 3);
        }

        auto it = symbolMap.find(symbolName);
        if (it == symbolMap.end()) {
            cerr << "Error: Unknown symbol '" << symbolName
                 << "' at line " << lineNo << " in " << filename << endl;
            if (root) {
                delete root;
            }
            return nullptr;
        }

        TreeNode *node = new TreeNode(it->second, nodeText, lineNo);

        while (!nodeStack.empty() && nodeStack.back().second >= depth) {
            nodeStack.pop_back();
        }

        if (nodeStack.empty()) {
            if (root) {
                cerr << "Error: Multiple root nodes detected in tree file."
                     << endl;
                delete node;
                delete root;
                return nullptr;
            }
            root = node;
        } else {
            nodeStack.back().first->children.push_back(node);
        }

        nodeStack.emplace_back(node, depth);
    }

    if (!root) {
        cerr << "Error: Syntax tree file is empty: " << filename << endl;
    }
    return root;
 }
 
 // ==========================================
 // 2. 主程序
 // ==========================================
 
 int main() {
    // --- 输入与输出文件配置 ---
    // 默认从 Syntax/output/tree.txt 读取语法树
    string treeFilename = "../Syntax/output/tree8.txt";
    // 如果需要运行不同测试，只需将 treeFilename 修改为 tree1.txt 等

     string numPart;
    for (char ch : treeFilename) {
        if (isdigit(static_cast<unsigned char>(ch))) {
            numPart.push_back(ch);
        }
    }
    if (numPart.empty()) {
        numPart = "tree";
    }

    string outputDir = "../complie_ir/output";
    string outPath = outputDir + "/" + numPart + "out.txt";
    ofstream outFile(outPath);
     if (!outFile.is_open()) {
        cerr << "Error: Cannot open log file " << outPath << endl;
         return 1;
     }

    streambuf *oldCoutBuf = cout.rdbuf(outFile.rdbuf());
    streambuf *oldCerrBuf = cerr.rdbuf(outFile.rdbuf());

    // --- 步骤 1: 准备文法信息 ---
     GrammarAnalyzer ga;
     cout << "[1] Initializing Grammar..." << endl;
     ga.initGrammar();
     ga.buildFirst();
     ga.buildFollow();
    cout << "[1] Grammar ready." << endl;

    // --- 步骤 2: 读取语法树 ---
    cout << "[2] Loading syntax tree from " << treeFilename << "..." << endl;
    TreeNode *root = loadSyntaxTree(treeFilename, ga);
    if (!root) {
        cerr << "Error: Failed to load syntax tree. Abort IR generation."
             << endl;
        cout.rdbuf(oldCoutBuf);
        cerr.rdbuf(oldCerrBuf);
         return 1;
     }
    cout << "    Syntax tree loaded successfully." << endl;

    // --- 步骤 3: 生成 IR 代码 ---
    cout << "[3] Generating IR code..." << endl;
    IRGenerator irGen(&ga);
    string irPath = outputDir + "/" + numPart + ".ll";
    bool success = irGen.generate(root, irPath);
    if (success) {
        cout << "    IR code written to " << irPath << endl;
     } else {
        cerr << "    IR generation failed." << endl;
    }

    delete root;

     cout.flush();
     cerr.flush();
     outFile.flush();
     outFile.close();
     
    cout.rdbuf(oldCoutBuf);
    cerr.rdbuf(oldCerrBuf);
 
    return success ? 0 : 1;
 }