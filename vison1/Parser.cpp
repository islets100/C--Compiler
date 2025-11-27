#include "Parser.h"
#include <algorithm>
#include <queue>

SLRParser::SLRParser() {
    initProductions();
    computeFirstSet();
    computeFollowSet();
    buildLR0Items();
    buildSLRTable();
}

void SLRParser::initProductions() {
    // 按照任务文档中的36条产生式定义
    // 为了简化SLR分析器，我们使用简化的产生式集合
    
    int prod_id = 0;
    
    // 基础产生式
    productions.push_back(Production(prod_id++, "Program", {"compUnit"}));
    productions.push_back(Production(prod_id++, "compUnit", {""})); // epsilon
    productions.push_back(Production(prod_id++, "compUnit", {"decl", "compUnit"}));
    productions.push_back(Production(prod_id++, "compUnit", {"funcDef", "compUnit"}));
    
    productions.push_back(Production(prod_id++, "decl", {"constDecl"}));
    productions.push_back(Production(prod_id++, "decl", {"varDecl"}));
    
    productions.push_back(Production(prod_id++, "constDecl", {"const", "btype", "constDef", ";"}));
    productions.push_back(Production(prod_id++, "constDef", {"Ident", "=", "constInitial"}));
    productions.push_back(Production(prod_id++, "constInitial", {"constExp"}));
    
    productions.push_back(Production(prod_id++, "btype", {"int"}));
    productions.push_back(Production(prod_id++, "btype", {"float"}));
    
    productions.push_back(Production(prod_id++, "varDecl", {"btype", "varDef", ";"}));
    productions.push_back(Production(prod_id++, "varDef", {"Ident"}));
    productions.push_back(Production(prod_id++, "varDef", {"Ident", "=", "initVal"}));
    
    productions.push_back(Production(prod_id++, "initVal", {"exp"}));
    
    productions.push_back(Production(prod_id++, "funcDef", {"funcType", "Ident", "(", ")", "block"}));
    productions.push_back(Production(prod_id++, "funcType", {"void"}));
    productions.push_back(Production(prod_id++, "funcType", {"int"}));
    
    productions.push_back(Production(prod_id++, "block", {"{", "blockItems", "}"}));
    productions.push_back(Production(prod_id++, "blockItems", {""})); // epsilon
    productions.push_back(Production(prod_id++, "blockItems", {"blockItem", "blockItems"}));
    
    productions.push_back(Production(prod_id++, "blockItem", {"decl"}));
    productions.push_back(Production(prod_id++, "blockItem", {"stmt"}));
    
    productions.push_back(Production(prod_id++, "stmt", {"lVal", "=", "exp", ";"}));
    productions.push_back(Production(prod_id++, "stmt", {"exp", ";"}));
    productions.push_back(Production(prod_id++, "stmt", {";"}));
    productions.push_back(Production(prod_id++, "stmt", {"block"}));
    productions.push_back(Production(prod_id++, "stmt", {"return", ";"}));
    productions.push_back(Production(prod_id++, "stmt", {"return", "exp", ";"}));
    
    productions.push_back(Production(prod_id++, "exp", {"addExp"}));
    productions.push_back(Production(prod_id++, "cond", {"lOrExp"}));
    productions.push_back(Production(prod_id++, "lVal", {"Ident"}));
    
    productions.push_back(Production(prod_id++, "primaryExp", {"(", "exp", ")"}));
    productions.push_back(Production(prod_id++, "primaryExp", {"lVal"}));
    productions.push_back(Production(prod_id++, "primaryExp", {"number"}));
    
    productions.push_back(Production(prod_id++, "number", {"IntConst"}));
    productions.push_back(Production(prod_id++, "number", {"floatConst"}));
    
    productions.push_back(Production(prod_id++, "unaryExp", {"primaryExp"}));
    productions.push_back(Production(prod_id++, "unaryOp", {"+"}));
    productions.push_back(Production(prod_id++, "unaryOp", {"-"}));
    
    productions.push_back(Production(prod_id++, "mulExp", {"unaryExp"}));
    productions.push_back(Production(prod_id++, "mulExp", {"mulExp", "*", "unaryExp"}));
    productions.push_back(Production(prod_id++, "mulExp", {"mulExp", "/", "unaryExp"}));
    productions.push_back(Production(prod_id++, "mulExp", {"mulExp", "%", "unaryExp"}));
    
    productions.push_back(Production(prod_id++, "addExp", {"mulExp"}));
    productions.push_back(Production(prod_id++, "addExp", {"addExp", "+", "mulExp"}));
    productions.push_back(Production(prod_id++, "addExp", {"addExp", "-", "mulExp"}));
    
    productions.push_back(Production(prod_id++, "relExp", {"addExp"}));
    productions.push_back(Production(prod_id++, "relExp", {"relExp", "<", "addExp"}));
    productions.push_back(Production(prod_id++, "relExp", {"relExp", ">", "addExp"}));
    productions.push_back(Production(prod_id++, "relExp", {"relExp", "<=", "addExp"}));
    productions.push_back(Production(prod_id++, "relExp", {"relExp", ">=", "addExp"}));
    
    productions.push_back(Production(prod_id++, "eqExp", {"relExp"}));
    productions.push_back(Production(prod_id++, "eqExp", {"eqExp", "==", "relExp"}));
    productions.push_back(Production(prod_id++, "eqExp", {"eqExp", "!=", "relExp"}));
    
    productions.push_back(Production(prod_id++, "lAndExp", {"eqExp"}));
    productions.push_back(Production(prod_id++, "lAndExp", {"lAndExp", "&&", "eqExp"}));
    
    productions.push_back(Production(prod_id++, "lOrExp", {"lAndExp"}));
    productions.push_back(Production(prod_id++, "lOrExp", {"lOrExp", "||", "lAndExp"}));
    
    productions.push_back(Production(prod_id++, "constExp", {"addExp"}));
    
    // 收集终结符和非终结符
    for (auto& prod : productions) {
        non_terminals.insert(prod.left);
        for (auto& sym : prod.right) {
            if (sym != "" && sym[0] >= 'A' && sym[0] <= 'Z') {
                non_terminals.insert(sym);
            } else if (sym != "") {
                terminals.insert(sym);
            }
        }
    }
    
    // 添加特殊终结符
    terminals.insert("EOF");
    terminals.insert("Ident");
    terminals.insert("IntConst");
    terminals.insert("floatConst");
}

void SLRParser::computeFirstSet() {
    // 初始化FIRST集
    for (auto& nt : non_terminals) {
        first_set[nt] = set<string>();
    }
    
    // 对于终结符，FIRST集就是它本身
    for (auto& t : terminals) {
        first_set[t].insert(t);
    }
    
    // 迭代计算FIRST集
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& prod : productions) {
            int old_size = first_set[prod.left].size();
            
            for (auto& sym : prod.right) {
                if (sym == "") {
                    first_set[prod.left].insert("");
                    break;
                }
                
                auto& sym_first = first_set[sym];
                for (auto& f : sym_first) {
                    if (f != "") {
                        first_set[prod.left].insert(f);
                    }
                }
                
                if (sym_first.find("") == sym_first.end()) {
                    break;
                }
            }
            
            if (first_set[prod.left].size() > old_size) {
                changed = true;
            }
        }
    }
}

void SLRParser::computeFollowSet() {
    // 初始化FOLLOW集
    for (auto& nt : non_terminals) {
        follow_set[nt] = set<string>();
    }
    
    // 开始符号的FOLLOW集包含EOF
    follow_set["Program"].insert("EOF");
    
    // 迭代计算FOLLOW集
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& prod : productions) {
            for (int i = 0; i < prod.right.size(); i++) {
                string& sym = prod.right[i];
                if (non_terminals.find(sym) == non_terminals.end()) continue;
                
                int old_size = follow_set[sym].size();
                
                // 查看sym后面的符号
                for (int j = i + 1; j < prod.right.size(); j++) {
                    string& next_sym = prod.right[j];
                    auto& next_first = first_set[next_sym];
                    
                    for (auto& f : next_first) {
                        if (f != "") {
                            follow_set[sym].insert(f);
                        }
                    }
                    
                    if (next_first.find("") == next_first.end()) {
                        break;
                    }
                }
                
                // 如果sym是最后一个符号或后面都是epsilon
                bool all_epsilon = true;
                for (int j = i + 1; j < prod.right.size(); j++) {
                    if (first_set[prod.right[j]].find("") == first_set[prod.right[j]].end()) {
                        all_epsilon = false;
                        break;
                    }
                }
                
                if (all_epsilon) {
                    for (auto& f : follow_set[prod.left]) {
                        follow_set[sym].insert(f);
                    }
                }
                
                if (follow_set[sym].size() > old_size) {
                    changed = true;
                }
            }
        }
    }
}

set<Item> SLRParser::closure(const set<Item>& items) {
    set<Item> result = items;
    queue<Item> worklist;
    
    for (auto& item : items) {
        worklist.push(item);
    }
    
    while (!worklist.empty()) {
        Item item = worklist.front();
        worklist.pop();
        
        // 获取点后面的符号
        if (item.dot_pos < productions[item.prod_id].right.size()) {
            string next_sym = productions[item.prod_id].right[item.dot_pos];
            
            // 如果是非终结符，添加所有以该非终结符开头的产生式
            if (non_terminals.find(next_sym) != non_terminals.end()) {
                for (int i = 0; i < productions.size(); i++) {
                    if (productions[i].left == next_sym) {
                        Item new_item(i, 0, item.lookahead);
                        if (result.find(new_item) == result.end()) {
                            result.insert(new_item);
                            worklist.push(new_item);
                        }
                    }
                }
            }
        }
    }
    
    return result;
}

set<Item> SLRParser::goTo(const set<Item>& items, const string& symbol) {
    set<Item> result;
    
    for (auto& item : items) {
        if (item.dot_pos < productions[item.prod_id].right.size() &&
            productions[item.prod_id].right[item.dot_pos] == symbol) {
            Item new_item(item.prod_id, item.dot_pos + 1, item.lookahead);
            result.insert(new_item);
        }
    }
    
    return closure(result);
}

void SLRParser::buildLR0Items() {
    // 初始项目集
    set<Item> initial_items;
    initial_items.insert(Item(0, 0, "EOF")); // Program -> .compUnit, EOF
    
    lr0_items.push_back(closure(initial_items));
    
    queue<int> worklist;
    worklist.push(0);
    
    while (!worklist.empty()) {
        int state_id = worklist.front();
        worklist.pop();
        
        set<string> symbols;
        for (auto& item : lr0_items[state_id]) {
            if (item.dot_pos < productions[item.prod_id].right.size()) {
                symbols.insert(productions[item.prod_id].right[item.dot_pos]);
            }
        }
        
        for (auto& symbol : symbols) {
            set<Item> next_items = goTo(lr0_items[state_id], symbol);
            
            if (next_items.empty()) continue;
            
            // 检查是否已存在
            int next_state_id = -1;
            for (int i = 0; i < lr0_items.size(); i++) {
                if (lr0_items[i] == next_items) {
                    next_state_id = i;
                    break;
                }
            }
            
            if (next_state_id == -1) {
                next_state_id = lr0_items.size();
                lr0_items.push_back(next_items);
                worklist.push(next_state_id);
            }
        }
    }
}

void SLRParser::buildSLRTable() {
    for (int state_id = 0; state_id < lr0_items.size(); state_id++) {
        for (auto& item : lr0_items[state_id]) {
            if (item.dot_pos < productions[item.prod_id].right.size()) {
                string next_sym = productions[item.prod_id].right[item.dot_pos];
                
                // 计算goto状态
                set<Item> next_items = goTo(lr0_items[state_id], next_sym);
                int next_state = -1;
                for (int i = 0; i < lr0_items.size(); i++) {
                    if (lr0_items[i] == next_items) {
                        next_state = i;
                        break;
                    }
                }
                
                if (terminals.find(next_sym) != terminals.end()) {
                    // Shift
                    action_table[{state_id, next_sym}] = ActionEntry("shift", next_state);
                } else {
                    // Goto
                    goto_table[{state_id, next_sym}] = next_state;
                }
            } else {
                // 点在最后
                if (item.prod_id == 0) {
                    // Accept
                    action_table[{state_id, "EOF"}] = ActionEntry("accept", 0);
                } else {
                    // Reduce
                    for (auto& follow_sym : follow_set[productions[item.prod_id].left]) {
                        action_table[{state_id, follow_sym}] = ActionEntry("reduce", item.prod_id);
                    }
                }
            }
        }
    }
}

shared_ptr<ASTNode> SLRParser::parse(vector<Token>& tokens) {
    stack<int> state_stack;
    stack<shared_ptr<ASTNode>> node_stack;
    
    state_stack.push(0);
    
    int token_idx = 0;
    int step = 1;
    
    while (true) {
        int current_state = state_stack.top();
        string current_token = (token_idx < tokens.size()) ? tokens[token_idx].type : "EOF";
        
        auto action_it = action_table.find({current_state, current_token});
        if (action_it == action_table.end()) {
            cerr << "Parse error at token " << token_idx << endl;
            return nullptr;
        }
        
        ActionEntry action = action_it->second;
        
        cout << step++ << "\t";
        cout << "state:" << current_state << "#" << current_token << "\t";
        cout << action.action;
        if (action.action == "shift") {
            cout << " " << action.state << endl;
            state_stack.push(action.state);
            
            auto node = make_shared<ASTNode>(tokens[token_idx].type, tokens[token_idx].value);
            node_stack.push(node);
            token_idx++;
        } else if (action.action == "reduce") {
            cout << " " << action.state << endl;
            Production& prod = productions[action.state];
            
            vector<shared_ptr<ASTNode>> children;
            for (int i = 0; i < prod.right.size(); i++) {
                if (!node_stack.empty()) {
                    children.push_back(node_stack.top());
                    node_stack.pop();
                }
                if (!state_stack.empty()) {
                    state_stack.pop();
                }
            }
            reverse(children.begin(), children.end());
            
            auto new_node = make_shared<ASTNode>(prod.left, "", action.state);
            for (auto& child : children) {
                new_node->addChild(child);
            }
            node_stack.push(new_node);
            
            int prev_state = state_stack.top();
            auto goto_it = goto_table.find({prev_state, prod.left});
            if (goto_it != goto_table.end()) {
                state_stack.push(goto_it->second);
            }
        } else if (action.action == "accept") {
            cout << endl;
            return node_stack.top();
        } else {
            cout << endl;
            cerr << "Parse error!" << endl;
            return nullptr;
        }
    }
}

void SLRParser::printProductions() {
    for (auto& prod : productions) {
        cout << prod.id << ": " << prod.left << " -> ";
        for (auto& sym : prod.right) {
            cout << sym << " ";
        }
        cout << endl;
    }
}

void SLRParser::printFirstFollow() {
    cout << "FIRST Sets:" << endl;
    for (auto& [sym, first] : first_set) {
        cout << sym << ": {";
        for (auto& f : first) {
            cout << f << " ";
        }
        cout << "}" << endl;
    }
    
    cout << "\nFOLLOW Sets:" << endl;
    for (auto& [sym, follow] : follow_set) {
        cout << sym << ": {";
        for (auto& f : follow) {
            cout << f << " ";
        }
        cout << "}" << endl;
    }
}

void SLRParser::printParseTrace(const vector<Token>& tokens) {
    cout << "Parse trace:" << endl;
}
