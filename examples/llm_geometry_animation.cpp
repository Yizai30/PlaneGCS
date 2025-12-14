/***************************************************************************
 * 智能几何图动画系统 - 基于自然语言理解和PlaneGCS
 * 功能：根据输入几何图和自然语言描述，计算下一个关键帧状态
 *
 * 系统架构：
 * 1. 几何图定义：节点=几何元素，边=几何关系
 * 2. 自然语言处理：LLM理解用户指令并决定下个状态
 * 3. 约束验证：使用PlaneGCS验证几何图正确性
 * 4. 自动修正：如果不正确则自动修正几何图
 * 5. 循环演示：3次循环的智能动画演示
 *
 * 编译：g++ -std=c++17 -I../src llm_geometry_animation.cpp -o llm_geo_anim
 ***************************************************************************/
#include "llm_geometry_animation.h"

// 设置控制台UTF-8编码
void setConsoleUTF8() {
    // 暂时禁用控制台编码设置以避免Windows头文件冲突
    // 如果需要解决控制台中文显示问题，可以：
    // 1. 在运行程序前手动设置控制台为UTF-8: chcp 65001
    // 2. 或者在Visual Studio中将文件保存为UTF-8 BOM格式
}

// ========================
// 主函数
// ========================

int main() {
    // 设置控制台编码为UTF-8
    setConsoleUTF8();

    cout << "=== 智能几何图动画系统 ===" << endl;
    cout << "基于自然语言理解和PlaneGCS的几何动画" << endl;
    cout << "版本: 1.0" << endl;

    try {
        IntelligentGeometryAnimation animation_system;

        cout << "\n请选择演示模式：" << endl;
        cout << "1. 三循环动画演示 (传统模式)" << endl;
        cout << "2. 解决方案到关键帧演示 (新功能)" << endl;
        cout << "0. 退出" << endl;
        cout << "选择 (0-2): ";

        int choice;
        cin >> choice;

        switch (choice) {
            case 1:
                animation_system.runThreeCycleDemo();
                break;
            case 2:
                animation_system.runSolutionToKeyframesDemo();
                break;
            case 0:
                cout << "退出程序。" << endl;
                break;
            default:
                cout << "无效选择，运行默认演示。" << endl;
                animation_system.runSolutionToKeyframesDemo();
                break;
        }

    } catch (const exception& e) {
        cerr << "程序运行出错: " << e.what() << endl;
        return -1;
    }

    cout << "\n程序正常结束。" << endl;
    return 0;
}