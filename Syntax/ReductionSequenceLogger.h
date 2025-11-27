#ifndef REDUCTION_SEQUENCE_LOGGER_H
#define REDUCTION_SEQUENCE_LOGGER_H

#include "GrammarAnalyzer.h"
#include "Parser.h"
#include <vector>
#include <string>
#include <fstream>

using namespace std;

// 规约序列记录条目
struct ReductionSequenceEntry {
    int step;                    // 序号
    string stackTopSymbol;       // 栈顶符号
    string inputSymbol;          // 面临输入符号
    string action;               // 执行动作: "move", "reduction", "accept"
};

// 规约序列记录器类 - 完全隔离，不干扰 Parser 的核心逻辑
class ReductionSequenceLogger {
private:
    GrammarAnalyzer* grammar;    // 用于获取符号名称
    vector<ReductionSequenceEntry> entries;
    int currentStep;
    
    // 格式化符号名称为用户需要的格式
    string formatSymbolName(int symbolId, const string& tokenText = "");
    
public:
    ReductionSequenceLogger(GrammarAnalyzer* ga) : grammar(ga), currentStep(1) {}
    
    // 记录移进动作
    void logShift(int stackTopSymbolId, const string& stackTopText, 
                  int inputSymbolId, const string& inputText);
    
    // 记录归约动作
    void logReduce(int stackTopSymbolId, const string& stackTopText,
                   int inputSymbolId, const string& inputText,
                   int reducedSymbolId);
    
    // 记录接受动作
    void logAccept(int stackTopSymbolId, const string& stackTopText);
    
    // 记录错误动作
    void logError(int stackTopSymbolId, const string& stackTopText,
                  int inputSymbolId, const string& inputText);
    
    // 输出到文件
    void writeToFile(const string& filename);
    
    // 清空记录
    void clear() { entries.clear(); currentStep = 1; }
};

#endif // REDUCTION_SEQUENCE_LOGGER_H

