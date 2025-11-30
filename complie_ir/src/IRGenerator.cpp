#include "IRGenerator.h"

#include <fstream>
#include <iostream>
#include <functional>
#include <stdexcept>

namespace {

Type *getIRType(Module *module, IRGenerator::ValueKind kind) {
    switch (kind) {
    case IRGenerator::ValueKind::INT:
        return module->get_int32_type();
    case IRGenerator::ValueKind::FLOAT:
        return module->get_float_type();
    case IRGenerator::ValueKind::BOOL:
        return module->get_int1_type();
    case IRGenerator::ValueKind::VOID:
    default:
        return module->get_void_type();
    }
}

} // namespace

using ValueKind = IRGenerator::ValueKind;

IRGenerator::IRGenerator(GrammarAnalyzer *grammar) : grammar_(grammar) {}

bool IRGenerator::generate(TreeNode *root, const std::string &outputPath) {
    std::cerr << "    [DEBUG] IRGenerator::generate called, root=" << (root ? "valid" : "null") << std::endl;
    if (!root) {
        std::cerr << "    [DEBUG] Root is null, returning false" << std::endl;
        return false;
    }
    std::cerr << "    [DEBUG] Root symbolId=" << root->symbolId << " (NON_COMPUNIT=" << NON_COMPUNIT << ", NON_PROGRAM=" << NON_PROGRAM << ")" << std::endl;
    
    module_ = std::make_unique<Module>("sysy2022_complier");
    // 从 outputPath 推断源文件名，例如 output/1.ll -> ./input/1.sy
    std::string sourceFileName = "./input/";
    size_t lastSlash = outputPath.find_last_of("/\\");
    size_t lastDot = outputPath.find_last_of(".");
    if (lastSlash != std::string::npos && lastDot != std::string::npos && lastDot > lastSlash) {
        std::string baseName = outputPath.substr(lastSlash + 1, lastDot - lastSlash - 1);
        sourceFileName += baseName + ".sy";
    } else {
        sourceFileName += "input.sy";
    }
    module_->set_source_file_name(sourceFileName);
    builder_ = std::make_unique<IRBuilder>(nullptr, module_.get());
    currentFunction_ = nullptr;
    currentReturnType_ = ValueKind::VOID;
    globalVars_.clear();
    scopeStack_.clear();
    functions_.clear();
    ifLabelCounter_ = 0;  // 初始化标签计数器

    try {
        std::cerr << "    [DEBUG] Registering functions..." << std::endl;
        registerFunctions(root);
        std::cerr << "    [DEBUG] Functions registered. Starting IR generation..." << std::endl;
        // 检查root节点类型：如果是CompUnit则直接访问，如果是Program则访问其children
        if (root->symbolId == NON_COMPUNIT) {
            std::cerr << "    [DEBUG] Visiting CompUnit..." << std::endl;
            visitCompUnit(root);
        } else if (root->symbolId == NON_PROGRAM) {
            std::cerr << "    [DEBUG] Visiting Program..." << std::endl;
            visitProgram(root);
        } else {
            std::cerr << "    Error: Unexpected root node type: " << root->symbolId 
                      << " (expected NON_COMPUNIT=" << NON_COMPUNIT 
                      << " or NON_PROGRAM=" << NON_PROGRAM << ")" << std::endl;
            return false;
        }
        std::cerr << "    [DEBUG] IR generation completed successfully." << std::endl;
    } catch (const std::runtime_error &ex) {
        std::cerr << "    [ERROR] Runtime error during IR generation: " << ex.what() << std::endl;
        return false;
    } catch (const std::exception &ex) {
        std::cerr << "    [ERROR] Exception during IR generation: " << ex.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "    [ERROR] Unknown exception during IR generation" << std::endl;
        return false;
    }

    module_->set_print_name();
    std::ofstream ofs(outputPath);
    if (!ofs.is_open()) {
        return false;
    }
    ofs << module_->print();
    ofs.flush();
    ofs.close();
    return true;
}

void IRGenerator::visitProgram(TreeNode *node) {
    if (!node || node->children.empty()) {
        return;
    }
    visitCompUnit(node->children[0]);
}

void IRGenerator::visitCompUnit(TreeNode *node) {
    if (!node || node->children.empty()) {
        return;
    }
    visitUnitList(node->children[0]);
}

void IRGenerator::visitUnitList(TreeNode *node) {
    if (!node) {
        return;
    }
    // UnitList -> UnitList Unit (递归情况)
    if (node->children.size() == 2) {
        visitUnitList(node->children[0]);
        if (node->children[1]) {
            visitUnit(node->children[1]);
        }
        return;
    }
    // UnitList -> Unit (单个 Unit 情况)
    if (node->children.size() == 1) {
        visitUnit(node->children[0]);
        return;
    }
    // UnitList -> ε (空列表情况，直接返回)
    // 这种情况在语法树中可能表现为 children 为空
    // 或者有一个 epsilon 节点，但通常不会出现在实际的语法树中
}

void IRGenerator::visitUnit(TreeNode *node) {
    if (!node || node->children.empty()) {
        return;
    }
    // Unit -> Decl | FuncDef
    auto child = node->children[0];
    if (!child) {
        return;
    }
    if (child->symbolId == NON_DECL) {
        visitDecl(child, true);
    } else if (child->symbolId == NON_FUNCDEF) {
        visitFuncDef(child);
    } else {
        // 如果遇到其他类型，可能是语法树结构问题
        std::cerr << "    Warning: visitUnit encountered unexpected node type: " 
                  << child->symbolId << std::endl;
    }
}

void IRGenerator::visitDecl(TreeNode *node, bool isGlobal) {
    if (!node || node->children.empty()) {
        return;
    }
    if (node->children[0]->symbolId == NON_CONSTDECL) {
        visitConstDecl(node->children[0], isGlobal);
    } else if (node->children[0]->symbolId == NON_VARDECL) {
        visitVarDecl(node->children[0], isGlobal);
    }
}

static void collectConstDefs(TreeNode *node, std::vector<TreeNode *> &defs) {
    if (!node) {
        return;
    }
    if (node->symbolId == NON_CONSTDEF) {
        defs.push_back(node);
        return;
    }
    if (node->children.empty()) {
        return;
    }
    if (node->children.size() == 1) {
        collectConstDefs(node->children[0], defs);
    } else if (node->children.size() == 3) {
        collectConstDefs(node->children[0], defs);
        collectConstDefs(node->children[2], defs);
    }
}

static void collectVarDefs(TreeNode *node, std::vector<TreeNode *> &defs) {
    if (!node) {
        return;
    }
    if (node->symbolId == NON_VARDEF) {
        defs.push_back(node);
        return;
    }
    if (node->children.empty()) {
        return;
    }
    if (node->children.size() == 1) {
        collectVarDefs(node->children[0], defs);
    } else if (node->children.size() == 3) {
        collectVarDefs(node->children[0], defs);
        collectVarDefs(node->children[2], defs);
    }
}

static TreeNode *findFirstLiteral(TreeNode *node, SymbolType target) {
    if (!node) {
        return nullptr;
    }
    if (node->symbolId == target) {
        return node;
    }
    for (auto *child : node->children) {
        auto *res = findFirstLiteral(child, target);
        if (res) {
            return res;
        }
    }
    return nullptr;
}

Constant *IRGenerator::buildConstant(ValueKind kind, const std::string &literal) {
    if (kind == ValueKind::INT || kind == ValueKind::BOOL) {
        int val = 0;
        try {
            val = std::stoi(literal);
        } catch (...) {
            val = 0;
        }
        if (kind == ValueKind::BOOL) {
            return ConstantInt::get(val != 0, module_.get());
        }
        return ConstantInt::get(val, module_.get());
    } else if (kind == ValueKind::FLOAT) {
        float fv = 0.0f;
        try {
            fv = std::stof(literal);
        } catch (...) {
            fv = 0.0f;
        }
        return ConstantFloat::get(fv, module_.get());
    }
    return ConstantZero::get(getIRType(module_.get(), kind), module_.get());
}

void IRGenerator::visitConstDecl(TreeNode *node, bool isGlobal) {
    if (!node || node->children.size() < 4) {
        return;
    }
    auto bTypeNode = node->children[1];
    ValueKind kind = parseBType(bTypeNode);
    auto listNode = node->children[2];

    std::vector<TreeNode *> defs;
    collectConstDefs(listNode, defs);

    for (auto *def : defs) {
        if (def->children.size() < 3) {
            continue;
        }
        std::string name = parseIdent(def->children[0]);
        TreeNode *initNode = def->children[2]; // ConstInit
        SymbolType literalType = (kind == ValueKind::FLOAT) ? SYM_FLOAT_CONST : SYM_INT_CONST;
        auto *litNode = findFirstLiteral(initNode, literalType);
        if (!litNode) {
            throw std::runtime_error("Const initializer must be literal");
        }
        Constant *c = buildConstant(kind, litNode->text);
        if (isGlobal) {
            auto gv = GlobalVariable::create(name, module_.get(),
                                             getIRType(module_.get(), kind), true, c);
            VarInfo info{gv, kind, true, true};
            globalVars_[name] = info;
        } else {
            auto allocaInst = builder_->create_alloca(getIRType(module_.get(), kind));
            builder_->create_store(c, allocaInst);
            VarInfo info{allocaInst, kind, true, false};
            defineLocal(name, info);
        }
    }
}

void IRGenerator::visitVarDecl(TreeNode *node, bool isGlobal) {
    if (!node || node->children.size() < 3) {
        return;
    }
    ValueKind kind = parseBType(node->children[0]);
    std::vector<TreeNode *> defs;
    collectVarDefs(node->children[1], defs);

    for (size_t i = 0; i < defs.size(); ++i) {
        auto *def = defs[i];
        if (def->children.empty()) {
            continue;
        }
        std::string name = parseIdent(def->children[0]);
        bool hasInit = def->children.size() > 1;
        if (isGlobal) {
            Constant *initConst = ConstantZero::get(getIRType(module_.get(), kind), module_.get());
            if (hasInit) {
                if (def->children.size() < 3) {
                    throw std::runtime_error("VarDef with initialization must have at least 3 children");
                }
                TreeNode *initNode = def->children[2];
                if (!initNode) {
                    throw std::runtime_error("InitVal node is null");
                }
                SymbolType literalType = (kind == ValueKind::FLOAT) ? SYM_FLOAT_CONST : SYM_INT_CONST;
                auto *litNode = findFirstLiteral(initNode, literalType);
                if (!litNode) {
                    // 尝试查找 NON_NUMBER 节点，然后访问其子节点
                    // 因为语法树中 Number 节点可能包含 SYM_INT_CONST 作为子节点
                    TreeNode *numberNode = findFirstLiteral(initNode, NON_NUMBER);
                    if (numberNode && !numberNode->children.empty()) {
                        TreeNode *firstChild = numberNode->children[0];
                        if (firstChild && firstChild->symbolId == literalType) {
                            litNode = firstChild;
                        }
                    }
                }
                if (!litNode) {
                    throw std::runtime_error("Global initializer must be literal: " + name);
                }
                initConst = buildConstant(kind, litNode->text);
            }
            auto gv = GlobalVariable::create(name, module_.get(),
                                             getIRType(module_.get(), kind), false, initConst);
            VarInfo info{gv, kind, false, true};
            globalVars_[name] = info;
        } else {
            auto allocaInst = builder_->create_alloca(getIRType(module_.get(), kind));
            VarInfo info{allocaInst, kind, false, false};
            defineLocal(name, info);
            if (hasInit) {
                ExprValue initVal = evalExp(def->children[2]->children[0]); // InitVal->Exp
                if (initVal.type == ValueKind::BOOL && kind == ValueKind::INT) {
                    initVal = castBoolToInt(initVal);
                }
                if (initVal.type != kind) {
                    throw std::runtime_error("Initializer type mismatch for " + name);
                }
                builder_->create_store(initVal.value, allocaInst);
            }
        }
    }
}

ValueKind IRGenerator::parseBType(TreeNode *node) {
    if (!node || node->children.empty()) {
        return ValueKind::INT;
    }
    auto *token = node->children[0];
    if (token->symbolId == SYM_KW_INT) {
        return ValueKind::INT;
    }
    if (token->symbolId == SYM_KW_FLOAT) {
        return ValueKind::FLOAT;
    }
    return ValueKind::INT;
}

std::string IRGenerator::parseIdent(TreeNode *node) {
    if (!node) {
        return "";
    }
    if (node->symbolId == SYM_ID) {
        return node->text;
    }
    for (auto *child : node->children) {
        if (child->symbolId == SYM_ID) {
            return child->text;
        }
    }
    return "";
}

void IRGenerator::pushScope() { scopeStack_.push_back({}); }

void IRGenerator::popScope() {
    if (!scopeStack_.empty()) {
        scopeStack_.pop_back();
    }
}

IRGenerator::VarInfo *IRGenerator::defineLocal(const std::string &name,
                                               const VarInfo &info) {
    if (scopeStack_.empty()) {
        pushScope();
    }
    scopeStack_.back()[name] = info;
    return &scopeStack_.back()[name];
}

IRGenerator::VarInfo *IRGenerator::lookupVar(const std::string &name) {
    for (auto it = scopeStack_.rbegin(); it != scopeStack_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &found->second;
        }
    }
    auto g = globalVars_.find(name);
    if (g != globalVars_.end()) {
        return &g->second;
    }
    return nullptr;
}

IRGenerator::FunctionInfo *IRGenerator::lookupFunction(const std::string &name) {
    auto it = functions_.find(name);
    if (it != functions_.end()) {
        return &it->second;
    }
    return nullptr;
}

// ---- Function definition handling -------------------------------------------------

static std::vector<std::pair<std::string, IRGenerator::ValueKind>>
collectFuncParams(TreeNode *node) {
    std::vector<std::pair<std::string, IRGenerator::ValueKind>> result;
    if (!node || node->children.empty()) {
        return result;
    }
    if (node->symbolId == NON_FUNCFPARAM) {
        IRGenerator::ValueKind kind = IRGenerator::ValueKind::INT;
        if (!node->children.empty() && node->children[0]->symbolId == NON_BTYPE) {
            if (!node->children[0]->children.empty() &&
                node->children[0]->children[0]->symbolId == SYM_KW_FLOAT) {
                kind = IRGenerator::ValueKind::FLOAT;
            }
        }
        std::string name;
        for (auto *child : node->children) {
            if (child->symbolId == SYM_ID) {
                name = child->text;
                break;
            }
        }
        if (!name.empty()) {
            result.emplace_back(name, kind);
        }
        return result;
    }
    if (node->symbolId == NON_FUNCFPARAMS) {
        if (node->children.size() == 1) {
            auto tmp = collectFuncParams(node->children[0]);
            result.insert(result.end(), tmp.begin(), tmp.end());
        } else if (node->children.size() == 3) {
            auto left = collectFuncParams(node->children[0]);
            auto right = collectFuncParams(node->children[2]);
            result.insert(result.end(), left.begin(), left.end());
            result.insert(result.end(), right.begin(), right.end());
        }
    } else if (node->symbolId == NON_OPTFUNCFPARAMS) {
        if (!node->children.empty()) {
            auto tmp = collectFuncParams(node->children[0]);
            result.insert(result.end(), tmp.begin(), tmp.end());
        }
    }
    return result;
}

void IRGenerator::registerFunctions(TreeNode *node) {
    if (!node) {
        return;
    }
    if (node->symbolId == NON_FUNCDEF) {
        registerFunctionDef(node);
        return;
    }
    for (auto *child : node->children) {
        registerFunctions(child);
    }
}

void IRGenerator::registerFunctionDef(TreeNode *node) {
    if (!node || node->children.size() < 6) {
        return;
    }
    size_t cursor = 0;
    ValueKind retKind = ValueKind::VOID;
    if (node->children[cursor]->symbolId == NON_FUNCTYPE) {
        retKind = ValueKind::VOID;
    } else {
        retKind = parseBType(node->children[cursor]);
    }
    cursor++;
    auto *nameNode = node->children[cursor++];
    std::string funcName = parseIdent(nameNode);
    if (funcName.empty() && !nameNode->children.empty() &&
        nameNode->children[0]->symbolId == SYM_KW_MAIN) {
        funcName = "main";
    }
    cursor++; // '('
    auto *paramsNode = node->children[cursor++];
    auto params = collectFuncParams(paramsNode);
    std::vector<Type *> paramTypes;
    std::vector<ValueKind> paramKinds;
    paramTypes.reserve(params.size());
    paramKinds.reserve(params.size());
    for (auto &p : params) {
        paramTypes.push_back(getIRType(module_.get(), p.second));
        paramKinds.push_back(p.second);
    }
    auto retType = getIRType(module_.get(), retKind);
    auto funcType = FunctionType::get(retType, paramTypes);

    if (functions_.count(funcName)) {
        throw std::runtime_error("Redefinition of function " + funcName);
    }

    FunctionInfo info;
    info.func = Function::create(funcType, funcName, module_.get());
    info.returnType = retKind;
    info.paramTypes = std::move(paramKinds);
    functions_.emplace(funcName, std::move(info));
}

static void collectFuncRParamNodes(TreeNode *node, std::vector<TreeNode *> &out) {
    if (!node) {
        return;
    }
    if (node->symbolId == NON_OPTFUNCRPARAMS) {
        if (!node->children.empty()) {
            collectFuncRParamNodes(node->children[0], out);
        }
        return;
    }
    if (node->symbolId == NON_FUNCRPARAMS) {
        if (node->children.size() == 1) {
            collectFuncRParamNodes(node->children[0], out);
        } else if (node->children.size() == 3) {
            collectFuncRParamNodes(node->children[0], out);
            collectFuncRParamNodes(node->children[2], out);
        }
        return;
    }
    if (node->symbolId == NON_EXP) {
        out.push_back(node);
    }
}

void IRGenerator::visitFuncDef(TreeNode *node) {
    if (!node || node->children.size() < 6) {
        return;
    }
    size_t cursor = 0;
    ValueKind retKind = ValueKind::VOID;
    if (node->children[cursor]->symbolId == NON_FUNCTYPE) {
        retKind = ValueKind::VOID;
    } else {
        retKind = parseBType(node->children[cursor]);
    }
    cursor++;
    auto *nameNode = node->children[cursor++];
    std::string funcName = parseIdent(nameNode);
    if (funcName.empty() && !nameNode->children.empty() &&
        nameNode->children[0]->symbolId == SYM_KW_MAIN) {
        funcName = "main";
    }
    cursor++; // '('
    auto *paramsNode = node->children[cursor++];
    cursor++; // ')'
    // FuncDef结构: FuncType/BType FuncName ( OptFParams ) Block
    // 索引: 0=FuncType/BType, 1=FuncName, 2='(', 3=OptFParams, 4=')', 5=Block
    const size_t BLOCK_INDEX = 5;
    if (cursor != BLOCK_INDEX) {
        cursor = BLOCK_INDEX;
    }
    if (cursor >= node->children.size()) {
        throw std::runtime_error("visitFuncDef: Block node index out of range");
    }
    auto *blockNode = node->children[cursor];
    if (!blockNode) {
        throw std::runtime_error("visitFuncDef: Block node is null");
    }
    if (blockNode->symbolId != NON_BLOCK) {
        throw std::runtime_error("visitFuncDef: Expected Block node but got different node type");
    }

    auto params = collectFuncParams(paramsNode);
    auto *funcInfo = lookupFunction(funcName);
    if (!funcInfo) {
        throw std::runtime_error("Function " + funcName + " not registered");
    }
    if (funcInfo->returnType != retKind) {
        throw std::runtime_error("Function return type mismatch for " + funcName);
    }
    if (funcInfo->paramTypes.size() != params.size()) {
        throw std::runtime_error("Function parameter count mismatch for " + funcName);
    }
    for (size_t i = 0; i < params.size(); ++i) {
        if (params[i].second != funcInfo->paramTypes[i]) {
            throw std::runtime_error("Function parameter type mismatch for " + funcName);
        }
    }
    currentFunction_ = funcInfo->func;
    currentReturnType_ = funcInfo->returnType;

    // Entry block 命名为 函数名_ENTRY
    std::string entryName = currentFunction_->get_name() + "_ENTRY";
    auto *entry = BasicBlock::create(module_.get(), entryName, currentFunction_);
    builder_->set_insert_point(entry);

    pushScope();
    auto argIt = currentFunction_->get_args().begin();
    for (size_t i = 0; i < params.size(); ++i, ++argIt) {
        const std::string &paramName = params[i].first;
        ValueKind kind = params[i].second;
        auto *allocaInst = builder_->create_alloca(getIRType(module_.get(), kind));
        builder_->create_store(*argIt, allocaInst);
        VarInfo info{allocaInst, kind, false, false};
        defineLocal(paramName, info);
    }

    try {
        visitBlock(blockNode, true);
    } catch (const std::exception &ex) {
        throw;
    } catch (...) {
        throw;
    }
    finalizeCurrentFunction();
    popScope();
    currentFunction_ = nullptr;
    currentReturnType_ = ValueKind::VOID;
}

// ---- Block / statement handling ---------------------------------------------------

static bool blockTerminated(BasicBlock *bb) {
    return bb && bb->get_terminator();
}

void IRGenerator::visitBlock(TreeNode *node, bool isFunctionBody) {
    if (!node) {
        return;
    }
    
    if (node->symbolId != NON_BLOCK) {
        throw std::runtime_error("visitBlock: Expected Block node but got different node type");
    }
    
    if (node->children.size() < 3) {
        return;
    }
    
    if (!isFunctionBody) {
        pushScope();
    }
    
    try {
        auto *listNode = node->children[1];
        
        if (!listNode) {
            if (!isFunctionBody) {
                popScope();
            }
            return;
        }
        
        // 递归遍历 BlockItemList 语法树结构
        // BlockItemList -> BlockItemList BlockItem | BlockItem | ε
        std::function<void(TreeNode *)> walk = [&](TreeNode *cur) {
            if (!cur) {
                return;
            }
            
            // 检查当前基本块是否已终止，如果已终止则不再处理后续节点
            auto *currentBB = builder_->get_insert_block();
            if (currentBB && blockTerminated(currentBB)) {
                return;
            }
            
            // 如果当前节点是 BlockItem，直接访问
            if (cur->symbolId == NON_BLOCKITEM) {
                visitBlockItem(cur);
                // 再次检查，因为visitBlockItem可能终止了基本块
                currentBB = builder_->get_insert_block();
                if (currentBB && blockTerminated(currentBB)) {
                    return;
                }
                return; // BlockItem处理完后直接返回，不再递归子节点
            }
            // 如果当前节点是 BlockItemList，需要递归处理
            if (cur->symbolId == NON_BLOCKITEMLIST) {
                // BlockItemList -> BlockItemList BlockItem (递归情况)
                if (cur->children.size() == 2) {
                    walk(cur->children[0]); // 先处理左边的 BlockItemList
                    // 检查基本块是否已终止
                    currentBB = builder_->get_insert_block();
                    if (currentBB && blockTerminated(currentBB)) {
                        return;
                    }
                    if (cur->children[1]) {
                        walk(cur->children[1]); // 再处理右边的 BlockItem
                    }
                    return;
                }
                // BlockItemList -> BlockItem (单个 BlockItem 情况)
                if (cur->children.size() == 1) {
                    walk(cur->children[0]);
                    return;
                }
                // BlockItemList -> ε (空列表情况，直接返回)
                return;
            }
            // 对于其他节点类型，递归遍历所有子节点（兜底逻辑）
            for (auto *child : cur->children) {
                // 每次递归前检查基本块是否已终止
                currentBB = builder_->get_insert_block();
                if (currentBB && blockTerminated(currentBB)) {
                    return;
                }
                walk(child);
            }
        };
        
        try {
            walk(listNode);
        } catch (const std::exception &ex) {
            throw;
        } catch (...) {
            throw;
        }
    
    } catch (const std::exception &ex) {
        if (!isFunctionBody) {
            popScope();
        }
        throw;
    } catch (...) {
        if (!isFunctionBody) {
            popScope();
        }
        throw;
    }
    
    if (!isFunctionBody) {
        popScope();
    }
}

void IRGenerator::visitBlockItem(TreeNode *node) {
    if (!node || node->children.empty()) {
        return;
    }
    // BlockItem -> Decl | Stmt
    auto *child = node->children[0];
    if (!child) {
        return;
    }
    try {
        if (child->symbolId == NON_DECL) {
            visitDecl(child, false);
        } else if (child->symbolId == NON_STMT) {
            visitStmt(child);
        } else {
            // 如果遇到其他类型，可能是语法树结构问题
            std::cerr << "    Warning: visitBlockItem encountered unexpected node type: " 
                      << child->symbolId << std::endl;
        }
    } catch (const std::exception &ex) {
        throw;
    } catch (...) {
        throw;
    }
}

void IRGenerator::visitStmt(TreeNode *node) {
    if (!node || node->children.empty()) {
        return;
    }
    auto &c = node->children;
    if (c[0]->symbolId == NON_LVAL && c.size() >= 4) {
        auto rhs = evalExp(c[2]);
        handleAssignment(c[0], rhs);
        return;
    }
    if (c[0]->symbolId == NON_BLOCK) {
        visitBlock(c[0], false);
        return;
    }
    if (c[0]->symbolId == NON_OPTEXP && c.size() >= 2) {
        if (!c[0]->children.empty()) {
            evalExp(c[0]->children[0]);
        }
        return;
    }
    if (c[0]->symbolId == SYM_KW_RETURN) {
        ExprValue retVal;
        retVal.type = ValueKind::VOID;
        if (c[1]->symbolId == NON_OPTEXP && !c[1]->children.empty()) {
            retVal = evalExp(c[1]->children[0]);
        }
        if (currentReturnType_ == ValueKind::VOID) {
            builder_->create_void_ret();
        } else {
            // 如果函数返回类型是INT，但return语句没有提供值，默认返回0
            if (retVal.type == ValueKind::VOID && currentReturnType_ == ValueKind::INT) {
                retVal.value = ConstantInt::get(0, module_.get());
                retVal.type = ValueKind::INT;
            } else if (currentReturnType_ == ValueKind::INT &&
                       retVal.type == ValueKind::BOOL) {
                retVal = castBoolToInt(retVal);
            }
            if (retVal.type != currentReturnType_) {
                throw std::runtime_error("Return type mismatch: expected " + 
                                         std::to_string((int)currentReturnType_) + 
                                         ", got " + std::to_string((int)retVal.type));
            }
            builder_->create_ret(retVal.value);
        }
        return;
    }
    if (c[0]->symbolId == SYM_KW_IF) {
        bool hasElse = (c.size() == 7);
        auto *condNode = c[2];
        auto condVal = buildCondValue(condNode);
        
        // 生成唯一的标签名
        int labelId = ifLabelCounter_++;
        std::string thenLabel = "if_then" + std::to_string(labelId);
        std::string elseLabel = hasElse ? ("if_else" + std::to_string(labelId)) : "";
        std::string mergeLabel = "if_merge" + std::to_string(labelId);
        
        auto *thenBB = BasicBlock::create(module_.get(), thenLabel, currentFunction_);
        BasicBlock *elseBB =
            hasElse ? BasicBlock::create(module_.get(), elseLabel, currentFunction_)
                    : nullptr;
        BasicBlock *mergeBB = nullptr;
        if (hasElse) {
            builder_->create_cond_br(condVal, thenBB, elseBB);
            builder_->set_insert_point(thenBB);
            visitStmt(c[4]);
            bool thenTerminated = blockTerminated(builder_->get_insert_block());
            if (!thenTerminated) {
                if (!mergeBB) {
                    mergeBB = BasicBlock::create(module_.get(), mergeLabel, currentFunction_);
                }
                builder_->create_br(mergeBB);
            }
            builder_->set_insert_point(elseBB);
            visitStmt(c[6]);
            bool elseTerminated = blockTerminated(builder_->get_insert_block());
            if (!elseTerminated) {
                if (!mergeBB) {
                    mergeBB = BasicBlock::create(module_.get(), mergeLabel, currentFunction_);
                }
                builder_->create_br(mergeBB);
            }
            // 只有当至少一个分支没有终止时，才使用 mergeBB
            if (mergeBB && (!thenTerminated || !elseTerminated)) {
                builder_->set_insert_point(mergeBB);
            }
        } else {
            // 对于没有else的if，需要创建mergeBB作为false分支的目标
            mergeBB = BasicBlock::create(module_.get(), mergeLabel, currentFunction_);
            builder_->create_cond_br(condVal, thenBB, mergeBB);
            builder_->set_insert_point(thenBB);
            visitStmt(c[4]);
            bool thenTerminated = blockTerminated(builder_->get_insert_block());
            if (!thenTerminated) {
                builder_->create_br(mergeBB);
            }
            // mergeBB总是需要，因为它是false分支的目标
            // 如果then分支终止了，mergeBB包含后续代码
            // 如果then分支没有终止，mergeBB会被then分支跳转到
            builder_->set_insert_point(mergeBB);
        }
        return;
    }
}

// ---- Expression helpers -----------------------------------------------------------

IRGenerator::ExprValue IRGenerator::ensureBool(const ExprValue &val) {
    if (val.type == ValueKind::BOOL) {
        return val;
    }
    if (val.type == ValueKind::INT) {
        auto zero = ConstantInt::get(0, module_.get());
        auto cmp = builder_->create_icmp_ne(val.value, zero);
        return {cmp, ValueKind::BOOL};
    }
    if (val.type == ValueKind::FLOAT) {
        auto zero = ConstantFloat::get(0.0f, module_.get());
        auto cmp = builder_->create_fcmp_ne(val.value, zero);
        return {cmp, ValueKind::BOOL};
    }
    throw std::runtime_error("Cannot convert expression to bool");
}

IRGenerator::ExprValue IRGenerator::castBoolToInt(const ExprValue &val) {
    if (val.type != ValueKind::BOOL) {
        return val;
    }
    auto zext = builder_->create_zext(val.value, module_->get_int32_type());
    return {zext, ValueKind::INT};
}

IRGenerator::ExprValue IRGenerator::ensureInt(const ExprValue &val) {
    if (val.type == ValueKind::INT) {
        return val;
    }
    if (val.type == ValueKind::BOOL) {
        return castBoolToInt(val);
    }
    throw std::runtime_error("Expect integer expression");
}

IRGenerator::ExprValue IRGenerator::evalExp(TreeNode *node) {
    if (!node || node->children.empty()) {
        return {};
    }
    return evalLOr(node->children[0]);
}

IRGenerator::ExprValue IRGenerator::evalLOr(TreeNode *node) {
    if (!node) {
        return {};
    }
    if (node->children.size() == 1) {
        return evalLAnd(node->children[0]);
    }
    auto lhs = evalLOr(node->children[0]);
    auto rhs = evalLAnd(node->children[2]);
    auto lhsBool = ensureBool(lhs);
    auto rhsBool = ensureBool(rhs);
    auto lhsInt = castBoolToInt(lhsBool);
    auto rhsInt = castBoolToInt(rhsBool);
    auto sum = builder_->create_iadd(lhsInt.value, rhsInt.value);
    auto zero = ConstantInt::get(0, module_.get());
    auto cmp = builder_->create_icmp_ne(sum, zero);
    return {cmp, ValueKind::BOOL};
}

IRGenerator::ExprValue IRGenerator::evalLAnd(TreeNode *node) {
    if (!node) return {};
    if (node->children.size() == 1) {
        return evalEq(node->children[0]);
    }
    auto lhs = evalLAnd(node->children[0]);
    auto rhs = evalEq(node->children[2]);
    auto lhsInt = castBoolToInt(ensureBool(lhs));
    auto rhsInt = castBoolToInt(ensureBool(rhs));
    auto prod = builder_->create_imul(lhsInt.value, rhsInt.value);
    auto zero = ConstantInt::get(0, module_.get());
    auto cmp = builder_->create_icmp_ne(prod, zero);
    return {cmp, ValueKind::BOOL};
}

IRGenerator::ExprValue IRGenerator::evalEq(TreeNode *node) {
    if (!node) return {};
    if (node->children.size() == 1) {
        return evalRel(node->children[0]);
    }
    auto lhs = evalEq(node->children[0]);
    auto rhs = evalRel(node->children[2]);
    if (lhs.type == ValueKind::BOOL) lhs = castBoolToInt(lhs);
    if (rhs.type == ValueKind::BOOL) rhs = castBoolToInt(rhs);
    if (lhs.type != rhs.type) {
        throw std::runtime_error("Type mismatch in equality expression");
    }
    Value *cmp = nullptr;
    auto op = node->children[1]->symbolId;
    if (lhs.type == ValueKind::INT) {
        cmp = (op == SYM_OP_EQ) ? builder_->create_icmp_eq(lhs.value, rhs.value)
                                : builder_->create_icmp_ne(lhs.value, rhs.value);
    } else if (lhs.type == ValueKind::FLOAT) {
        cmp = (op == SYM_OP_EQ) ? builder_->create_fcmp_eq(lhs.value, rhs.value)
                                : builder_->create_fcmp_ne(lhs.value, rhs.value);
    } else {
        throw std::runtime_error("Unsupported equality operand type");
    }
    return {cmp, ValueKind::BOOL};
}

IRGenerator::ExprValue IRGenerator::evalRel(TreeNode *node) {
    if (!node) return {};
    if (node->children.size() == 1) {
        return evalAdd(node->children[0]);
    }
    auto lhs = evalRel(node->children[0]);
    auto rhs = evalAdd(node->children[2]);
    if (lhs.type == ValueKind::BOOL) lhs = castBoolToInt(lhs);
    if (rhs.type == ValueKind::BOOL) rhs = castBoolToInt(rhs);
    if (lhs.type != rhs.type) {
        throw std::runtime_error("Type mismatch in relational expression");
    }
    Value *cmp = nullptr;
    auto op = node->children[1]->symbolId;
    if (lhs.type == ValueKind::INT) {
        switch (op) {
        case SYM_OP_LT:
            cmp = builder_->create_icmp_lt(lhs.value, rhs.value);
            break;
        case SYM_OP_GT:
            cmp = builder_->create_icmp_gt(lhs.value, rhs.value);
            break;
        case SYM_OP_LE:
            cmp = builder_->create_icmp_le(lhs.value, rhs.value);
            break;
        case SYM_OP_GE:
            cmp = builder_->create_icmp_ge(lhs.value, rhs.value);
            break;
        default:
            break;
        }
    } else if (lhs.type == ValueKind::FLOAT) {
        switch (op) {
        case SYM_OP_LT:
            cmp = builder_->create_fcmp_lt(lhs.value, rhs.value);
            break;
        case SYM_OP_GT:
            cmp = builder_->create_fcmp_gt(lhs.value, rhs.value);
            break;
        case SYM_OP_LE:
            cmp = builder_->create_fcmp_le(lhs.value, rhs.value);
            break;
        case SYM_OP_GE:
            cmp = builder_->create_fcmp_ge(lhs.value, rhs.value);
            break;
        default:
            break;
        }
    }
    if (!cmp) {
        throw std::runtime_error("Unsupported relational operator");
    }
    return {cmp, ValueKind::BOOL};
}

IRGenerator::ExprValue IRGenerator::evalAdd(TreeNode *node) {
    if (!node) return {};
    if (node->children.size() == 1) {
        return evalMul(node->children[0]);
    }
    auto lhs = evalAdd(node->children[0]);
    auto rhs = evalMul(node->children[2]);
    if (lhs.type == ValueKind::BOOL) lhs = castBoolToInt(lhs);
    if (rhs.type == ValueKind::BOOL) rhs = castBoolToInt(rhs);
    if (lhs.type != rhs.type) {
        throw std::runtime_error("Type mismatch in add expression");
    }
    Value *res = nullptr;
    auto op = node->children[1]->symbolId;
    if (lhs.type == ValueKind::INT) {
        res = (op == SYM_OP_PLUS) ? builder_->create_iadd(lhs.value, rhs.value)
                                  : builder_->create_isub(lhs.value, rhs.value);
    } else if (lhs.type == ValueKind::FLOAT) {
        res = (op == SYM_OP_PLUS) ? builder_->create_fadd(lhs.value, rhs.value)
                                  : builder_->create_fsub(lhs.value, rhs.value);
    }
    if (!res) {
        throw std::runtime_error("Unsupported add operator");
    }
    return {res, lhs.type};
}

IRGenerator::ExprValue IRGenerator::evalMul(TreeNode *node) {
    if (!node) return {};
    if (node->children.size() == 1) {
        return evalUnary(node->children[0]);
    }
    auto lhs = evalMul(node->children[0]);
    auto rhs = evalUnary(node->children[2]);
    if (lhs.type == ValueKind::BOOL) lhs = castBoolToInt(lhs);
    if (rhs.type == ValueKind::BOOL) rhs = castBoolToInt(rhs);
    if (lhs.type != rhs.type) {
        throw std::runtime_error("Type mismatch in mul expression");
    }
    Value *res = nullptr;
    auto op = node->children[1]->symbolId;
    if (lhs.type == ValueKind::INT) {
        if (op == SYM_OP_MUL) res = builder_->create_imul(lhs.value, rhs.value);
        else if (op == SYM_OP_DIV) res = builder_->create_isdiv(lhs.value, rhs.value);
        else if (op == SYM_OP_MOD) res = builder_->create_irem(lhs.value, rhs.value);
    } else if (lhs.type == ValueKind::FLOAT) {
        if (op == SYM_OP_MUL) res = builder_->create_fmul(lhs.value, rhs.value);
        else if (op == SYM_OP_DIV) res = builder_->create_fdiv(lhs.value, rhs.value);
    }
    if (!res) {
        throw std::runtime_error("Unsupported mul operator");
    }
    return {res, lhs.type};
}

IRGenerator::ExprValue IRGenerator::evalUnary(TreeNode *node) {
    if (!node) return {};
    if (node->children.size() == 4 && node->children[0]->symbolId == SYM_ID &&
        node->children[1]->symbolId == SYM_SE_LPAREN) {
        std::string funcName = parseIdent(node->children[0]);
        auto *funcInfo = lookupFunction(funcName);
        if (!funcInfo) {
            throw std::runtime_error("Call to undefined function " + funcName);
        }
        std::vector<TreeNode *> argNodes;
        collectFuncRParamNodes(node->children[2], argNodes);
        if (argNodes.size() != funcInfo->paramTypes.size()) {
            throw std::runtime_error("Argument count mismatch in call to " + funcName);
        }
        std::vector<Value *> args;
        args.reserve(argNodes.size());
        for (size_t i = 0; i < argNodes.size(); ++i) {
            auto argVal = evalExp(argNodes[i]);
            if (funcInfo->paramTypes[i] == ValueKind::INT &&
                argVal.type == ValueKind::BOOL) {
                argVal = castBoolToInt(argVal);
            }
            if (argVal.type != funcInfo->paramTypes[i]) {
                throw std::runtime_error("Argument type mismatch in call to " + funcName);
            }
            args.push_back(argVal.value);
        }
        auto *call = builder_->create_call(funcInfo->func, args);
        if (funcInfo->returnType == ValueKind::VOID) {
            return {nullptr, ValueKind::VOID};
        }
        return {call, funcInfo->returnType};
    }
    if (node->children.size() == 1) {
        return evalPrimary(node->children[0]);
    }
    if (node->children.size() == 2) {
        auto op = node->children[0]->children[0]->symbolId;
        auto val = evalUnary(node->children[1]);
        if (val.type == ValueKind::BOOL) {
            val = castBoolToInt(val);
        }
        if (val.type == ValueKind::INT) {
            if (op == SYM_OP_PLUS) return val;
            auto zero = ConstantInt::get(0, module_.get());
            auto neg = builder_->create_isub(zero, val.value);
            return {neg, ValueKind::INT};
        } else if (val.type == ValueKind::FLOAT) {
            if (op == SYM_OP_PLUS) return val;
            auto zero = ConstantFloat::get(0.0f, module_.get());
            auto neg = builder_->create_fsub(zero, val.value);
            return {neg, ValueKind::FLOAT};
        }
    }
    return evalPrimary(node->children.back());
}

IRGenerator::ExprValue IRGenerator::evalPrimary(TreeNode *node) {
    if (!node || node->children.empty()) {
        return {};
    }
    auto first = node->children[0];
    if (first->symbolId == SYM_SE_LPAREN) {
        return evalExp(node->children[1]);
    }
    if (first->symbolId == NON_LVAL) {
        return evalLVal(first);
    }
    if (first->symbolId == NON_NUMBER) {
        auto *lit = first->children[0];
        if (lit->symbolId == SYM_INT_CONST) {
            auto c = ConstantInt::get(std::stoi(lit->text), module_.get());
            return {c, ValueKind::INT};
        }
        if (lit->symbolId == SYM_FLOAT_CONST) {
            auto c = ConstantFloat::get(std::stof(lit->text), module_.get());
            return {c, ValueKind::FLOAT};
        }
    }
    return {};
}

IRGenerator::ExprValue IRGenerator::evalLVal(TreeNode *node) {
    std::string name = parseIdent(node);
    auto *info = lookupVar(name);
    if (!info) {
        throw std::runtime_error("Undefined identifier " + name);
    }
    
    Value *loaded = nullptr;
    if (info->isGlobal) {
        // 全局变量：确保地址是指针类型
        if (!info->address->get_type()->is_pointer_type()) {
            throw std::runtime_error("Global variable address is not pointer type: " + name);
        }
        // 获取指针指向的元素类型
        Type *elementType = info->address->get_type()->get_pointer_element_type();
        loaded = builder_->create_load(elementType, info->address);
    } else {
        // 局部变量：直接加载
        loaded = builder_->create_load(info->address);
    }
    return {loaded, info->type};
}

Value *IRGenerator::buildCondValue(TreeNode *node) {
    if (!node) {
        throw std::runtime_error("buildCondValue: node is null");
    }
    // NON_COND -> NON_LOREXP, so we need to evaluate the LOrExp
    TreeNode *expNode = node;
    if (node->symbolId == NON_COND) {
        if (node->children.empty()) {
            throw std::runtime_error("buildCondValue: NON_COND node has no children");
        }
        expNode = node->children[0]; // NON_LOREXP
    }
    // If the node is already LOrExp, use it directly
    if (expNode->children.empty()) {
        throw std::runtime_error("buildCondValue: LOrExp node has no children");
    }
    // Create a dummy NON_EXP node structure for evalExp
    // evalExp expects NON_EXP -> NON_LOREXP, so we can directly use the LOrExp
    auto val = evalLOr(expNode);
    return ensureBool(val).value;
}

void IRGenerator::handleAssignment(TreeNode *lvalNode, ExprValue rhs) {
    std::string name = parseIdent(lvalNode);
    auto *info = lookupVar(name);
    if (!info) {
        throw std::runtime_error("Undefined identifier " + name);
    }
    if (info->isConst) {
        throw std::runtime_error("Cannot assign to const " + name);
    }
    if (info->type == ValueKind::INT && rhs.type == ValueKind::BOOL) {
        rhs = castBoolToInt(rhs);
    }
    if (info->type != rhs.type) {
        throw std::runtime_error("Assignment type mismatch for " + name + ": expected " + 
                                 std::to_string((int)info->type) + ", got " + 
                                 std::to_string((int)rhs.type));
    }
    
    // 全局变量和局部变量的存储逻辑相同，但需要确保地址类型正确
    if (info->isGlobal) {
        // 全局变量：确保地址是指针类型
        if (!info->address->get_type()->is_pointer_type()) {
            throw std::runtime_error("Global variable address is not pointer type: " + name);
        }
    }
    builder_->create_store(rhs.value, info->address);
}

void IRGenerator::finalizeCurrentFunction() {
    if (!currentFunction_) {
        return;
    }
    auto *bb = builder_->get_insert_block();
    if (!bb || blockTerminated(bb)) {
        return;
    }
    if (currentReturnType_ == ValueKind::VOID) {
        builder_->create_void_ret();
    } else if (currentReturnType_ == ValueKind::INT) {
        builder_->create_ret(ConstantInt::get(0, module_.get()));
    } else if (currentReturnType_ == ValueKind::FLOAT) {
        builder_->create_ret(ConstantFloat::get(0.0f, module_.get()));
    }
}

