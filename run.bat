@echo off
chcp 65001 >nul
echo ========================================
echo 运行编译器工具链
echo ========================================

REM 检查可执行文件是否存在
if not exist "lex\Lex_Analysis.exe" (
    echo 错误：词法分析器未编译，请先运行 build.bat
    pause
    exit /b 1
)

if not exist "Syntax\Syntaxer.exe" (
    echo 错误：语法分析器未编译，请先运行 build.bat
    pause
    exit /b 1
)

echo [1/2] 运行词法分析器...
cd lex
Lex_Analysis.exe
if %errorlevel% neq 0 (
    echo 词法分析器运行失败！
    cd ..
    pause
    exit /b 1
)
cd ..
echo 词法分析完成！
echo.

echo [2/2] 运行语法分析器...
cd Syntax
Syntaxer.exe
cd ..

REM 等待一下确保文件写入完成
timeout /t 1 >nul

REM 检查语法分析器是否成功生成输出文件
dir /b "Syntax\output\*out.txt" >nul 2>&1
if %errorlevel% equ 0 (
    echo 语法分析完成！
) else (
    echo 语法分析器运行失败！（未生成输出文件）
    pause
    exit /b 1
)
echo.

echo ========================================
echo 编译链执行完成！
echo ========================================
echo 词法分析结果: lex\output\lex*.txt
echo 语法分析日志: Syntax\output\*out.txt
echo 规约序列: Syntax\output\syntax*.txt
echo 语法树: Syntax\output\tree*.txt
pause