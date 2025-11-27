#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "SimpleParser.h"
#include "complie_ir/include/Module.h"
#include "complie_ir/include/IRbuilder.h"
#include "complie_ir/include/IRprinter.h"
#include "complie_ir/include/Constant.h"
#include "complie_ir/include/Type.h"
#include "complie_ir/include/Function.h"
#include "complie_ir/include/BasicBlock.h"
#include <memory>
#include <map>
#include <string>

class CodeGenerator {
private:
    Module* module;
    IRBuilder* builder;
    Function* current_function;
    BasicBlock* current_block;
    map<string, Value*> symbol_table;
    map<string, Value*> global_vars;
    
public:
    CodeGenerator();
    ~CodeGenerator();
    
    void generate(shared_ptr<ASTNode> ast);
    void printIR(const string& filename);
    
private:
    void visitProgram(shared_ptr<ASTNode> node);
    void visitCompUnit(shared_ptr<ASTNode> node);
    void visitDecl(shared_ptr<ASTNode> node);
    void visitConstDecl(shared_ptr<ASTNode> node);
    void visitVarDecl(shared_ptr<ASTNode> node);
    void visitFuncDef(shared_ptr<ASTNode> node);
    void visitBlock(shared_ptr<ASTNode> node);
    void visitBlockItem(shared_ptr<ASTNode> node);
    void visitStmt(shared_ptr<ASTNode> node);
    void visitExp(shared_ptr<ASTNode> node);
    void visitAddExp(shared_ptr<ASTNode> node);
    void visitMulExp(shared_ptr<ASTNode> node);
    void visitUnaryExp(shared_ptr<ASTNode> node);
    void visitPrimaryExp(shared_ptr<ASTNode> node);
    void visitNumber(shared_ptr<ASTNode> node);
    void visitLVal(shared_ptr<ASTNode> node);
    void visitCond(shared_ptr<ASTNode> node);
    void visitLOrExp(shared_ptr<ASTNode> node);
    void visitLAndExp(shared_ptr<ASTNode> node);
    void visitEqExp(shared_ptr<ASTNode> node);
    void visitRelExp(shared_ptr<ASTNode> node);
    
    Value* visitExpressionNode(shared_ptr<ASTNode> node);
};

#endif
