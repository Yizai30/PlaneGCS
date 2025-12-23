# 解决方案到动画关键帧核心流程(AI生成的文档)

## 概述

这个模块实现了从几何问题解决方案描述到动画关键帧生成的核心流程，基于时序图中定义的处理步骤。

## 文件结构

```
solution_to_keyframes_core.h    - 核心流程定义和类声明
solution_to_keyframes_core.cpp  - 核心流程实现
solution_to_keyframes_demo.cpp - 演示程序
solution_to_keyframes_README.md - 本说明文件
```

## 核心流程步骤

根据时序图，核心流程包含以下步骤：

### 1. 初始化阶段
- 实例化 `SolutionToKeyframesProcessor`
- 调用 `setupSampleGeometry()` 设置初始几何图

### 2. 解决方案处理循环
对每个解决方案内容执行：
- **步骤2**: `updateGeometryGraph()` - 使用LLM更新几何图
- **步骤3**: `validateGeometry()` - 验证几何图有效性
- **步骤4**: `autoCorrectGeometry()` - 如需要则自动修正
- **步骤5**: `parseAnimCommand()` - 解析动画指令

### 3. 生成阶段
- **步骤6**: `generateKeyframes()` - 从动画指令生成关键帧
- 保存动画指令和关键帧到文件

## 使用方法

### 基本用法

```cpp
#include "solution_to_keyframes_core.h"

int main() {
    // 创建处理器
    SolutionToKeyframesProcessor processor;

    // 准备解决方案内容
    std::vector<std::string> solution_contents = {
        "Move the point clockwise by 30 degrees",
        "Continue rotating to 90 degree position",
        "Scale up the circle to 1.2x"
        // ... 更多步骤
    };

    // 执行核心流程
    std::vector<std::vector<double>> keyframes =
        processor.processSolutionToKeyframes(solution_contents);

    // 使用生成的关键帧
    for (const auto& keyframe : keyframes) {
        // 处理每个关键帧
    }

    return 0;
}
```

### 编译和运行

```bash
# 编译演示程序
g++ -o solution_demo solution_to_keyframes_demo.cpp solution_to_keyframes_core.cpp -I./include

# 运行演示
./solution_demo
```

## 输出文件

### animation_commands.txt
包含解析得到的动画指令：
```
# 智能几何图动画指令
# 生成时间: Mon Dec 16 13:37:25 2025

# 动画指令总数: 3

指令 1:
  类型: MOVE
  参数:
    ANGLE = 30
    DIRECTION = CLOCKWISE

指令 2:
  类型: MOVE
  参数:
    ANGLE = 60
    DIRECTION = CLOCKWISE

# 动画指令结束
```

### animation_keyframes.txt
包含生成的关键帧数据：
```
# 智能几何图动画关键帧数据
# 生成时间: Mon Dec 16 13:37:25 2025

# 关键帧总数: 3

关键帧 1: 0, 50, 0
关键帧 2: 1, 25, 43.3
关键帧 3: 2, 0, 50

# 关键帧数据结束
```

## 扩展性

### 自定义LLM处理器
可以通过继承 `LLMGeometryProcessor` 类来实现自定义的LLM集成：

```cpp
class CustomLLMProcessor : public SolutionToKeyframesProcessor::LLMGeometryProcessor {
public:
    GeometryGraph updateGeometryGraph(const GeometryGraph& current_graph,
                                     const string& new_content) override {
        // 实现自定义LLM调用逻辑
    }
};
```

### 自定义几何约束验证
可以通过重写验证方法实现自定义的几何约束：

```cpp
class CustomProcessor : public SolutionToKeyframesProcessor {
protected:
    bool validateGeometry(const GeometryGraph& graph) override {
        // 实现自定义验证逻辑
        return true;
    }
};
```

## 与原系统的关系

这个核心流程是从 `llm_geometry_animation.h` 中提取的独立模块，专注于：

1. **清晰的接口** - 提供简单的 `processSolutionToKeyframes()` 方法
2. **模块化设计** - 各个步骤可以独立测试和替换
3. **可扩展性** - 便于添加新的几何类型和动画指令
4. **文件输出** - 自动保存中间结果和最终结果

## 注意事项

1. 当前实现包含简化的LLM集成，实际使用时需要连接真实的LLM API
2. 几何约束验证和自动修正逻辑可以根据具体需求扩展
3. 关键帧生成算法可以根据动画需求优化
4. 输出格式可以根据动画引擎要求调整