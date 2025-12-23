#include "IterationState.h"
#include "../examples/solution_to_keyframes_core.h"

IterationState::IterationState()
{
    geometryGraph = new GeometryGraph();
}

// 析构函数的实现
IterationState::~IterationState() {
    if (geometryGraph) {
        delete geometryGraph;
        geometryGraph = nullptr;
    }
}

// 拷贝构造函数的实现
IterationState::IterationState(const IterationState& other) : geometryGraph(nullptr) {
    confidence = other.confidence;
    elementCounts = other.elementCounts;
    missingElements = other.missingElements;
    conflicts = other.conflicts;
    isConverged = other.isConverged;
    confidenceHistory = other.confidenceHistory;

    if (other.geometryGraph) {
        // 需要包含 GeometryGraph 的完整定义
        // 这里我们无法直接创建 GeometryGraph，因为避免循环依赖
        // 在实际使用中，这个功能应该由调用者处理
        geometryGraph = nullptr;  // 暂时设为 nullptr
    }
}

// 赋值操作符的实现
IterationState& IterationState::operator=(const IterationState& other) {
    if (this != &other) {
        confidence = other.confidence;
        elementCounts = other.elementCounts;
        missingElements = other.missingElements;
        conflicts = other.conflicts;
        isConverged = other.isConverged;
        confidenceHistory = other.confidenceHistory;

        if (geometryGraph) {
            delete geometryGraph;
            geometryGraph = nullptr;
        }

        if (other.geometryGraph) {
            // 同样避免循环依赖
            geometryGraph = nullptr;  // 暂时设为 nullptr
        }
    }
    return *this;
}