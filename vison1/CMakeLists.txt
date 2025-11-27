cmake_minimum_required(VERSION 3.10)
project(SysyCompiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含中间代码生成库的头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/complie_ir/include)

# 添加中间代码生成库的源文件
set(IR_SOURCES
    complie_ir/src/Module.cpp
    complie_ir/src/Type.cpp
    complie_ir/src/Value.cpp
    complie_ir/src/User.cpp
    complie_ir/src/Constant.cpp
    complie_ir/src/GlobalVariable.cpp
    complie_ir/src/Function.cpp
    complie_ir/src/BasicBlock.cpp
    complie_ir/src/Instruction.cpp
    complie_ir/src/IRprinter.cpp
)

# 添加编译器前端的源文件
set(COMPILER_SOURCES
    SimpleParser.cpp
    CodeGenerator.cpp
    main.cpp
)

# 创建可执行文件
add_executable(compiler ${COMPILER_SOURCES} ${IR_SOURCES})

# 设置输出目录
set_target_properties(compiler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
