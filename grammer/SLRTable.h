#ifndef SLR_TABLE_H
#define SLR_TABLE_H

#include "GrammarAnalyzer.h"
#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <iomanip>

using namespace std;

// 1. 定义项目 (Item): A -> alpha . beta
// 例如: Stmt -> if . ( Cond )
struct Item {
    int prodId;   // 产生式编号 (对应 GrammarAnalyzer 中的 productions 下标+1)
    int dotPos;   // 圆点位置 (0 表示在最左边)
    
    // 重载 < 运算符，以便放入 set/map
    bool operator<(const Item& other) const {
        if (prodId != other.prodId) return prodId < other.prodId;
        return dotPos < other.dotPos;
    }
    
    bool operator==(const Item& other) const {
        return prodId == other.prodId && dotPos == other.dotPos;
    }
};

// 2. 动作类型
enum ActionType {
    ACT_ERROR = 0,
    ACT_SHIFT,  // 移进
    ACT_REDUCE, // 归约
    ACT_ACCEPT  // 接受
};

// 表项内容
struct Action {
    ActionType type;
    int target; // 对于 Shift 是状态 ID；对于 Reduce 是产生式 ID
};

class SLRTable {
public:
    GrammarAnalyzer* grammar; // 引用之前的分析器
    
    // 项目集规范族 (Canonical Collection) -> 即所有的状态
    // states[i] 就是第 i 个状态包含的所有项目
    vector<set<Item>> states; 
    
    // 分析表: actionTable[state_id][terminal_id] = Action
    map<int, map<int, Action>> actionTable;
    
    // GOTO 表: gotoTable[state_id][non_terminal_id] = next_state_id
    map<int, map<int, int>> gotoTable;

    SLRTable(GrammarAnalyzer* g) : grammar(g) {}

    // 核心构建函数
    void buildTable();
    
    // 打印分析表 (用于调试)
    void printTable(); 

private:
    // 计算闭包
    set<Item> getClosure(const set<Item>& I);
    
    // 计算 GOTO(I, X)
    set<Item> getGoto(const set<Item>& I, int symbol);
    
    // 辅助：向表中添加动作，并处理冲突
    void addAction(int state, int symbol, Action newAction);
};

#endif