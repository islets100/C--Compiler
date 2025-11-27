#!/bin/bash

# 编译编译器
echo "Building compiler..."
mkdir -p build
cd build
cmake ..
cmake --build . --config Release
cd ..

if [ ! -f build/bin/compiler ]; then
    echo "Compilation failed!"
    exit 1
fi

echo "Compiler built successfully!"

# 测试词法分析
echo ""
echo "=== Testing Lexical Analysis ==="
./build/bin/compiler test_simple.sy

# 检查输出文件
if [ -f output_lex.txt ]; then
    echo "Lexical analysis output:"
    cat output_lex.txt
fi

if [ -f output.ll ]; then
    echo ""
    echo "Code generation output:"
    cat output.ll
fi
