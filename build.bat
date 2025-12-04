@echo off
chcp 65001 >nul
echo ========================================
echo 编译编译器工具链
echo ========================================
echo.

echo [1/2] 编译词法分析器...
cd lex
g++ -std=c++17 -Wall Lex_Analysis.cpp -o Lex_Analysis.exe
if %errorlevel% neq 0 (
    echo 词法分析器编译失败！
    cd ..
    exit /b 1
)
cd ..
echo 词法分析器编译成功！
echo.

echo [2/2] 编译语法分析器...
cd Syntax
g++ -std=c++17 -Wall main.cpp GrammarAnalyzer.cpp SLRTable.cpp Parser.cpp ReductionSequenceLogger.cpp -o Syntaxer.exe
if %errorlevel% neq 0 (
    echo 语法分析器编译失败！
    cd ..
    exit /b 1
)
cd ..
echo 语法分析器编译成功！
echo.

echo ========================================
echo 编译完成！
echo ========================================
pause