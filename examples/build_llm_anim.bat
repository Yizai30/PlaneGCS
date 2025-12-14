@echo off
chcp 65001
echo ==========================================
echo 智能几何图动画系统
echo 基于 PlaneGCS 和自然语言理解
echo ==========================================

REM 设置构建环境
set PATH=C:\env\cpplib\mingw64\bin;%PATH%

REM 设置编译器路径
set COMPILER=g++
set CXX_STANDARD=-std=c++17
set EIGEN3_INCLUDE=C:\env\cpplib\eigen3\include\eigen3
set BOOST_INCLUDE=C:\env\cpplib\boost_1_89_0

REM 编译几何动画系统
echo.
echo 编译器: g++ %CXX_STANDARD%
echo Eigen3路径: %EIGEN3_INCLUDE%
echo Boost路径: %BOOST_INCLUDE%
echo.
echo 包含源文件: ../src/GCS.cpp ../src/Constraints.cpp ../src/Geo.cpp ../src/SubSystem.cpp ../src/qp_eq.cpp ../examples/llm_geometry_animation.cpp
echo 链接库: -L"C:\env\cpplib\boost_1_89_0\lib"
echo 输出文件: -o build_llm\llm_geometry_animation.exe
echo.

g++ %CXX_STANDARD% ^
    -I"%EIGEN3_INCLUDE%" ^
    -I"%BOOST_INCLUDE%" ^
    ../src/GCS.cpp ../src/Constraints.cpp ../src/Geo.cpp ../src/SubSystem.cpp ../src/qp_eq.cpp ../examples/llm_geometry_animation.cpp ^
    -L"C:\env\cpplib\boost_1_89_0\lib" ^
    -o build_llm\llm_geometry_animation.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ==========================================
    echo ✅ 编译成功！
    echo.
    echo 可执行文件: build_llm\llm_geometry_animation.exe
    echo ==========================================
    echo 运行演示...
    build_llm\llm_geometry_animation.exe
) else (
    echo.
    echo ❌ 编译失败！
    echo.
    echo 错误码: %ERRORLEVEL%
    echo.
    echo ==========================================
    echo.
    echo 排查建议：
    echo 1. 检查 g++ 编译器是否安装
    echo 2. 确认 Eigen3 和 Boost 路径正确
    echo 3. 验证源文件路径
    echo 4. 检查文件编码
    echo.
    pause
)