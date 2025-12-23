#pragma once

#include <map>
#include <vector>
#include <string>

class GeometryGraph;

// 迭代状态
struct IterationState {
    double confidence = 0.0;
    std::map<std::string, int> elementCounts;
    std::vector<std::string> missingElements;
    std::vector<std::string> conflicts;
    bool isConverged = false;
    std::vector<double> confidenceHistory;
    GeometryGraph* geometryGraph;  // 使用指针避免不完整类型错误

    // 默认构造函数
    IterationState();

    // 析构函数
    ~IterationState();

    // 拷贝构造函数 - 实现在 IterationState.cpp 中
    IterationState(const IterationState& other);

    // 赋值操作符 - 实现在 IterationState.cpp 中
    IterationState& operator=(const IterationState& other);
};