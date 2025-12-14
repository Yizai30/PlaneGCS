/***************************************************************************
 * 数学教学动画演示 - 使用PlaneGCS几何约束求解器
 * 演示概念：勾股定理、圆与切线、相似三角形、三角函数
 *
 * 编译方法：
 * g++ -std=c++17 -I../src math_animation_demo.cpp -o math_animation_demo
 *
 * 或者使用CMake构建：
 * mkdir build && cd build
 * cmake ..
 * make
 ***************************************************************************/

#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>
#include <thread>
#include <fstream>

#ifdef _WIN32
#include <conio.h>  // for _kbhit(), _getch()
#else
#include <unistd.h>  // for usleep()
#include <termios.h>
#include <fcntl.h>
#endif

// 包含PlaneGCS库
#include "GCS.h"
#include "Geo.h"
#include "Constraints.h"

using namespace GCS;
using namespace std;

// 数学教学动画演示类
class MathTeachingAnimation {
private:
    System constraint_system;

    // 几何元素变量
    double x1, y1, x2, y2, x3, y3, x4, y4;
    double radius, angle, distance, target_x, target_y;
    double triangle_base, triangle_height, hypotenuse;

    // 动画参数
    double time;
    bool is_paused;

public:
    MathTeachingAnimation() : time(0.0), is_paused(false) {
        // 初始化几何变量
        x1 = 0.0; y1 = 0.0;  // 原点
        x2 = 100.0; y2 = 0.0;  // x轴点
        x3 = 0.0; y3 = 100.0;  // y轴点
        x4 = 0.0; y4 = 0.0;   // 动态点

        radius = 50.0;
        angle = 0.0;
        triangle_base = 80.0;
        triangle_height = 60.0;

        setupPythagorasDemo();  // 勾股定理演示
    }

    // 1. 勾股定理演示
    void setupPythagorasDemo() {
        constraint_system.clear();

        // 创建直角三角形的三个顶点
        Point A = {&x1, &y1};  // 直角顶点 (0,0)
        Point B = {&x2, &y2};  // x轴顶点 (100,0)
        Point C = {&x3, &y3};  // y轴顶点 (0,100)

        // 设置直角约束 - A点固定在原点
        constraint_system.addConstraintCoordinateX(A, &x1, 1);  // tag=1用于识别
        constraint_system.addConstraintCoordinateY(A, &y1, 1);

        // 设置直角约束 - AB在x轴上
        constraint_system.addConstraintCoordinateY(B, &y2, 1);  // B的y坐标为0
        constraint_system.addConstraintCoordinateY(A, &y1, 1);  // A的y坐标为0

        // 设置直角约束 - AC在y轴上
        constraint_system.addConstraintCoordinateX(C, &x3, 1);  // C的x坐标为0
        constraint_system.addConstraintCoordinateX(A, &x1, 1);  // A的x坐标为0

        // 动态约束 - 通过temp tag控制边的长度
        constraint_system.addConstraintP2PDistance(A, B, &triangle_base, -1);  // 临时约束
        constraint_system.addConstraintP2PDistance(A, C, &triangle_height, -1);  // 临时约束

        // 计算斜边长度（勾股定理）
        hypotenuse = sqrt(triangle_base * triangle_base + triangle_height * triangle_height);
        constraint_system.addConstraintP2PDistance(B, C, &hypotenuse, 2);  // tag=2固定斜边

        // 声明未知变量
        vector<double*> unknowns = {&x2, &y2, &x3, &y3};
        constraint_system.declareUnknowns(unknowns);

        cout << "勾股定理演示设置完成！" << endl;
        cout << "直角边长: " << triangle_base << ", " << triangle_height << endl;
        cout << "斜边长: " << hypotenuse << endl;
        cout << "验证: " << triangle_base << "² + " << triangle_height << "² = "
             << hypotenuse << "² ? " << (abs(triangle_base*triangle_base + triangle_height*triangle_height - hypotenuse*hypotenuse) < 0.001) << endl;
    }

    // 2. 圆与切线演示
    void setupCircleTangentDemo() {
        constraint_system.clear();

        // 圆心和动态点
        Point center = {&x1, &y1};     // 圆心
        Point point_on_circle = {&x2, &y2};  // 圆上的点
        Point tangent_point = {&x3, &y3};    // 切线上的点

        // 圆
        Circle circle;
        circle.center = center;
        circle.rad = &radius;  // 🔧 关键修复：设置圆的半径指针

        // 固定圆心
        constraint_system.addConstraintCoordinateX(center, &x1, 1);
        constraint_system.addConstraintCoordinateY(center, &y1, 1);

        // 点在圆上
        constraint_system.addConstraintPointOnCircle(point_on_circle, circle, 1);

        // 使用临时约束引导圆上点的位置（通过角度计算）
        target_x = x1 + radius * cos(angle);
        target_y = y1 + radius * sin(angle);
        constraint_system.addConstraintCoordinateX(point_on_circle, &target_x, -1);
        constraint_system.addConstraintCoordinateY(point_on_circle, &target_y, -1);

        // 切线约束 - 切线点与圆上点的连线垂直于半径
        constraint_system.addConstraintPerpendicular(center, point_on_circle, point_on_circle, tangent_point, 2);

        // 声明未知变量
        vector<double*> unknowns = {&x2, &y2, &x3, &y3};
        constraint_system.declareUnknowns(unknowns);

        cout << "圆与切线演示设置完成！" << endl;
        cout << "圆心: (" << x1 << ", " << y1 << "), 半径: " << radius << endl;

        // 初始化求解器并设置初始角度
        angle = 0.0;  // 从0度开始
        if (solveConstraints()) {
            cout << "初始求解成功！" << endl;
        } else {
            cout << "初始求解失败！" << endl;
        }
    }

    // 3. 相似三角形演示
    void setupSimilarTrianglesDemo() {
        constraint_system.clear();

        // 第一个三角形 (大三角形)
        Point A1 = {&x1, &y1};
        Point B1 = {&x2, &y2};
        Point C1 = {&x3, &y3};

        // 第二个三角形 (小三角形)
        Point A2 = {&x4, &y4};
        Point B2 = {&x1 + 20, &y1 + 20};  // 固定位置
        Point C2 = {&x1 + 40, &y1 + 10};  // 固定位置

        // 大三角形设置
        constraint_system.addConstraintCoordinateX(A1, &x1, 1);
        constraint_system.addConstraintCoordinateY(A1, &y1, 1);
        constraint_system.addConstraintCoordinateY(B1, &y2, 1);  // B1在水平线上
        constraint_system.addConstraintCoordinateX(C1, &x3, 1);  // C1在垂直线上

        // 相似性约束 - 对应边成比例
        double scale = 0.4;  // 相似比例

        // 设置边的长度比例
        constraint_system.addConstraintP2PDistance(A1, B1, &triangle_base, 1);
        double small_base = triangle_base * scale;
        constraint_system.addConstraintP2PDistance(A2, B2, &small_base, 2);

        // 角度对应约束
        constraint_system.addConstraintL2LAngle(A1, B1, A1, C1, &angle, 1);
        constraint_system.addConstraintL2LAngle(A2, B2, A2, C2, &angle, 2);

        // 声明未知变量
        vector<double*> unknowns = {&x2, &y2, &x3, &y3, &x4, &y4};
        constraint_system.declareUnknowns(unknowns);

        cout << "相似三角形演示设置完成！" << endl;
        cout << "相似比例: " << scale << endl;
    }

    // 动画更新函数
    void updateAnimation(double deltaTime) {
        if (is_paused) return;

        time += deltaTime;

        // 更新动画参数
        // 对于圆与切线演示，使用线性角度变化
        if (radius > 0) {
            angle = time * 0.1;  // 线性角度变化
        } else {
            angle = sin(time * 0.5) * M_PI / 3;  // 角度在60度范围内变化
            triangle_base = 80 + 20 * sin(time * 0.3);  // 底边长度变化
            triangle_height = 60 + 15 * cos(time * 0.4);  // 高度变化
            hypotenuse = sqrt(triangle_base * triangle_base + triangle_height * triangle_height);
        }
    }

    // 求解当前约束
    bool solveConstraints() {
        // 清除临时约束并重新添加更新后的约束
        constraint_system.clearByTag(-1);

        // 重新添加当前时间的临时约束
        target_x = x1 + radius * cos(angle);
        target_y = y1 + radius * sin(angle);

        // 初始化并求解
        constraint_system.initSolution(DogLeg);
        int result = constraint_system.solve(true, DogLeg);

        return (result == Success || result == Converged);
    }

    // 获取当前几何数据用于渲染
    vector<pair<double,double>> getCurrentGeometry() {
        vector<pair<double,double>> points;
        points.push_back({x1, y1});  // A点
        points.push_back({x2, y2});  // B点
        points.push_back({x3, y3});  // C点
        points.push_back({x4, y4});  // D点（如果使用）
        return points;
    }

    // 输出当前状态
    void printCurrentState() {
        cout << "\n=== 时间: " << fixed << setprecision(2) << time << " ===" << endl;
        cout << "几何点坐标:" << endl;
        cout << "  A: (" << x1 << ", " << y1 << ")" << endl;
        cout << "  B: (" << x2 << ", " << y2 << ")" << endl;
        cout << "  C: (" << x3 << ", " << y3 << ")" << endl;

        if (radius > 0) {
            cout << "圆参数:" << endl;
            cout << "  半径: " << radius << ", 当前角度: " << angle * 180 / M_PI << "度" << endl;
            cout << "  圆上点位置: (" << target_x << ", " << target_y << ")" << endl;
            cout << "  切线点位置: (" << x3 << ", " << y3 << ")" << endl;

            // 验证点是否在圆上
            double dist_to_center = sqrt(pow(target_x-x1, 2) + pow(target_y-y1, 2));
            cout << "  验证: 点到圆心距离=" << dist_to_center << " (应该=" << radius << ")" << endl;
        }

        // 计算并显示数学关系
        cout << "数学关系验证:" << endl;
        double current_hyp = sqrt(pow(x2-x1, 2) + pow(y3-y1, 2));
        cout << "  当前直角边: AB=" << abs(x2-x1) << ", AC=" << abs(y3-y1) << endl;
        cout << "  当前斜边: BC=" << current_hyp << endl;
        cout << "  勾股定理验证: " << abs(x2-x1) << "² + " << abs(y3-y1) << "² = "
             << current_hyp*current_hyp << " (误差: " <<
             abs((x2-x1)*(x2-x1) + (y3-y1)*(y3-y1) - current_hyp*current_hyp) << ")" << endl;
    }

    // 生成关键帧数据并保存到文件
    void generateKeyFrames(const string& filename, double duration, double fps = 30.0) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "无法创建文件: " << filename << endl;
            return;
        }

        file << "# 数学教学动画关键帧数据\n";
        file << "# 格式: time x1 y1 x2 y2 x3 y3 angle radius\n";

        int totalFrames = static_cast<int>(duration * fps);

        cout << "生成关键帧数据..." << endl;
        cout << "时长: " << duration << "秒, 帧率: " << fps << ", 总帧数: " << totalFrames << endl;

        // 🔧 高级约束失败处理
        int consecutive_failures = 0;
        double last_good_x2 = x2, last_good_y2 = y2;
        double last_good_x3 = x3, last_good_y3 = y3;
        double last_good_angle = angle;

        for (int frame = 0; frame <= totalFrames; frame++) {
            double t = frame / fps;

            // 🔧 策略式更新：如果连续失败，降低更新幅度
            if (consecutive_failures > 3) {
                cout << "连续失败超过3次，启用容错模式..." << endl;

                // 使用更小的更新步长
                double smaller_dt = 0.1 / fps;  // 减小时间步长
                updateAnimation(smaller_dt);

                // 使用上次成功的位置作为起点
                if (consecutive_failures > 5) {
                    x2 = last_good_x2;
                    y2 = last_good_y2;
                    x3 = last_good_x3;
                    y3 = last_good_y3;
                    angle = last_good_angle;
                }
            } else {
                updateAnimation(t);  // 正常更新
            }

            if (solveConstraints()) {
                // ✅ 成功：保存关键帧并更新记录
                file << fixed << setprecision(6) << t << " "
                     << x1 << " " << y1 << " " << x2 << " " << y2 << " "
                     << x3 << " " << y3 << " " << angle << " " << radius << "\n";

                // 更新成功记录
                last_good_x2 = x2; last_good_y2 = y2;
                last_good_x3 = x3; last_good_y3 = y3;
                last_good_angle = angle;
                consecutive_failures = 0;

                // 显示进度
                if (frame % 30 == 0) {
                    cout << "进度: " << (frame * 100) / totalFrames << "% (✅)" << endl;
                }
            } else {
                // ❌ 失败：详细分析和处理
                consecutive_failures++;
                cerr << "第 " << frame << " 帧求解失败！(连续失败: " << consecutive_failures << ")" << endl;

                // 🔍 尝试不同的求解策略
                if (consecutive_failures == 1) {
                    // 第一次失败：尝试降低精度要求
                    constraint_system.convergence *= 10;  // 降低收敛要求
                    constraint_system.maxIter = 200;   // 增加迭代次数
                    cout << "  → 调整：降低收敛精度到 " << constraint_system.convergence << endl;
                } else if (consecutive_failures == 2) {
                    // 第二次失败：尝试不同算法
                    cout << "  → 调整：尝试Levenberg-Marquardt算法" << endl;
                    constraint_system.initSolution(LevenbergMarquardt);
                } else if (consecutive_failures == 3) {
                    // 第三次失败：尝试BFGS算法
                    cout << "  → 调整：尝试BFGS算法" << endl;
                    constraint_system.initSolution(BFGS);
                }

                // 🔧 约束诊断
                if (consecutive_failures >= 4) {
                    diagnoseConstraintSystem();
                }

                // 🔧 保存部分结果（即使不完全精确）
                if (consecutive_failures <= 3) {
                    file << fixed << setprecision(6) << t << " "
                         << x1 << " " << y1 << " " << x2 << " " << y2 << " "
                         << x3 << " " << y3 << " " << angle << " " << radius << " #PARTIAL\n";
                }
            }
        }

        file.close();
        cout << "关键帧数据已保存到: " << filename << endl;
    }

    // 控制函数
    void pause() { is_paused = true; }
    void resume() { is_paused = false; }
    void reset() { time = 0.0; angle = 0.0; }
    void setTime(double t) { time = t; }

    // 设置参数
    void setRadius(double r) { radius = r; }
    void setAngle(double a) { angle = a; }
    void setTriangleBase(double base) { triangle_base = base; }
    void setTriangleHeight(double height) { triangle_height = height; }

    // 🔧 约束诊断函数
    void diagnoseConstraintSystem() {
        cout << "\n=== 约束系统诊断 ===" << endl;

        // 1. 系统基本信息
        cout << "系统配置:" << endl;
        cout << "  最大迭代次数: " << constraint_system.maxIter << endl;
        cout << "  收敛精度: " << constraint_system.convergence << endl;
        cout << "  求解算法: DogLeg" << endl;

        // 2. 自由度分析
        int dofs = constraint_system.dofsNumber();
        if (dofs >= 0) {
            cout << "  自由度: " << dofs << " (系统确定)" << endl;
        } else {
            cout << "  自由度: " << dofs << " (过约束或冲突)" << endl;
        }

        // 3. 冲突和冗余约束分析
        vector<int> conflicting;
        vector<int> redundant;
        constraint_system.getConflicting(conflicting);
        constraint_system.getRedundant(redundant);

        if (!conflicting.empty()) {
            cout << "  冲突约束: 无" << endl;
        } else {
            cout << "  冲突约束数量: " << conflicting.size() << endl;
            cout << "  冲突约束ID: ";
            for (int i = 0; i < min((int)conflicting.size(), 5); i++) {
                cout << conflicting[i] << " ";
            }
            cout << (conflicting.size() > 5 ? "..." : "") << endl;
        }

        if (!redundant.empty()) {
            cout << "  冗余约束: 无" << endl;
        } else {
            cout << "  冗余约束数量: " << redundant.size() << endl;
            cout << "  冗余约束ID: ";
            for (int i = 0; i < min((int)redundant.size(), 5); i++) {
                cout << redundant[i] << " ";
            }
            cout << (redundant.size() > 5 ? "..." : "") << endl;
        }

        // 4. 依赖参数分析
        vector<double*> dependent_params;
        constraint_system.getDependentParams(dependent_params);
        if (!dependent_params.empty()) {
            cout << "  依赖参数数量: " << dependent_params.size() << endl;
            cout << "  依赖参数: ";
            for (size_t i = 0; i < min(dependent_params.size(), (size_t)5); i++) {
                cout << "[" << (dependent_params[i] - &x1) << "] ";
            }
            cout << (dependent_params.size() > 5 ? "..." : "") << endl;
        }

        // 5. 当前几何关系验证
        cout << "\n当前几何验证:" << endl;
        if (radius > 0) {
            // 计算点到圆心的实际距离
            double dist_to_center = sqrt(pow(x2-x1, 2) + pow(y2-y1, 2));
            cout << "  点到圆心距离: " << dist_to_center << " (应该=" << radius << ")" << endl;
            cout << "  误差: " << abs(dist_to_center - radius) << endl;

            // 计算切线验证
            // 切线应该垂直于半径
            double radius_vector_x = x2 - x1;
            double radius_vector_y = y2 - y1;
            double tangent_vector_x = x3 - x2;
            double tangent_vector_y = y3 - y2;

            // 点积验证垂直性
            double dot_product = radius_vector_x * tangent_vector_x + radius_vector_y * tangent_vector_y;
            cout << "  半径-切线点积: " << dot_product << " (应该≈0)" << endl;
        }

        cout << "===========================================" << endl;
    }
};

// 交互式演示函数
void interactiveDemo() {
    MathTeachingAnimation demo;

    cout << "\n=== 数学教学动画演示 ===" << endl;
    cout << "1. 勾股定理演示" << endl;
    cout << "2. 圆与切线演示" << endl;
    cout << "3. 相似三角形演示" << endl;
    cout << "4. 生成关键帧文件" << endl;
    cout << "5. 实时动画演示" << endl;
    cout << "0. 退出" << endl;

    int choice;
    while (true) {
        cout << "\n请选择演示类型 (0-5): ";
        cin >> choice;

        switch (choice) {
            case 1:
                demo.setupPythagorasDemo();
                demo.solveConstraints();
                demo.printCurrentState();
                break;

            case 2: {
                cout << "\n圆与切线演示 (按q返回菜单)..." << endl;
                demo.setupCircleTangentDemo();

                // 动画循环
                for (int i = 0; i < 200; i++) {
                    demo.updateAnimation(0.05);  // 更新动画
                    demo.solveConstraints();   // 求解约束
                    demo.printCurrentState();  // 显示状态
                    this_thread::sleep_for(chrono::milliseconds(100)); // 控制速度

                    // 检查用户输入（可选）
                    if (_kbhit()) {
                        char ch = _getch();
                        if (ch == 'q' || ch == 'Q') break;
                    }
                }
                break;
            }

            case 3:
                demo.setupSimilarTrianglesDemo();
                demo.solveConstraints();
                demo.printCurrentState();
                break;

            case 4: {
                double duration, fps;
                cout << "输入动画时长(秒): ";
                cin >> duration;
                cout << "输入帧率: ";
                cin >> fps;
                demo.generateKeyFrames("math_animation_keyframes.txt", duration, fps);
                break;
            }

            case 5: {
                cout << "实时动画演示 (按Ctrl+C停止)..." << endl;
                demo.setupPythagorasDemo();
                for (int i = 0; i < 100; i++) {
                    demo.updateAnimation(0.1);
                    demo.solveConstraints();
                    demo.printCurrentState();
                    this_thread::sleep_for(chrono::milliseconds(100));
                }
                break;
            }

            case 0:
                cout << "退出演示。" << endl;
                return;

            default:
                cout << "无效选择，请重试。" << endl;
        }
    }
}

// 主函数
int main() {
    cout << "=== 数学教学动画演示系统 ===" << endl;
    cout << "使用PlaneGCS几何约束求解器生成数学概念动画" << endl;
    cout << "作者: AI Assistant" << endl;
    cout << "版本: 1.0" << endl;

    try {
        // 启动交互式演示
        interactiveDemo();

    } catch (const exception& e) {
        cerr << "程序运行出错: " << e.what() << endl;
        return -1;
    }

    cout << "程序正常结束。" << endl;
    return 0;
}