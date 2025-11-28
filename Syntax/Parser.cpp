/*
 *                        _oo0oo_
 *                       o8888888o
 *                       88" . "88
 *                       (| -_- |)
 *                       0\  =  /0
 *                     ___/`---'\___
 *                   .' \\|     |// '.
 *                  / \\|||  :  |||// \
 *                 / _||||| -:- |||||- \
 *                |   | \\\  - /// |   |
 *                | \_|  ''\---/''  |_/ |
 *                \  .-\__  '-'  ___/-. /
 *              ___'. .'  /--.--\  `. .'___
 *           ."" '<  `.___\_<|>_/___.' >' "".
 *          | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 *          \  \ `_.   \_ __\ /__ _/   .-` /  /
 *      =====`-.____`.___ \_____/___.-`___.-'=====
 *                        `=---='
 * 
 * 
 *      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 
 *            佛祖保佑       永不宕机     永无BUG
 * 
 *        佛曰:  
 *                写字楼里写字间，写字间里程序员；  
 *                程序人员写程序，又拿程序换酒钱。  
 *                酒醒只在网上坐，酒醉还来网下眠；  
 *                酒醉酒醒日复日，网上网下年复年。  
 *                但愿老死电脑间，不愿鞠躬老板前；  
 *                奔驰宝马贵者趣，公交自行程序员。  
 *                别人笑我忒疯癫，我笑自己命太贱；  
 *                不见满街漂亮妹，哪个归得程序员？
 * 
 * @Description    : parser part of the compiler
 * @Auther         : Ren Xiaohua
 * @LastEditors    : Ren Xiaohua
 * @Date           : 2025-11-28 19:20:53
 * @LastEditTime   : 2025-11-28 19:20:53
 * Copyright 2025 Ren Xiaohua, All Rights Reserved. 
 */
#include "Parser.h"
#include "ReductionSequenceLogger.h"
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

        // ========== 处理词法错误（ERROR标记）==========
        // 如果是词法分析阶段识别的错误字符（ERROR标记），跳过并继续分析
        if (symbolId == SYM_ERROR && currentToken.category.find("ERROR") == 0) {
            // 从category中提取列号信息（格式为 "ERROR:列号"）
            int errorCol = 0;
            size_t colonPos = currentToken.category.find(':');
            if (colonPos != string::npos) {
                try {
                    errorCol = stoi(currentToken.category.substr(colonPos + 1));
                } catch (...) {}
            }
            
            if (errorCol > 0) {
                cerr << "\n[词法错误] 行号 " << currentToken.line 
                     << ", 列号 " << errorCol
                     << ": 无法识别的字符 '" << currentToken.text << "'" << endl;
            } else {
                cerr << "\n[词法错误] 行号 " << currentToken.line 
                     << ": 无法识别的字符 '" << currentToken.text << "'" << endl;
            }
            cerr << "          该字符将被跳过，继续分析后续内容。" << endl;
            
            // 跳过这个错误token，继续分析下一个
            ip++;
            // 检查是否到达文件末尾
            if (ip >= tokens.size()) {
                cerr << "\n错误: 在跳过错误字符后到达文件末尾，无法完成语法分析。" << endl;
                return nullptr;
            }
            continue;  // 继续分析下一个token
        }

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
            
            // 记录错误到规约序列（隔离处理，不影响核心逻辑）
            if (sequenceLogger) {
                int stackTopId = SYM_EPSILON;
                string stackTopText = "";
                if (!nodeStack.empty()) {
                    TreeNode* top = nodeStack.top();
                    stackTopId = top->symbolId;
                    stackTopText = top->text;
                }
                sequenceLogger->logError(stackTopId, stackTopText, symbolId, currentToken.text);
            }
            
            return nullptr;
        }

        Action act = slrTable->actionTable[currentState][symbolId];

        // 打印步骤 (作业要求)
        cout << step++ << "\t" << currentState << "\t" 
             << slrTable->grammar->getSymbolName(symbolId) << "\t";

        if (act.type == ACT_SHIFT) {
            // === 移进 (Shift) ===
            cout << "Shift " << act.target << endl;
            
            // 记录规约序列（在执行动作之前，获取当前栈顶符号）
            int prevStackTopId = SYM_EPSILON;
            string prevStackTopText = "";
            if (!nodeStack.empty()) {
                TreeNode* top = nodeStack.top();
                prevStackTopId = top->symbolId;
                prevStackTopText = top->text;
            }
            
            stateStack.push(act.target);
            
            // 创建终结符节点入栈
            TreeNode* leaf = new TreeNode(symbolId, currentToken.text, currentToken.line);
            nodeStack.push(leaf);
            
            ip++; // 读下一个 Token
            
            // 记录规约序列（隔离处理，不影响核心逻辑）
            // 移进后，栈顶是刚移进的符号，面临输入是下一个符号
            if (sequenceLogger) {
                int nextSymbolId = (ip < tokens.size()) ? tokens[ip].type : SYM_EOF;
                string nextText = (ip < tokens.size()) ? tokens[ip].text : "$";
                // 使用文本值（如 "int", "a"）而不是符号类型
                sequenceLogger->logShift(symbolId, currentToken.text, nextSymbolId, nextText);
            }
            
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
            
            // 记录规约序列（隔离处理，不影响核心逻辑）
            // 归约后，栈顶是归约产生的非终结符，面临输入是当前输入符号
            if (sequenceLogger) {
                sequenceLogger->logReduce(prod.left, "", symbolId, currentToken.text, prod.left);
            }
            
            // 注意：归约时不移动输入指针 ip

        } else if (act.type == ACT_ACCEPT) {
            // === 接受 (Accept) ===
            cout << "ACCEPT!" << endl;
            
            // 记录规约序列（隔离处理，不影响核心逻辑）
            if (sequenceLogger && !nodeStack.empty()) {
                TreeNode* root = nodeStack.top();
                sequenceLogger->logAccept(root->symbolId, "");
            }
            
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