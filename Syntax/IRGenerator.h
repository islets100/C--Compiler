#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "GrammarAnalyzer.h"
#include "SyntaxTree.h"

#include "../complie_ir/include/IRbuilder.h"
#include "../complie_ir/include/Module.h"
#include "../complie_ir/include/Function.h"
#include "../complie_ir/include/BasicBlock.h"
#include "../complie_ir/include/Constant.h"

class IRGenerator {
public:
    enum class ValueKind { INT, FLOAT, BOOL, VOID };

    explicit IRGenerator(GrammarAnalyzer *grammar);
    bool generate(TreeNode *root, const std::string &outputPath);

private:
    struct VarInfo {
        Value *address = nullptr;
        ValueKind type = ValueKind::INT;
        bool isConst = false;
        bool isGlobal = false;
    };

    struct ExprValue {
        Value *value = nullptr;
        ValueKind type = ValueKind::INT;
    };

    struct FunctionInfo {
        Function *func = nullptr;
        ValueKind returnType = ValueKind::VOID;
        std::vector<ValueKind> paramTypes;
    };

    GrammarAnalyzer *grammar_;
    std::unique_ptr<Module> module_;
    std::unique_ptr<IRBuilder> builder_;

    Function *currentFunction_ = nullptr;
    ValueKind currentReturnType_ = ValueKind::VOID;

    std::unordered_map<std::string, VarInfo> globalVars_;
    std::vector<std::unordered_map<std::string, VarInfo>> scopeStack_;
    std::unordered_map<std::string, FunctionInfo> functions_;
    
    int ifLabelCounter_ = 0;  // 用于生成唯一的if-else标签名

    // entry helpers
    void visitProgram(TreeNode *node);
    void visitCompUnit(TreeNode *node);
    void visitUnitList(TreeNode *node);
    void visitUnit(TreeNode *node);

    void registerFunctions(TreeNode *node);
    void registerFunctionDef(TreeNode *node);

    void visitDecl(TreeNode *node, bool isGlobal);
    void visitConstDecl(TreeNode *node, bool isGlobal);
    void visitVarDecl(TreeNode *node, bool isGlobal);

    void visitFuncDef(TreeNode *node);
    void visitBlock(TreeNode *node, bool isFunctionBody);
    void visitBlockItem(TreeNode *node);
    void visitStmt(TreeNode *node);

    ExprValue evalExp(TreeNode *node);
    ExprValue evalLOr(TreeNode *node);
    ExprValue evalLAnd(TreeNode *node);
    ExprValue evalEq(TreeNode *node);
    ExprValue evalRel(TreeNode *node);
    ExprValue evalAdd(TreeNode *node);
    ExprValue evalMul(TreeNode *node);
    ExprValue evalUnary(TreeNode *node);
    ExprValue evalPrimary(TreeNode *node);

    ExprValue evalLVal(TreeNode *node);

    ValueKind parseBType(TreeNode *node);
    std::string parseIdent(TreeNode *node);

    void pushScope();
    void popScope();
    VarInfo *lookupVar(const std::string &name);
    VarInfo *defineLocal(const std::string &name, const VarInfo &info);
    FunctionInfo *lookupFunction(const std::string &name);

    ExprValue ensureInt(const ExprValue &val);
    ExprValue ensureBool(const ExprValue &val);
    ExprValue castBoolToInt(const ExprValue &val);

    Value *buildCondValue(TreeNode *node);

    void handleAssignment(TreeNode *lvalNode, ExprValue rhs);

    Constant *buildConstant(ValueKind kind, const std::string &literal);

    void finalizeCurrentFunction();
};

#endif

