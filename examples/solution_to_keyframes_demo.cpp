#include "solution_to_keyframes_core.h"
#include <iostream>

int main() {
    cout << "=== 解决方案到动画关键帧核心流程演示 ===" << endl;

    // 创建核心处理器
    SolutionToKeyframesProcessor processor;

    // 模拟解决方案内容（通常来自文件或用户输入）
    std::vector<std::string> solution_contents = {
        "Move the point clockwise by 30 degrees",
        "Continue rotating to 90 degree position",
        "Scale up the circle to 1.2x",
        "Rotate to 180 degree position",
        "Scale down to original size",
        "Return to starting position"
    };

    cout << "解决方案内容数量: " << solution_contents.size() << std::endl;
    for (size_t i = 0; i < solution_contents.size(); ++i) {
        std::cout << (i + 1) << ". " << solution_contents[i] << std::endl;
    }

    // 执行核心流程：从解决方案生成动画关键帧
    std::vector<std::string> keyframes = processor.generateAnimationKeyframes(solution_contents);

    // 显示结果
    std::cout << "\n=== 处理结果 ===" << std::endl;
    std::cout << "生成的关键帧数量: " << keyframes.size() << std::endl;

    for (size_t i = 0; i < keyframes.size(); ++i) {
        std::cout << "关键帧 " << (i + 1) << ": ";
        for (size_t j = 0; j < keyframes[i].size(); ++j) {
            std::cout << keyframes[i][j];
            if (j < keyframes[i].size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
    }

    std::cout << "\n输出文件:" << std::endl;
    std::cout << "- animation_commands.txt: 动画指令详情" << std::endl;
    std::cout << "- generated_animation_keyframes.txt: 关键帧数据" << std::endl;

    std::cout << "\n=== 演示完成 ===" << std::endl;
    return 0;
}