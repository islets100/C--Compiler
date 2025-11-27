#include "ReductionSequenceLogger.h"
#include <algorithm>
#include <cctype>

// 格式化符号名称为用户需要的格式
string ReductionSequenceLogger::formatSymbolName(int symbolId, const string& tokenText) {
    if (!grammar) return "?";
    
    string name = grammar->getSymbolName(symbolId);
    
    // 如果是终结符，使用原始名称
    if (symbolId < 50) {
        // 根据用户的示例格式进行转换
        if (symbolId == SYM_ID) {
            return "Ident";
        } else if (symbolId == SYM_INT_CONST) {
            return "IntConst";
        } else if (symbolId == SYM_FLOAT_CONST) {
            return "FloatConst";
        } else {
            // 其他终结符直接返回名称
            return name;
        }
    } else {
        // 非终结符：转换为首字母小写的格式（如 BType -> bType）
        if (!name.empty()) {
            string formatted = name;
            formatted[0] = tolower(formatted[0]);
            return formatted;
        }
        return name;
    }
}

// 记录移进动作
void ReductionSequenceLogger::logShift(int stackTopSymbolId, const string& stackTopText,
                                       int inputSymbolId, const string& inputText) {
    ReductionSequenceEntry entry;
    entry.step = currentStep++;
    
    // 栈顶符号：移进后，栈顶是刚移进的符号，使用其文本值
    if (!stackTopText.empty() && stackTopSymbolId < 50) {
        entry.stackTopSymbol = stackTopText;  // 直接使用文本值，如 "int", "a"
    } else {
        entry.stackTopSymbol = formatSymbolName(stackTopSymbolId, stackTopText);
    }
    
    // 面临输入符号：下一个输入符号，使用其文本值
    if (!inputText.empty() && inputSymbolId < 50 && inputSymbolId != SYM_EOF) {
        entry.inputSymbol = inputText;  // 直接使用文本值
    } else {
        entry.inputSymbol = formatSymbolName(inputSymbolId, inputText);
    }
    
    entry.action = "move";
    entries.push_back(entry);
}

// 记录归约动作
void ReductionSequenceLogger::logReduce(int stackTopSymbolId, const string& stackTopText,
                                        int inputSymbolId, const string& inputText,
                                        int reducedSymbolId) {
    ReductionSequenceEntry entry;
    entry.step = currentStep++;
    
    // 归约后，栈顶符号是归约产生的非终结符（使用格式化名称，如 bType）
    entry.stackTopSymbol = formatSymbolName(reducedSymbolId, "");
    
    // 面临输入符号：当前输入符号的文本值
    if (!inputText.empty() && inputSymbolId < 50) {
        entry.inputSymbol = inputText;  // 直接使用文本值，如 "int", "a"
    } else {
        entry.inputSymbol = formatSymbolName(inputSymbolId, inputText);
    }
    
    entry.action = "reduction";
    entries.push_back(entry);
}

// 记录接受动作
void ReductionSequenceLogger::logAccept(int stackTopSymbolId, const string& stackTopText) {
    ReductionSequenceEntry entry;
    entry.step = currentStep++;
    entry.stackTopSymbol = formatSymbolName(stackTopSymbolId, stackTopText);
    entry.inputSymbol = "$";  // EOF
    entry.action = "accept";
    entries.push_back(entry);
}

// 记录错误动作
void ReductionSequenceLogger::logError(int stackTopSymbolId, const string& stackTopText,
                                       int inputSymbolId, const string& inputText) {
    ReductionSequenceEntry entry;
    entry.step = currentStep++;
    
    // 栈顶符号：当前栈顶符号（如果有）
    if (stackTopSymbolId > 0) {
        if (!stackTopText.empty() && stackTopSymbolId < 50) {
            entry.stackTopSymbol = stackTopText;
        } else {
            entry.stackTopSymbol = formatSymbolName(stackTopSymbolId, stackTopText);
        }
    } else {
        entry.stackTopSymbol = "";  // 栈为空或初始状态
    }
    
    // 面临输入符号：出错的输入符号
    if (!inputText.empty() && inputSymbolId < 50) {
        entry.inputSymbol = inputText;  // 直接使用文本值，如 "!="
    } else {
        entry.inputSymbol = formatSymbolName(inputSymbolId, inputText);
    }
    
    entry.action = "error";
    entries.push_back(entry);
}

// 输出到文件
void ReductionSequenceLogger::writeToFile(const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot open file " << filename << " for writing reduction sequence." << endl;
        return;
    }
    
    for (const auto& entry : entries) {
        file << entry.step << "\t"
             << entry.stackTopSymbol << "#" << entry.inputSymbol << "\t"
             << entry.action << endl;
    }
    
    file.close();
}

