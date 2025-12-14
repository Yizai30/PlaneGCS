# PlaneGCS 构建说明

## 使用 vcpkg 管理依赖

### 前置条件

1. **安装 CMake** (3.15+)
   ```bash
   # 下载并安装 CMake
   # 或使用包管理器：winget install Kitware.CMake
   ```

2. **安装 vcpkg**
   ```bash
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat  # Windows
   # 或 ./bootstrap-vcpkg.sh  # Linux/macOS
   ```

### 构建步骤

1. **安装依赖包**
   ```bash
   # 在项目根目录下
   vcpkg install --triplet=x64-windows
   ```

2. **配置和构建项目**
   ```bash
   # 创建构建目录
   mkdir build
   cd build

   # 配置 CMake（指定 vcpkg 工具链）
   cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake

   # 构建项目
   cmake --build . --config Release
   ```

   或者一次性执行：
   ```bash
   cmake --build . --config Release --parallel
   ```

### 运行示例

构建完成后，可执行文件位于：
- `build/bin/Release/llm_geometry_animation.exe` - LLM 几何动画演示
- `build/bin/Release/math_animation_demo.exe` - 基础数学动画演示

### 故障排除

1. **找不到 vcpkg 包**：
   ```bash
   vcpkg update
   vcpkg install --triplet=x64-windows
   ```

2. **CMake 版本过低**：确保使用 CMake 3.15+

3. **编译错误**：
   - 确保所有源文件都已提交
   - 检查 vcpkg 工具链路径是否正确
   - 清理构建目录重新构建

### IDE 集成

#### Visual Studio
```bash
# 在 VS 开发者命令提示符中
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
```

#### VS Code
安装扩展：
- C/C++
- CMake Tools

在 `.vscode/settings.json` 中配置：
```json
{
    "cmake.configureArgs": [
        "-DCMAKE_TOOLCHAIN_FILE=[vcpkg-root-path]/scripts/buildsystems/vcpkg.cmake"
    ]
}
```

### 依赖包列表

项目依赖以下包：
- **eigen3** (3.4.0+): 线性代数库
- **boost-locale** (1.82.0+): 本地化支持
- **curl** (8.0.0+): HTTP 客户端（用于 LLM API）
- **cpprestsdk** (2.10.19+): C++ REST SDK（用于 LLM API）

### 自定义构建选项

```bash
# Debug 构建
cmake --build . --config Debug

# 多线程编译
cmake --build . --config Release --parallel 4

# 只构建特定目标
cmake --build . --target llm_geometry_animation --config Release
```