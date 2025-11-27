#include "Parser.h"
#include <iostream>
#include <algorithm> // for reverse

using namespace std;

TreeNode* Parser::parse() {
    // 1. 初始化
    while (!stateStack.empty()) stateStack.pop();
    while (!nodeStack.empty()) nodeStack.pop();
    
    stateStack.push(0); // 初始状态 0
    int ip = 0; // 输入指针
    int step = 1;

    cout << "\n================ PARSING PROCESS ================\n";
    cout << "Step\tState\tInput\tAction\n";

    while (true) {
        int currentState = stateStack.top();
        Token currentToken = tokens[ip];
        int symbolId = currentToken.type;

        // 查找动作
        if (slrTable->actionTable.find(currentState) == slrTable->actionTable.end() ||
            slrTable->actionTable[currentState].find(symbolId) == slrTable->actionTable[currentState].end()) {
            
            // 详细错误报告
            cerr << "\n========== 语法错误详情 ==========" << endl;
            cerr << "[错误位置] 行号: " << currentToken.line << endl;
            cerr << "[错误符号] 文本: '" << currentToken.text << "'" << endl;
            cerr << "[符号类型] ID: " << symbolId;
            if (!currentToken.category.empty()) {
                cerr << ", 原始类别: '" << currentToken.category << "'";
            }
            cerr << ", 符号名: '" << slrTable->grammar->getSymbolName(symbolId) << "'" << endl;
            cerr << "[当前状态] State " << currentState << endl;
            
            // 显示在当前状态下期望的符号
            if (slrTable->actionTable.find(currentState) != slrTable->actionTable.end()) {
                cerr << "[期望符号] 在当前状态 " << currentState << " 下，可以接受以下符号:" << endl;
                cerr << "            ";
                int count = 0;
                for (const auto& entry : slrTable->actionTable[currentState]) {
                    if (count > 0 && count % 5 == 0) cerr << endl << "            ";
                    cerr << "'" << slrTable->grammar->getSymbolName(entry.first) << "' ";
                    count++;
                }
                cerr << endl;
            } else {
                cerr << "[期望符号] 状态 " << currentState << " 没有定义任何动作" << endl;
            }
            
            // 如果符号无法识别（SYM_ERROR），给出更详细的提示
            if (symbolId == SYM_ERROR) {
                cerr << "[错误原因] 词法分析阶段无法识别的符号类别" << endl;
                if (!currentToken.category.empty()) {
                    cerr << "           类别 '" << currentToken.category << "' 不在语法定义中" << endl;
                }
            } else {
                cerr << "[错误原因] 语法分析阶段：当前符号 '" << currentToken.text 
                     << "' (类型: " << slrTable->grammar->getSymbolName(symbolId) 
                     << ") 在当前语法状态下不被接受" << endl;
            }
            cerr << "===================================\n" << endl;
                
            cerr << "Syntax Error at line " << currentToken.line 
                 << ": Unexpected token '" << currentToken.text << "'" << endl;
            return nullptr;
        }

        Action act = slrTable->actionTable[currentState][symbolId];

        // 打印步骤 (作业要求)
        cout << step++ << "\t" << currentState << "\t" 
             << slrTable->grammar->getSymbolName(symbolId) << "\t";

        if (act.type == ACT_SHIFT) {
            // === 移进 (Shift) ===
            cout << "Shift " << act.target << endl;
            
            stateStack.push(act.target);
            
            // 创建终结符节点入栈
            TreeNode* leaf = new TreeNode(symbolId, currentToken.text, currentToken.line);
            nodeStack.push(leaf);
            
            ip++; // 读下一个 Token
            
        } else if (act.type == ACT_REDUCE) {
            // === 归约 (Reduce) ===
            int prodId = act.target;
            const auto& prod = slrTable->grammar->productions[prodId - 1];
            
            cout << "Reduce " << prodId << " (" 
                 << slrTable->grammar->getSymbolName(prod.left) << " -> ...)" << endl;

            // 1. 弹出对应数量的状态和节点
            vector<TreeNode*> children;
            int rightLen = prod.right.size();
            
            // 特殊处理空产生式: A -> eps
            if (rightLen == 1 && prod.right[0] == SYM_EPSILON) {
                rightLen = 0;
            }

            for (int i = 0; i < rightLen; i++) {
                stateStack.pop();
                children.push_back(nodeStack.top());
                nodeStack.pop();
            }
            
            // 栈是后进先出，所以子节点现在是倒序的，需要反转
            reverse(children.begin(), children.end());

            // 2. 创建新的非终结符节点
            TreeNode* innerNode = new TreeNode(prod.left, "", 0);
            if (!children.empty()) innerNode->line = children[0]->line; // 继承行号
            innerNode->children = children;
            nodeStack.push(innerNode);

            // 3. 查 GOTO 表
            int preState = stateStack.top();
            int nextState = slrTable->gotoTable[preState][prod.left];
            
            stateStack.push(nextState);
            
            // 注意：归约时不移动输入指针 ip

        } else if (act.type == ACT_ACCEPT) {
            // === 接受 (Accept) ===
            cout << "ACCEPT!" << endl;
            return nodeStack.top(); // 返回根节点
        }
    }
}

void Parser::printTree(TreeNode* root, int depth) {
    if (!root) return;
    
    for (int i = 0; i < depth; i++) cout << "  ";
    
    // 打印格式： [ID] Name (Text)
    cout << "[" << root->symbolId << "] " 
         << (root->symbolId < 50 ? "T: " : "N: ") 
         // 这里需要一种方法获取符号名，可以传 GrammarAnalyzer 进来或者把 getSymbolName 设为静态
         // 暂时简单打印 text
         << root->text << endl;

    for (auto child : root->children) {
        printTree(child, depth + 1);
    }
}