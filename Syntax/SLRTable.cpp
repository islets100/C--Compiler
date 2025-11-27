#include "SLRTable.h"

// ==========================================
// 1. 计算闭包 (Closure)
// ==========================================
set<Item> SLRTable::getClosure(const set<Item>& I) {
    set<Item> J = I;
    bool changed = true;
    while (changed) {
        changed = false;
        // 使用临时副本遍历，避免在遍历时插入导致迭代器失效
        set<Item> currentItems = J;
        
        for (const auto& item : currentItems) {
            // item: A -> alpha . B beta
            // prodId 是从 1 开始的，所以要 -1 才能取到 vector 中的产生式
            const auto& prod = grammar->productions[item.prodId - 1];
            
            // 如果圆点还没到最后
            if (item.dotPos < prod.right.size()) {
                int B = prod.right[item.dotPos]; // 圆点后的符号
                
                // 如果 B 是非终结符，需要展开: B -> . gamma
                if (!grammar->isTerminal(B) && B != SYM_EPSILON) {
                    for (const auto& subProd : grammar->productions) {
                        if (subProd.left == B) {
                            // 新项目: B -> . gamma (圆点在 0)
                            Item newItem = {subProd.id, 0};
                            
                            // 如果是 A -> . eps，圆点其实还在 0，但逻辑上它已经可以归约了
                            // 这里我们只管加进去，归约逻辑在 buildTable 处理
                            if (J.find(newItem) == J.end()) {
                                J.insert(newItem);
                                changed = true;
                            }
                        }
                    }
                }
            }
        }
    }
    return J;
}

// ==========================================
// 2. 计算 GOTO(I, X)
// ==========================================
set<Item> SLRTable::getGoto(const set<Item>& I, int symbol) {
    set<Item> J;
    for (const auto& item : I) {
        const auto& prod = grammar->productions[item.prodId - 1];
        
        // 检查圆点后的符号是否匹配 symbol
        // item: A -> alpha . symbol beta
        if (item.dotPos < prod.right.size()) {
            int currentSym = prod.right[item.dotPos];
            
            // 特殊处理 epsilon: 如果产生式是 A -> epsilon，它不接受任何符号移进
            if (currentSym == SYM_EPSILON) continue;

            if (currentSym == symbol) {
                // 移进圆点: A -> alpha symbol . beta
                J.insert({item.prodId, item.dotPos + 1});
            }
        }
    }
    return getClosure(J);
}

// ==========================================
// 3. 构建表 (Build Table)
// ==========================================
void SLRTable::buildTable() {
    // 3.1 初始化状态 I0
    // 假设文法第 1 条是 Program -> CompUnit
    set<Item> startItemSet;
    startItemSet.insert({1, 0}); // Program -> . CompUnit
    states.push_back(getClosure(startItemSet));
    
    // 3.2 循环构造所有状态
    int processedStateCount = 0;
    while (processedStateCount < states.size()) {
        int i = processedStateCount; // 当前处理的状态 ID
        set<Item> I = states[i];
        
        // 收集该状态下圆点后出现的所有符号 (终结符 + 非终结符)
        set<int> nextSymbols;
        for (const auto& item : I) {
            const auto& prod = grammar->productions[item.prodId - 1];
            if (item.dotPos < prod.right.size()) {
                int sym = prod.right[item.dotPos];
                if (sym != SYM_EPSILON) nextSymbols.insert(sym);
            }
        }
        
        // 为每个符号计算 GOTO
        for (int sym : nextSymbols) {
            set<Item> nextStateItems = getGoto(I, sym);
            if (nextStateItems.empty()) continue;
            
            // 检查这个新状态集是否已存在
            int targetStateId = -1;
            for (int k = 0; k < states.size(); k++) {
                if (states[k] == nextStateItems) {
                    targetStateId = k;
                    break;
                }
            }
            
            // 如果是全新的状态
            if (targetStateId == -1) {
                targetStateId = states.size();
                states.push_back(nextStateItems);
            }
            
            // 填表
            if (grammar->isTerminal(sym)) {
                // ACTION[i, sym] = Shift(targetStateId)
                addAction(i, sym, {ACT_SHIFT, targetStateId});
            } else {
                // GOTO[i, sym] = targetStateId
                gotoTable[i][sym] = targetStateId;
            }
        }
        
        // 3.3 处理归约 (Reduce) 和 接受 (Accept)
        for (const auto& item : I) {
            const auto& prod = grammar->productions[item.prodId - 1];
            
            // 判断是否可以归约
            // 情况 A: A -> alpha . (圆点在末尾)
            // 情况 B: A -> epsilon . (产生式本身是空, 且 item.dotPos=0)
            bool readyToReduce = false;
            if (item.dotPos == prod.right.size()) readyToReduce = true;
            if (prod.right.size() == 1 && prod.right[0] == SYM_EPSILON) readyToReduce = true;

            if (readyToReduce) {
                // 如果是开始符号 S' -> S . (这里是 Program -> CompUnit)
                if (prod.left == NON_PROGRAM) {
                    addAction(i, SYM_EOF, {ACT_ACCEPT, 0});
                } else {
                    // SLR 归约规则: 对 FOLLOW(A) 中的每个符号 a，执行 Reduce(prodId)
                    for (int followSym : grammar->followSet[prod.left]) {
                        addAction(i, followSym, {ACT_REDUCE, prod.id});
                    }
                }
            }
        }

        // // ================= 调试输出：查看指定状态的项目与动作 =================
        // // 目前根据调试信息，我们重点关心 state 34 在遇到 ',' 时为什么没有动作
        // if (i == 34) {
        //     cout << "\n[DEBUG] Items in State 34:\n";
        //     for (const auto& item : I) {
        //         const auto& prod = grammar->productions[item.prodId - 1];
        //         cout << "  (" << item.prodId << ") "
        //              << grammar->getSymbolName(prod.left) << " -> ";
        //         // 打印右部并标出圆点位置
        //         for (size_t p = 0; p <= prod.right.size(); ++p) {
        //             if (p == item.dotPos) cout << ". ";
        //             if (p < prod.right.size()) {
        //                 cout << grammar->getSymbolName(prod.right[p]) << " ";
        //             }
        //         }
        //         cout << endl;
        //     }

        //     cout << "\n[DEBUG] ACTION table for State 34:\n";
        //     if (actionTable.count(34)) {
        //         for (auto const& [sym, act] : actionTable[34]) {
        //             cout << "  on '" << grammar->getSymbolName(sym) << "' -> ";
        //             if (act.type == ACT_SHIFT) cout << "Shift " << act.target;
        //             else if (act.type == ACT_REDUCE) cout << "Reduce " << act.target;
        //             else if (act.type == ACT_ACCEPT) cout << "ACCEPT";
        //             cout << endl;
        //         }
        //     } else {
        //         cout << "  (no actions)\n";
        //     }
        // }
        
        processedStateCount++;
    }
}

// ==========================================
// 4. 添加动作 & 解决冲突 (核心!)
// ==========================================
void SLRTable::addAction(int state, int symbol, Action newAction) {
    // 如果该格为空，直接填入
    if (actionTable[state].find(symbol) == actionTable[state].end()) {
        actionTable[state][symbol] = newAction;
        return;
    }
    
    Action existing = actionTable[state][symbol];
    
    // 如果动作完全一样，忽略
    if (existing.type == newAction.type && existing.target == newAction.target) return;
    
    // === 冲突处理逻辑 ===
    
    // 场景 1: 移进-归约冲突 (Shift-Reduce Conflict)
    // 通常发生在 if (...) stmt . else ...
    // 现有 Shift (Else)，新来 Reduce (Stmt -> if ...) -> 保留 Shift
    if (existing.type == ACT_SHIFT && newAction.type == ACT_REDUCE) {
        // C语言规则: else 总是与最近的 if 匹配。
        // 所以我们优先 Shift (读取 else)，而不是 Reduce (结束 if 语句)。
        // 保持 existing 不变。
        return; 
    }
    
    // 场景 2: 现有 Reduce，新来 Shift -> 覆盖为 Shift
    if (existing.type == ACT_REDUCE && newAction.type == ACT_SHIFT) {
        actionTable[state][symbol] = newAction;
        return;
    }
    
    // 场景 3: 归约-归约冲突 (Reduce-Reduce Conflict) -> 真正的错误
    if (existing.type == ACT_REDUCE && newAction.type == ACT_REDUCE) {
        cerr << "!! 严重冲突 (State " << state << ", Symbol " << grammar->getSymbolName(symbol) << "): ";
        cerr << "Reduce " << existing.target << " vs Reduce " << newAction.target << endl;
        // 这里可以根据产生式 ID 大小或其他规则强行选一个，或者报错
        // 比如选编号小的:
        if (newAction.target < existing.target) {
            actionTable[state][symbol] = newAction;
        }
    }
}

// ==========================================
// 5. 打印表
// ==========================================
void SLRTable::printTable() {
    cout << "\n===== SLR Table (" << states.size() << " states) =====\n";
    
    // 统计一下动作数量
    int shiftCnt = 0, reduceCnt = 0;
    
    for (auto const& [state, acts] : actionTable) {
        for (auto const& [sym, act] : acts) {
            if (act.type == ACT_SHIFT) shiftCnt++;
            if (act.type == ACT_REDUCE) reduceCnt++;
        }
    }
    
    cout << "Total Shifts: " << shiftCnt << ", Total Reduces: " << reduceCnt << endl;
    cout << "------------------------------------------------------" << endl;
    
    // 简单展示前 20 个状态的动作，避免刷屏
    int count = 0;
    for (auto const& [state, acts] : actionTable) {
        if (count++ > 20) {
            cout << "... (more states hidden) ..." << endl;
            break;
        }
        for (auto const& [sym, act] : acts) {
            cout << "State " << state << " on '" << grammar->getSymbolName(sym) << "' -> ";
            if (act.type == ACT_SHIFT) cout << "Shift " << act.target;
            else if (act.type == ACT_REDUCE) cout << "Reduce (" << act.target << ")";
            else if (act.type == ACT_ACCEPT) cout << "ACCEPT";
            cout << endl;
        }
    }
}