/*!
 *@file Module.h
 *@brief 模块接口定义文件
 *@version 1.0.0
 *@date 2022-10-04
 */
#include "Module.h"
#include "Function.h"
#include "Type.h"

#include <utility>
#include <set>

Module::Module(std::string name) : module_name_(std::move(name)) {
  /// @brief 创建类型指针对象
  /// @param name
  void_ty_ = new Type(Type::VoidTyID, this);
  label_ty_ = new Type(Type::LabelTyID, this);
  int1_ty_ = new IntegerType(1, this);
  int32_ty_ = new IntegerType(32, this);
  float32_ty_ = new FloatType(this);

  /// @brief id 与 字符串的映射添加
  instr_id2string_.insert({Instruction::ret, "ret"});
  instr_id2string_.insert({Instruction::br, "br"});

  instr_id2string_.insert({Instruction::add, "add"});
  instr_id2string_.insert({Instruction::sub, "sub"});
  instr_id2string_.insert({Instruction::mul, "mul"});
  instr_id2string_.insert({Instruction::sdiv, "sdiv"});
  instr_id2string_.insert({Instruction::mod, "srem"});
  instr_id2string_.insert({Instruction::fadd, "fadd"});
  instr_id2string_.insert({Instruction::fsub, "fsub"});
  instr_id2string_.insert({Instruction::fmul, "fmul"});
  instr_id2string_.insert({Instruction::fdiv, "fdiv"});

  instr_id2string_.insert({Instruction::cmp, "icmp"});
  instr_id2string_.insert({Instruction::fcmp, "fcmp"});

  instr_id2string_.insert({Instruction::alloca, "alloca"});
  instr_id2string_.insert({Instruction::load, "load"});
  instr_id2string_.insert({Instruction::store, "store"});
  instr_id2string_.insert({Instruction::phi, "phi"});
  instr_id2string_.insert({Instruction::call, "call"});
  instr_id2string_.insert({Instruction::getelementptr, "getelementptr"});
  instr_id2string_.insert({Instruction::zext, "zext"});
}

/**
 * @brief Destroy the Module:: Module object 析构函数
 *
 */
Module::~Module() {
  delete void_ty_;
  delete label_ty_;
  delete int1_ty_;
  delete int32_ty_;
  delete float32_ty_;
}
/**
 * @brief Get the void type object，获取一个构建好的void类型指针
 *
 * @return Type*
 */
Type *Module::get_void_type() { return void_ty_; }
/**
 * @brief Get the label type object，获取一个构建好的label类型指针
 *
 * @return Type*
 */
Type *Module::get_label_type() { return label_ty_; }
/**
 * @brief Get the int1 type object，获取一个构建好的integer1类型指针
 *
 * @return IntegerType*
 */
IntegerType *Module::get_int1_type() { return int1_ty_; }
/**
 * @brief Get the int32 type object，获取一个构建好的integer32类型指针
 *
 * @return IntegerType*
 */
IntegerType *Module::get_int32_type() { return int32_ty_; }
/**
 * @brief Get the pointer type object，获取一个构建好的指针类型指针
 *
 * @param contained 指针指向数据的类型
 * @return PointerType*
 */
PointerType *Module::get_pointer_type(Type *contained) {
  if (pointer_map_.find(contained) == pointer_map_.end()) {
    pointer_map_[contained] = new PointerType(contained);
  }
  return pointer_map_[contained];
}
/**
 * @brief Get the array type object，获取一个构建好的array类型指针
 *
 * @param contained 数组元素类型
 * @param num_elements 数组元素个数
 * @return ArrayType*
 */
ArrayType *Module::get_array_type(Type *contained, unsigned num_elements) {
  if (array_map_.find({contained, num_elements}) == array_map_.end()) {
    array_map_[{contained, num_elements}] =
        new ArrayType(contained, num_elements);
  }
  return array_map_[{contained, num_elements}];
}
/**
 * @brief Get the int32 ptr type object，获取一个构建好的integer32指针类型指针
 *
 * @return PointerType*
 */
PointerType *Module::get_int32_ptr_type() {
  return get_pointer_type(int32_ty_);
}
/**
 * @brief Get the float type object，获取一个构建好的float类型指针
 *
 * @return FloatType*
 */
FloatType *Module::get_float_type() { return float32_ty_; }
/**
 * @brief Get the float ptr type object，获取一个构建好的float指针类型指针
 *
 * @return PointerType*
 */
PointerType *Module::get_float_ptr_type() {
  return get_pointer_type(float32_ty_);
}
/**
 * @brief 添加函数
 *
 * @param f 函数指针
 */
void Module::add_function(Function *f) { function_list_.push_back(f); }
/**
 * @brief Get the functions object，获取函数列表
 *
 * @return std::list<Function *> 函数列表
 */
std::list<Function *> Module::get_functions() { return function_list_; }
/**
 * @brief 添加全局量
 *
 * @param g 全局量指针
 */
void Module::add_global_variable(GlobalVariable *g) {
  global_list_.push_back(g);
}
/**
 * @brief Get the global variable object，获取全局量指针数组
 *
 * @return std::list<GlobalVariable *> 全局量指针数组
 */
std::list<GlobalVariable *> Module::get_global_variable() {
  return global_list_;
}
/**
 * @brief Set the print name object，修正模块管理的函数下的名称
 *
 */
void Module::set_print_name() {
  for (auto func : this->get_functions()) {
    func->set_instr_name();
  }
  return;
}
/**
 * @brief 打印中间代码
 *
 * @return std::string
 */
std::string Module::print() {
  std::string module_ir;
  
  // 添加头部信息
  module_ir += "; ModuleID = 'sysy2022_complier'\n";
  if (!source_file_name_.empty()) {
    module_ir += "source_filename = \"" + source_file_name_ + "\"\n";
  } else {
    module_ir += "source_filename = \"<stdin>\"\n";
  }
  module_ir += "\n";
  
  // 添加全局变量
  for (auto global_val : this->global_list_) {
    module_ir += global_val->print();
    module_ir += "\n";
  }
  
  // 添加标准函数声明
  auto int32_ty = get_int32_type();
  auto void_ty = get_void_type();
  auto int32_ptr_ty = get_int32_ptr_type();
  
  // declare i32 @getinit()
  auto getinit_ty = FunctionType::get(int32_ty, std::vector<Type *>());
  auto getinit_func = Function::create(getinit_ty, "getinit", this);
  module_ir += getinit_func->print();
  
  // declare i32 @getch()
  auto getch_ty = FunctionType::get(int32_ty, std::vector<Type *>());
  auto getch_func = Function::create(getch_ty, "getch", this);
  module_ir += getch_func->print();
  
  // declare i32 @getarray(i32*)
  auto getarray_ty = FunctionType::get(int32_ty, std::vector<Type *>{int32_ptr_ty});
  auto getarray_func = Function::create(getarray_ty, "getarray", this);
  module_ir += getarray_func->print();
  
  // declare void @putinit(i32)
  auto putinit_ty = FunctionType::get(void_ty, std::vector<Type *>{int32_ty});
  auto putinit_func = Function::create(putinit_ty, "putinit", this);
  module_ir += putinit_func->print();
  
  // declare void @putch(i32)
  auto putch_ty = FunctionType::get(void_ty, std::vector<Type *>{int32_ty});
  auto putch_func = Function::create(putch_ty, "putch", this);
  module_ir += putch_func->print();
  
  // declare void @putarray(i32, i32*)
  auto putarray_ty = FunctionType::get(void_ty, std::vector<Type *>{int32_ty, int32_ptr_ty});
  auto putarray_func = Function::create(putarray_ty, "putarray", this);
  module_ir += putarray_func->print();
  
  // declare void @starttime()
  auto starttime_ty = FunctionType::get(void_ty, std::vector<Type *>());
  auto starttime_func = Function::create(starttime_ty, "starttime", this);
  module_ir += starttime_func->print();
  
  // declare void @stoptime()
  auto stoptime_ty = FunctionType::get(void_ty, std::vector<Type *>());
  auto stoptime_func = Function::create(stoptime_ty, "stoptime", this);
  module_ir += stoptime_func->print();
  module_ir += "\n";
  
  // 添加用户定义的函数（跳过标准函数声明）
  std::set<std::string> std_funcs = {"getinit", "getch", "getarray", "putinit", "putch", "putarray", "starttime", "stoptime"};
  for (auto func : this->function_list_) {
    // 跳过标准函数，因为它们已经在上面声明了
    if (std_funcs.find(func->get_name()) == std_funcs.end()) {
      module_ir += func->print();
      module_ir += "\n";
    }
  }
  return module_ir;
}
