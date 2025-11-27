#include "CodeGenerator.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

CodeGenerator::CodeGenerator() {
    module = new Module("sysy2022_compiler");
    current_function = nullptr;
    current_block = nullptr;
    builder = nullptr;
}

CodeGenerator::~CodeGenerator() {
    if (builder) delete builder;
    if (module) delete module;
}

void CodeGenerator::generate(shared_ptr<ASTNode> ast) {
    if (!ast) return;
    
    if (ast->name == "Program") {
        visitProgram(ast);
    } else if (ast->name == "compUnit") {
        visitCompUnit(ast);
    }
}

void CodeGenerator::visitProgram(shared_ptr<ASTNode> node) {
    for (auto& child : node->children) {
        if (child->name == "compUnit") {
            visitCompUnit(child);
        }
    }
}

void CodeGenerator::visitCompUnit(shared_ptr<ASTNode> node) {
    for (auto& child : node->children) {
        if (child->name == "decl") {
            visitDecl(child);
        } else if (child->name == "funcDef") {
            visitFuncDef(child);
        }
    }
}

void CodeGenerator::visitDecl(shared_ptr<ASTNode> node) {
    for (auto& child : node->children) {
        if (child->name == "constDecl") {
            visitConstDecl(child);
        } else if (child->name == "varDecl") {
            visitVarDecl(child);
        }
    }
}

void CodeGenerator::visitConstDecl(shared_ptr<ASTNode> node) {
    // 处理常量声明
    // const btype constDef (',' constDef)* ';'
    // 提取类型和常量定义
}

void CodeGenerator::visitVarDecl(shared_ptr<ASTNode> node) {
    // 处理变量声明
    // btype varDef (',' varDef)* ';'
    // 创建全局变量或局部变量
}

void CodeGenerator::visitFuncDef(shared_ptr<ASTNode> node) {
    // 处理函数定义
    // funcType Ident '(' (funcFParams)? ')' block
    
    // 获取函数名和返回类型
    string func_name = "main";
    Type* return_type = module->get_int32_type();
    
    // 创建函数类型
    vector<Type*> param_types;
    FunctionType* func_type = FunctionType::get(return_type, param_types);
    
    // 创建函数
    Function* func = Function::create(func_type, func_name, module);
    module->add_function(func);
    
    current_function = func;
    
    // 创建入口基本块
    BasicBlock* entry_block = BasicBlock::create(module, "main_ENTRY", func);
    func->add_basic_block(entry_block);
    current_block = entry_block;
    
    if (builder) delete builder;
    builder = new IRBuilder(entry_block, module);
    builder->set_curFunc(func);
    
    // 访问函数体
    for (auto& child : node->children) {
        if (child->name == "block") {
            visitBlock(child);
        }
    }
    
    // 如果没有return语句，添加默认return 0
    if (builder) {
        builder->create_ret(ConstantInt::get(0, module));
    }
}

void CodeGenerator::visitBlock(shared_ptr<ASTNode> node) {
    // 处理代码块
    // '{' (blockItem)* '}'
    for (auto& child : node->children) {
        if (child->name == "blockItem") {
            visitBlockItem(child);
        }
    }
}

void CodeGenerator::visitBlockItem(shared_ptr<ASTNode> node) {
    for (auto& child : node->children) {
        if (child->name == "decl") {
            visitDecl(child);
        } else if (child->name == "stmt") {
            visitStmt(child);
        }
    }
}

void CodeGenerator::visitStmt(shared_ptr<ASTNode> node) {
    // 处理语句
    // lVal '=' exp ';' | (exp)? ';' | block | 'if' '(' cond ')' stmt ('else' stmt)? | 'return' (exp)? ';'
    
    if (node->children.empty()) return;
    
    auto& first_child = node->children[0];
    
    if (first_child->name == "lVal") {
        // 赋值语句
        // lVal '=' exp ';'
        if (node->children.size() >= 3) {
            Value* lval = visitExpressionNode(first_child);
            Value* rval = visitExpressionNode(node->children[2]);
            if (builder && lval && rval) {
                builder->create_store(rval, lval);
            }
        }
    } else if (first_child->name == "exp") {
        // 表达式语句
        visitExp(first_child);
    } else if (first_child->name == "block") {
        // 块语句
        visitBlock(first_child);
    } else if (first_child->value == "if") {
        // if语句
        // 'if' '(' cond ')' stmt ('else' stmt)?
    } else if (first_child->value == "return") {
        // return语句
        // 'return' (exp)? ';'
        if (node->children.size() > 1 && node->children[1]->name == "exp") {
            Value* ret_val = visitExpressionNode(node->children[1]);
            if (builder && ret_val) {
                builder->create_ret(ret_val);
            }
        } else {
            if (builder) {
                builder->create_ret(module->get_const_int(0));
            }
        }
    }
}

void CodeGenerator::visitExp(shared_ptr<ASTNode> node) {
    for (auto& child : node->children) {
        if (child->name == "addExp") {
            visitAddExp(child);
        }
    }
}

void CodeGenerator::visitAddExp(shared_ptr<ASTNode> node) {
    // addExp -> mulExp | addExp ('+' | '-') mulExp
    // 这是递归下降的一部分，实际处理在visitExpressionNode中
}

void CodeGenerator::visitMulExp(shared_ptr<ASTNode> node) {
    // mulExp -> unaryExp | mulExp ('*' | '/' | '%') unaryExp
    // 这是递归下降的一部分，实际处理在visitExpressionNode中
}

void CodeGenerator::visitUnaryExp(shared_ptr<ASTNode> node) {
    // unaryExp -> primaryExp | Ident '(' (funcRParams)? ')' | unaryOp unaryExp
    // 这是递归下降的一部分，实际处理在visitExpressionNode中
}

void CodeGenerator::visitPrimaryExp(shared_ptr<ASTNode> node) {
    // primaryExp -> '(' exp ')' | lVal | number
    // 这是递归下降的一部分，实际处理在visitExpressionNode中
}

void CodeGenerator::visitNumber(shared_ptr<ASTNode> node) {
    // number -> IntConst | floatConst
    // 这是递归下降的一部分，实际处理在visitExpressionNode中
}

void CodeGenerator::visitLVal(shared_ptr<ASTNode> node) {
    // lVal -> Ident
    // 这是递归下降的一部分，实际处理在visitExpressionNode中
}

void CodeGenerator::visitCond(shared_ptr<ASTNode> node) {
    // cond -> lOrExp
    for (auto& child : node->children) {
        if (child->name == "lOrExp") {
            visitLOrExp(child);
        }
    }
}

void CodeGenerator::visitLOrExp(shared_ptr<ASTNode> node) {
    // lOrExp -> lAndExp | lOrExp '||' lAndExp
}

void CodeGenerator::visitLAndExp(shared_ptr<ASTNode> node) {
    // lAndExp -> eqExp | lAndExp '&&' eqExp
}

void CodeGenerator::visitEqExp(shared_ptr<ASTNode> node) {
    // eqExp -> relExp | eqExp ('==' | '!=') relExp
}

void CodeGenerator::visitRelExp(shared_ptr<ASTNode> node) {
    // relExp -> addExp | relExp ('<' | '>' | '<=' | '>=') addExp
}

Value* CodeGenerator::visitExpressionNode(shared_ptr<ASTNode> node) {
    if (!node) return nullptr;
    
    // 处理各种表达式节点类型
    if (node->name == "lVal") {
        // lVal -> Ident
        if (!node->children.empty() && node->children[0]->name == "Ident") {
            string var_name = node->children[0]->value;
            if (symbol_table.find(var_name) != symbol_table.end()) {
                return symbol_table[var_name];
            }
            if (global_vars.find(var_name) != global_vars.end()) {
                return global_vars[var_name];
            }
        }
        return nullptr;
    } 
    else if (node->name == "number") {
        // number -> IntConst | floatConst
        if (!node->children.empty()) {
            string value_str = node->children[0]->value;
            try {
                if (node->children[0]->name == "IntConst" || 
                    node->children[0]->type == "INT") {
                    int value = stoi(value_str);
                    return ConstantInt::get(value, module);
                } else {
                    // floatConst - 暂时转换为int
                    float fvalue = stof(value_str);
                    return ConstantInt::get((int)fvalue, module);
                }
            } catch (...) {
                return nullptr;
            }
        }
        return nullptr;
    }
    else if (node->name == "INT" || node->name == "FLOAT") {
        // 直接处理词法单元
        try {
            if (node->name == "INT") {
                int value = stoi(node->value);
                return ConstantInt::get(value, module);
            } else {
                float fvalue = stof(node->value);
                return ConstantInt::get((int)fvalue, module);
            }
        } catch (...) {
            return nullptr;
        }
    }
    else if (node->name == "exp" || node->name == "addExp" || 
             node->name == "mulExp" || node->name == "unaryExp" ||
             node->name == "primaryExp") {
        // 递归处理表达式
        for (auto& child : node->children) {
            if (child->name != "OP" && child->name != "SE") {
                Value* result = visitExpressionNode(child);
                if (result) return result;
            }
        }
    }
    else if (node->name == "addExp" && node->children.size() >= 3) {
        // addExp -> addExp ('+' | '-') mulExp
        Value* lhs = visitExpressionNode(node->children[0]);
        Value* rhs = visitExpressionNode(node->children[2]);
        
        if (builder && lhs && rhs) {
            if (node->children[1]->value == "+") {
                return builder->create_iadd(lhs, rhs);
            } else if (node->children[1]->value == "-") {
                return builder->create_isub(lhs, rhs);
            }
        }
    }
    else if (node->name == "mulExp" && node->children.size() >= 3) {
        // mulExp -> mulExp ('*' | '/' | '%') unaryExp
        Value* lhs = visitExpressionNode(node->children[0]);
        Value* rhs = visitExpressionNode(node->children[2]);
        
        if (builder && lhs && rhs) {
            if (node->children[1]->value == "*") {
                return builder->create_imul(lhs, rhs);
            } else if (node->children[1]->value == "/") {
                return builder->create_isdiv(lhs, rhs);
            } else if (node->children[1]->value == "%") {
                return builder->create_irem(lhs, rhs);
            }
        }
    }
    
    return nullptr;
}

void CodeGenerator::printIR(const string& filename) {
    if (!module) return;
    
    ofstream out(filename);
    if (!out.is_open()) {
        cerr << "Cannot open file: " << filename << endl;
        return;
    }
    
    // 输出LLVM IR头部
    out << "; ModuleID = 'sysy2022_compiler'" << endl;
    out << "source_filename = \"./input.sy\"" << endl;
    
    // 输出全局变量
    for (auto& global_var : module->get_global_variable()) {
        out << global_var->print() << endl;
    }
    
    // 输出函数
    for (auto& func : module->get_functions()) {
        out << func->print() << endl;
    }
    
    out.close();
}
