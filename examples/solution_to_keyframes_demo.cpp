#include "solution_to_keyframes_core.h"
#include <iostream>

// 辅助函数：去除字符串两端的空白字符
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// 辅助函数：查找标题的位置，支持多种格式
size_t findTitlePosition(const std::string& content, const std::string& title) {
    // 尝试多种格式
    std::vector<std::string> patterns = {
        "##" + title + ":",
        "## " + title + ":",
        "##" + title,
        "## " + title,
        "##" + title + " :",
        "## " + title + " :"
    };

    for (const auto& pattern : patterns) {
        size_t pos = content.find(pattern);
        if (pos != std::string::npos) {
            return pos;
        }
    }

    // 如果没有找到精确匹配，尝试不区分大小写的搜索
    std::string content_lower = content;
    std::transform(content_lower.begin(), content_lower.end(), content_lower.begin(), ::tolower);

    std::string title_lower = "##" + title;
    std::transform(title_lower.begin(), title_lower.end(), title_lower.begin(), ::tolower);

    size_t pos = content_lower.find(title_lower);
    if (pos != std::string::npos) {
        // 在原始字符串中找到对应的位置
        return pos;
    }

    title_lower = "## " + title;
    std::transform(title_lower.begin(), title_lower.end(), title_lower.begin(), ::tolower);

    pos = content_lower.find(title_lower);
    if (pos != std::string::npos) {
        return pos;
    }

    return std::string::npos;
}

// 辅助函数：提取标题内容，直到下一个标题或字符串结束
std::string extractSection(const std::string& content, size_t start_pos,
    const std::vector<size_t>& other_title_positions) {
    if (start_pos == std::string::npos) {
        return "";
    }

    // 找到下一个标题的位置（如果有）
    size_t next_title_pos = std::string::npos;
    for (size_t pos : other_title_positions) {
        if (pos > start_pos && (next_title_pos == std::string::npos || pos < next_title_pos)) {
            next_title_pos = pos;
        }
    }

    if (next_title_pos != std::string::npos) {
        return content.substr(start_pos, next_title_pos - start_pos);
    }
    else {
        return content.substr(start_pos);
    }
}

int main() {
    std::cout << "=== 解决方案到动画关键帧核心流程演示 ===" << endl;

    // 创建核心处理器
    SolutionToKeyframesProcessor processor;

    // 模拟解决方案内容（通常来自文件或用户输入）
    //std::vector<std::string> solution_contents = {
    //    "Move the point clockwise by 30 degrees",
    //    "Continue rotating to 90 degree position",
    //    "Scale up the circle to 1.2x",
    //    "Rotate to 180 degree position",
    //    "Scale down to original size",
    //    "Return to starting position"
    //};
    std::string solution_content = R"(## Theorem:
- Parabola: $x^2 = 4y$ is a special form of a conic section
- Focus: One of the properties of a parabola; for $x^2 = 4y$, the focus is $F(0,1)$
- Distance Formula: Belongs to analytic geometry, used to calculate the distance between two points
- Minimum value problem: Solvable using differentiation methods

##Step:
0. Given that $M$ is a point on the parabola $x^{2}=4y$ and $F$ is its focus, find the minimum value of $|MF| + |MA| + |MB|$ for the points $A(2,6)$ and $B(-3,1)$.
1. Determine that the focus of the parabola $x^2 = 4y$ is $F(0,1)$
2. Let point $M$ lie on the parabola with coordinates $(x, \frac{x^2}{4})$
3. Objective function: $f(x) = |MF| + |MA| + |MB|$
   - Using the parabola's properties: $|MF| = \frac{x^2}{4} + 1$ (Distance from point to focus equals distance from point to directrix)
   - $|MA| = \sqrt{(x-2)^2 + (\frac{x^2}{4} - 6)^2}$
   - $|MB| = \sqrt{(x+3)^2 + (\frac{x^2}{4} - 1)^2}$
4. Differentiate the objective function to obtain the derivative equation:
   $f'(x) = \frac{x}{2} + \frac{\frac{x(\frac{x^2}{4} - 1)}{2} + x + 3}{\sqrt{(x+3)^2 + (\frac{x^2 {4} - 1)^2}} + \frac{\frac{x(\frac{x^2}{4} - 6)}{2} + x - 2}{\sqrt{(x-2)^2 + (\frac{x^2}{4} - 6)^2}}$
5. Solve $f'(x) = 0$ numerically to obtain the critical point $x \approx -2.09384$
6. Compute the objective function values at the critical point:
   - $|MF| = 2.096$
   - $|MA| = 6.388$  
   - $|MB| = 0.911$
   - Total: $2.096 + 6.388 + 0.911 = 9.395$

##Answer:
The minimum value of $|MF| + |MA| + |MB|$ is approximately $9.395$.)";
    std::vector<std::string> solution_contents {};

    // 将solution_content按照标题分割成三个部分，添加到solution_contents
    // 定义要查找的标题
    std::vector<std::string> titles = { "Theorem", "Step", "Answer" };
    std::vector<size_t> title_positions;

    // 查找所有标题的位置
    for (const auto& title : titles) {
        size_t pos = findTitlePosition(solution_content, title);
        if (pos == std::string::npos) {
            std::cerr << "Warning: Could not find title '" << title << "' in the solution content.\n";
        }
        title_positions.push_back(pos);
    }

    // 检查是否所有标题都找到了
    bool all_found = std::all_of(title_positions.begin(), title_positions.end(),
        [](size_t pos) { return pos != std::string::npos; });

    if (!all_found) {
        std::cerr << "Error: Not all required sections were found in the solution content.\n";
        std::cerr << "Found positions: ";
        for (size_t i = 0; i < titles.size(); ++i) {
            std::cerr << titles[i] << ":" << (title_positions[i] == std::string::npos ? "not found" : "found") << " ";
        }
        std::cerr << "\n";
        return 1;
    }

    // 提取每个部分的内容
    for (size_t i = 0; i < titles.size(); ++i) {
        // 创建其他标题位置的副本（排除当前标题）
        std::vector<size_t> other_positions;
        for (size_t j = 0; j < title_positions.size(); ++j) {
            if (j != i) {
                other_positions.push_back(title_positions[j]);
            }
        }

        std::string section = extractSection(solution_content, title_positions[i], other_positions);

        // 清理提取的内容：去除开头的多余空白行
        size_t first_char = section.find_first_not_of("\n\r");
        if (first_char != std::string::npos) {
            // 找到标题后的第一个非空白字符
            size_t title_end = section.find_first_of("\n\r", 0);
            if (title_end != std::string::npos) {
                // 保留标题行，去除标题后的多余空白行
                size_t content_start = section.find_first_not_of("\n\r", title_end);
                if (content_start != std::string::npos) {
                    // 重新构建：标题行 + 内容
                    std::string title_line = section.substr(0, title_end);
                    std::string content = section.substr(content_start);
                    section = trim(title_line) + "\n" + content;
                }
            }
        }

        solution_contents.push_back(section);
    }

    std::cout << "解决方案内容数量: " << solution_contents.size() << std::endl;
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