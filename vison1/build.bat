@echo off
REM 创建build目录
if not exist build mkdir build

REM 进入build目录
cd build

REM 运行cmake
cmake ..

REM 编译
cmake --build . --config Release

REM 返回到原目录
cd ..

echo Build completed!
