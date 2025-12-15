#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <memory>
#include <map>
#include <set>
#include <sstream>
#include <fstream>
#include <chrono>
#include <algorithm>

// 控制台UTF-8编码设置函数声明
void setConsoleUTF8();

 // 包含PlaneGCS库
#include "../src/GCS.h"
#include "../src/Geo.h"
#include "../src/Constraints.h"

// 包含AI聊天系统
#include "../aichater/AIChater.h"

using namespace GCS;
using namespace std;

// ========================
// 几何元素类型定义
// ========================

enum GeometryType {
    POINT = 0,
    LINE,
    CIRCLE,
    ELLIPSE,
    ARC,
    UNKNOWN
};

// ========================
// 几何节点属性
// ========================

class GeometryAttributes {
private:
    map<string, double> numeric_properties;
    map<string, string> text_properties;

public:
    void setNumeric(const string& key, double value) {
        numeric_properties[key] = value;
    }

    void setText(const string& key, const string& value) {
        text_properties[key] = value;
    }

    double getNumeric(const std::string& key, double default_val = 0.0) const {
        auto it = numeric_properties.find(key);
        return it != numeric_properties.end() ? it->second : default_val;
    }

    double* getNumericPtr(const string& key) {
        auto it = numeric_properties.find(key);
        return it != numeric_properties.end() ? &it->second : nullptr;
    }

    string getText(const string& key, const string& default_val = "") const {
        auto it = text_properties.find(key);
        return it != text_properties.end() ? it->second : default_val;
    }

    bool hasNumeric(const string& key) const {
        return numeric_properties.find(key) != numeric_properties.end();
    }

    bool hasText(const string& key) const {
        return text_properties.find(key) != text_properties.end();
    }

    void print() const {
        cout << "  Numeric Properties: ";
        for (const auto& [key, value] : numeric_properties) {
            cout << key << "=" << value << " ";
        }
        cout << "\n  Text Properties: ";
        for (const auto& [key, value] : text_properties) {
            cout << key << "=" << value << " ";
        }
        cout << endl;
    }
};

// ========================
// 几何节点
// ========================

class GeometryNode {
private:
    int id;
    GeometryType type;
    GeometryAttributes attributes;

    // PlaneGCS几何对象指针
    unique_ptr<GCS::Point> point_ptr;
    unique_ptr<GCS::Line> line_ptr;
    unique_ptr<GCS::Circle> circle_ptr;

    // 实际变量存储
    double x, y, radius;

public:
    GeometryNode(int node_id, GeometryType node_type)
        : id(node_id), type(node_type), x(0.0), y(0.0), radius(1.0) {

        switch (type) {
        case POINT:
            point_ptr = make_unique<GCS::Point>(&x, &y);
            attributes.setNumeric("x", x);
            attributes.setNumeric("y", y);
            break;
        case LINE:
            // 线段需要两个点
            attributes.setText("endpoints", "undefined");
            break;
        case CIRCLE:
            circle_ptr = make_unique<GCS::Circle>();
            circle_ptr->center = GCS::Point(&x, &y);
            circle_ptr->rad = &radius;
            attributes.setNumeric("center_x", x);
            attributes.setNumeric("center_y", y);
            attributes.setNumeric("radius", radius);
            break;
        default:
            break;
        }
    }

    int getId() const { return id; }
    GeometryType getType() const { return type; }
    GeometryAttributes& getAttributes() { return attributes; }

    // 获取PlaneGCS几何对象
    GCS::Point* getPoint() { return point_ptr.get(); }
    GCS::Circle* getCircle() { return circle_ptr.get(); }

    // 位置访问和设置
    double getX() const { return x; }
    double getY() const { return y; }
    double getRadius() const { return radius; }

    std::pair<double*, double*> getPositionPtr() {
        switch (type) {
        case CIRCLE:
            return { attributes.getNumericPtr("center_x"), attributes.getNumericPtr("center_y") };
            break;
        default:
            return { attributes.getNumericPtr("x"), attributes.getNumericPtr("y") };
            break;
        }
    }

    void setPosition(double new_x, double new_y) {
        x = new_x;
        y = new_y;
        attributes.setNumeric("x", x);
        attributes.setNumeric("y", y);
    }

    double* getRadiusPtr() {
        return attributes.getNumericPtr("radius");
    }

    void setRadius(double new_radius) {
        radius = new_radius;
        attributes.setNumeric("radius", radius);
    }

    void print() const {
        string type_str;
        switch (type) {
        case POINT: type_str = "Point"; break;
        case LINE: type_str = "Line"; break;
        case CIRCLE: type_str = "Circle"; break;
        default: type_str = "Unknown"; break;
        }

        cout << "节点 " << id << " (" << type_str << "): ";
        attributes.print();
    }
};

// ========================
// 几何边关系
// ========================

enum RelationType {
    P2P_DISTANCE = 0,    // 点点距离
    P2P_COINCIDENT,      // 点点重合
    POINT_ON_LINE,       // 点在线上
    POINT_ON_CIRCLE,      // 点在圆上
    TANGENT,             // 相切
    PERPENDICULAR,       // 垂直
    PARALLEL,            // 平行
    ANGLE_CONSTRAINT,    // 角度约束
    UNKNOWN_RELATION
};

class GeometryEdge {
private:
    int id;
    int node1_id, node2_id;
    RelationType relation_type;
    GeometryAttributes attributes;

public:
    GeometryEdge(int edge_id, int n1_id, int n2_id, RelationType rel_type)
        : id(edge_id), node1_id(n1_id), node2_id(n2_id), relation_type(rel_type) {
    }

    int getId() const { return id; }
    int getNode1Id() const { return node1_id; }
    int getNode2Id() const { return node2_id; }
    RelationType getRelationType() const { return relation_type; }
    GeometryAttributes& getAttributes() { return attributes; }

    void print() const {
        string relation_str;
        switch (relation_type) {
        case P2P_DISTANCE: relation_str = "点点距离"; break;
        case P2P_COINCIDENT: relation_str = "点点重合"; break;
        case POINT_ON_LINE: relation_str = "点在线上"; break;
        case POINT_ON_CIRCLE: relation_str = "on_circle"; break;
        case TANGENT: relation_str = "相切"; break;
        case PERPENDICULAR: relation_str = "垂直"; break;
        case PARALLEL: relation_str = "平行"; break;
        case ANGLE_CONSTRAINT: relation_str = "角度约束"; break;
        default: relation_str = "未知关系"; break;
        }

        cout << "边 " << id << ": 节点" << node1_id << " <-> 节点" << node2_id
            << " (" << relation_str << ")" << endl;
    }
};

// ========================
// 几何图
// ========================

class GeometryGraph {
private:
    vector<unique_ptr<GeometryNode>> nodes;
    vector<unique_ptr<GeometryEdge>> edges;
    int next_node_id;//obsolete?
    int next_edge_id;//obsolete?

public:
    // 1. 禁用拷贝（关键，显式禁用拷贝构造函数，避免使用编译器隐式生成的版本，从而拷贝unique_ptr）
    GeometryGraph(const GeometryGraph&) = delete;
    GeometryGraph& operator=(const GeometryGraph&) = delete;

    // 2. 允许移动
    GeometryGraph(GeometryGraph&&) = default;
    GeometryGraph& operator=(GeometryGraph&&) = default;

    GeometryGraph() : next_node_id(1), next_edge_id(1) {}

    // 添加节点
    int addNode(GeometryType type) {
        int node_id = next_node_id++;
        nodes.push_back(make_unique<GeometryNode>(node_id, type));
        return node_id;
    }

    // 添加边
    int addEdge(int node1_id, int node2_id, RelationType relation_type) {
        // 验证节点存在
        if (!getNode(node1_id) || !getNode(node2_id)) {
            cerr << "错误：节点不存在" << endl;
            return -1;
        }

        int edge_id = next_edge_id++;
        edges.push_back(make_unique<GeometryEdge>(edge_id, node1_id, node2_id, relation_type));
        return edge_id;
    }

    // 获取节点和边
    GeometryNode* getNode(int node_id) {
        for (auto& node : nodes) {
            if (node->getId() == node_id) return node.get();
        }
        return nullptr;
    }

    GeometryEdge* getEdge(int edge_id) {
        for (auto& edge : edges) {
            if (edge->getId() == edge_id) return edge.get();
        }
        return nullptr;
    }

    const vector<unique_ptr<GeometryNode>>& getNodes() const { return nodes; }
    const vector<unique_ptr<GeometryEdge>>& getEdges() const { return edges; }

    // 打印图结构
    void print() const {
        cout << "\n=== 几何图结构 ===" << endl;
        cout << "节点:" << endl;
        for (const auto& node : nodes) {
            node->print();
        }

        cout << "\n边:" << endl;
        for (const auto& edge : edges) {
            edge->print();
        }
        cout << "=================" << endl;
    }

    // 将图结构转为 string
    std::string toString() {
        return "";
    }
};

// ========================
// 动画指令结构体
// ========================

struct AnimationCommand {
    string command_type;
    map<string, double> parameters;
};

// ========================
// 全局几何图操作函数
// ========================

// 复制几何图结构
inline void copyGeometryStructure(const GeometryGraph& source, GeometryGraph& target) {
    // 复制所有节点
    for (const auto& node : source.getNodes()) {
        int new_id = target.addNode(node->getType());
        auto new_node = target.getNode(new_id);

        // 复制属性
        new_node->getAttributes() = node->getAttributes();
        new_node->setPosition(node->getX(), node->getY());
        if (node->getType() == CIRCLE) {
            new_node->setRadius(node->getRadius());
        }
    }

    // 复制所有边
    for (const auto& edge : source.getEdges()) {
        target.addEdge(edge->getNode1Id(), edge->getNode2Id(), edge->getRelationType());
        auto new_edge = target.getEdge(target.getEdges().size());
        if (new_edge) {
            new_edge->getAttributes() = edge->getAttributes();
        }
    }
}

// ========================
// LLM自然语言处理器（模拟）
// ========================

class LLMGeometryProcessor {
private:
    // AI聊天系统实例
    AIChater ai_chater;

    // 模拟的LLM响应缓存 - 在AI服务不可用时使用
    map<string, string> response_cache;

public:
    // LLM处理NewContent，更新几何图
    GeometryGraph updateGeometryGraph(const GeometryGraph& current_graph, const string& new_content) {
        cout << "LLM processing new content: " << new_content << endl;  // LLM处理新内容

        // 初始化状态
        ai_chater.state = IterationState();
        ai_chater.newContent = new_content;

        // 构建完整的LLM提示词
        stringstream prompt;

        // TaskInstruction
        prompt << ai_chater.taskInstructionPrompts_[TaskInstruction::GEO_GRAPH_EXTRACT] << "\n\n";

        // TaskDetail - 使用现有的buildStateToPrompt
        prompt << ai_chater.taskDetailPrompts_[TaskDetail::GEO_GRAPH_EXTRACT_DETAIL] << "\n\n";

        // OutputFormat
        prompt << ai_chater.taskOutputFormatPrompts_[OutputFormat::GEO_GRAPH_EXTRACT_OUTPUT_FORMAT] << "\n\n";

        // 使用AIChater的示例作为参考 (Use AIChater example as reference)
        prompt << ai_chater.taskExamplePrompts_[TaskExample::GEO_GRAPH_EXTRACT_EXAMPLE];

        // Add Question section after TaskExample with previous geometry graph concepts and new content
        prompt << "\n\n### Question\n";
        prompt << "**NewContent**:\n";
        prompt << ai_chater.newContent << "\n\n";

        prompt << "**CurGraph**:\n";
        prompt << "Geometry Elements List:\n";
        int element_id = 1;
        std::map<int, std::string> element_names;

        // First pass: collect element names
        for (const auto& node : current_graph.getNodes()) {
            switch (node->getType()) {
                case POINT:
                    element_names[element_id] = "Point";
                    break;
                case CIRCLE:
                    element_names[element_id] = "Circle";
                    break;
                case LINE:
                    element_names[element_id] = "Line";
                    break;
                default:
                    element_names[element_id] = "Element";
                    break;
            }
            element_id++;
        }

        // Output elements with numbers
        for (const auto& [id, name] : element_names) {
            prompt << id << "." << name << "\n";
        }

        prompt << "Geometry Relations List:\n";
        int relation_id = 1;
        for (const auto& edge : current_graph.getEdges()) {
            prompt << relation_id << ".";
            switch (edge->getRelationType()) {
                case P2P_DISTANCE:
                    prompt << "distance_constraint";
                    break;
                case POINT_ON_CIRCLE:
                    prompt << "on_circle";
                    break;
                default:
                    prompt << "relation";
                    break;
            }
            prompt << "\n";
            prompt << "- Start: Element" << edge->getNode1Id() << "\n";
            prompt << "- End: Element" << edge->getNode2Id() << "\n";
            relation_id++;
        }

        prompt << "**NewGraph**:\n\n";

        cout << "Sending prompt to LLM with length: " << prompt.str().length() << " characters" << endl;

        // 调用AIChater的DeepSeek API (Call AIChater's DeepSeek API)
        string llm_response = callLLMAPI(prompt.str());

        cout << "LLM response: " << llm_response << endl;

        // 解析LLM响应并创建新的几何图 (Parse LLM response and create new geometry graph)
        GeometryGraph new_graph = parseLLMGeometryGraphResponse(llm_response, current_graph);

        cout << "LLM geometry graph update completed" << endl;  // LLM几何图更新完成
        return new_graph;
    }

    // LLM解析两个几何图之间的变化，生成动画指令 (LLM analyzes changes between geometry graphs to generate animation commands)
    AnimationCommand parseAnimCommand(const GeometryGraph& old_graph, const GeometryGraph& new_graph) {
        cout << "LLM analyzing geometry graph changes..." << endl;  // LLM分析几何图变化

        AnimationCommand cmd;

        // 比较几何图，检测变化
        if (old_graph.getNodes().size() != new_graph.getNodes().size()) {
            cmd.command_type = "MODIFY_STRUCTURE";
            return cmd;
        }

        // 检测位置变化
        for (size_t i = 0; i < old_graph.getNodes().size() && i < new_graph.getNodes().size(); i++) {
            auto old_node = old_graph.getNodes()[i].get();
            auto new_node = new_graph.getNodes()[i].get();

            double old_x = old_node->getX();
            double old_y = old_node->getY();
            double new_x = new_node->getX();
            double new_y = new_node->getY();

            double dx = new_x - old_x;
            double dy = new_y - old_y;
            double distance = sqrt(dx * dx + dy * dy);

            if (distance > 1e-6) {
                // 检测到位置变化
                if (new_node->getAttributes().getText("role") == "moving" &&
                    new_node->getAttributes().hasNumeric("angle")) {

                    // 圆周运动
                    double old_angle = old_node->getAttributes().getNumeric("angle", 0.0);
                    double new_angle = new_node->getAttributes().getNumeric("angle", 0.0);
                    double angle_diff = new_angle - old_angle;

                    cmd.command_type = "ROTATE_ON_CIRCLE";
                    cmd.parameters["ANGLE_DELTA"] = angle_diff * 180.0 / M_PI;
                    cmd.parameters["RADIUS"] = new_node->getAttributes().getNumeric("radius", 50.0);
                } else {
                    // 线性移动
                    cmd.command_type = "MOVE_LINEAR";
                    cmd.parameters["DELTA_X"] = dx;
                    cmd.parameters["DELTA_Y"] = dy;
                }
                return cmd;
            }

            // 检测半径变化
            if (new_node->getType() == CIRCLE) {
                double old_radius = old_node->getRadius();
                double new_radius = new_node->getRadius();

                if (abs(new_radius - old_radius) > 1e-6) {
                    cmd.command_type = "SCALE_RADIUS";
                    cmd.parameters["OLD_RADIUS"] = old_radius;
                    cmd.parameters["NEW_RADIUS"] = new_radius;
                    cmd.parameters["FACTOR"] = new_radius / old_radius;
                    return cmd;
                }
            }
        }

        // 如果没有检测到变化，返回默认命令
        cmd.command_type = "NO_CHANGE";
        return cmd;
    }

    // 生成关键帧数据
    vector<string> generateKeyframes(const vector<AnimationCommand>& animation_commands) {
        cout << "生成关键帧数据..." << endl;

        vector<string> keyframes;

        // 关键帧头信息
        keyframes.push_back("# 关键帧数据格式: time node1_x node1_y node2_x node2_y ...");

        // 为每个动画指令生成关键帧
        double current_time = 0.0;
        double time_step = 0.033; // 30 FPS

        for (const auto& cmd : animation_commands) {
            // 为每个动画指令生成平滑过渡的关键帧
            int frames_per_command = 10; // 每个指令用10帧过渡

            for (int frame = 0; frame < frames_per_command; frame++) {
                double progress = (double)frame / frames_per_command;
                double time = current_time + frame * time_step;

                // 根据动画指令类型插值计算位置
                string keyframe = generateInterpolatedKeyframe(cmd, progress, time);
                keyframes.push_back(keyframe);
            }

            current_time += frames_per_command * time_step;
        }

        cout << "生成了 " << keyframes.size() << " 个关键帧" << endl;
        return keyframes;
    }

    // 生成插值关键帧
    string generateInterpolatedKeyframe(const AnimationCommand& cmd, double progress, double time) {
        stringstream ss;
        ss << fixed << setprecision(6) << time << " ";

        // 根据命令类型进行插值计算
        if (cmd.command_type == "ROTATE_ON_CIRCLE") {
            double angle = cmd.parameters.at("ANGLE_DELTA") * progress;
            double radius = cmd.parameters.at("RADIUS");

            double x = radius * cos(angle * M_PI / 180.0);
            double y = radius * sin(angle * M_PI / 180.0);

            ss << "0.000000 0.000000 " << x << " " << y << " 0.000000 0.000000 50.000000";

        } else if (cmd.command_type == "MOVE_LINEAR") {
            double dx = cmd.parameters.at("DELTA_X") * progress;
            double dy = cmd.parameters.at("DELTA_Y") * progress;

            ss << "0.000000 0.000000 " << dx << " " << dy << " 0.000000 0.000000 50.000000";

        } else if (cmd.command_type == "SCALE_RADIUS") {
            double old_radius = cmd.parameters.at("OLD_RADIUS");
            double new_radius = cmd.parameters.at("OLD_RADIUS") +
                             (cmd.parameters.at("NEW_RADIUS") - cmd.parameters.at("OLD_RADIUS")) * progress;

            ss << "0.000000 0.000000 50.000000 0.000000 0.000000 0.000000 " << new_radius;

        } else {
            // 默认无变化
            ss << "0.000000 0.000000 50.000000 0.000000 0.000000 0.000000 50.000000";
        }

        return ss.str();
    }

    // 模拟LLM响应
    string simulateLLMResponse(const string& geometry_description, const string& natural_language_command) {
        string cache_key = geometry_description + "|" + natural_language_command;

        // 检查缓存
        if (response_cache.find(cache_key) != response_cache.end()) {
            return response_cache[cache_key];
        }

        // 模拟LLM解析用户意图
        string response;

        if (natural_language_command.find("move") != string::npos) {  // 移动
            if (natural_language_command.find("circle") != string::npos) {
                response = "MOVE_POINT_ON_CIRCLE:ANGLE_INCREMENT=30";
            }
            else if (natural_language_command.find("clockwise") != string::npos) {  // 顺时针
                response = "ROTATE_CLOCKWISE:ANGLE=45";
            }
            else {
                response = "MOVE_POINT:DELTA_X=10:DELTA_Y=0";
            }
        }
        else if (natural_language_command.find("scale up") != string::npos) {
            response = "SCALE_UP:FACTOR=1.2";
        }
        else if (natural_language_command.find("scale down") != string::npos) {
            response = "SCALE_DOWN:FACTOR=0.8";
        }
        else if (natural_language_command.find("rotate") != string::npos) {
            response = "ROTATE:ANGLE=30";
        }
        else if (natural_language_command.find("reset") != string::npos) {
            response = "RESET_POSITION";
        }
        else {
            response = "DEFAULT_ANIMATION:INCREMENT_ANGLE=15";
        }

        // 缓存响应
        response_cache[cache_key] = response;
        return response;
    }

    // 解析LLM响应并生成几何图修改指令
    AnimationCommand parseLLMResponse(const string& llm_response) {
        AnimationCommand cmd;

        // 解析响应格式
        vector<string> parts;
        string token;
        stringstream ss(llm_response);
        while (getline(ss, token, ':')) {
            parts.push_back(token);
        }

        if (!parts.empty()) {
            cmd.command_type = parts[0];

            // 解析参数
            for (size_t i = 1; i < parts.size(); i++) {
                vector<string> param_parts;
                string param_token;
                stringstream param_ss(parts[i]);
                while (getline(param_ss, param_token, '=')) {
                    param_parts.push_back(param_token);
                }
                if (param_parts.size() == 2) {
                    cmd.parameters[param_parts[0]] = stod(param_parts[1]);
                }
            }
        }

        return cmd;
    }

    // 应用动画命令到几何图
    void applyCommand(GeometryGraph& graph, const AnimationCommand& cmd) {
        cout << "执行命令: " << cmd.command_type << endl;

        if (cmd.command_type == "MOVE_POINT_ON_CIRCLE") {
            // 在圆上移动点
            for (auto& node : graph.getNodes()) {
                if (node->getType() == POINT && node->getAttributes().getText("role") == "moving") {
                    double angle = cmd.parameters.at("ANGLE_INCREMENT");
                    double current_angle = node->getAttributes().getNumeric("angle", 0.0);
                    double new_angle = current_angle + angle * M_PI / 180.0;
                    double radius = node->getAttributes().getNumeric("radius", 50.0);
                    double center_x = node->getAttributes().getNumeric("center_x", 0.0);
                    double center_y = node->getAttributes().getNumeric("center_y", 0.0);

                    double new_x = center_x + radius * cos(new_angle);
                    double new_y = center_y + radius * sin(new_angle);
                    node->setPosition(new_x, new_y);
                    node->getAttributes().setNumeric("angle", new_angle);
                    break;
                }
            }
        }
        else if (cmd.command_type == "ROTATE_CLOCKWISE") {
            // 顺时针旋转
            double angle = cmd.parameters.at("ANGLE") * M_PI / 180.0;
            for (auto& node : graph.getNodes()) {
                if (node->getType() == POINT && node->getAttributes().getText("role") == "moving") {
                    double current_angle = node->getAttributes().getNumeric("angle", 0.0);
                    double new_angle = current_angle - angle; // 顺时针为负
                    double radius = node->getAttributes().getNumeric("radius", 50.0);
                    double center_x = node->getAttributes().getNumeric("center_x", 0.0);
                    double center_y = node->getAttributes().getNumeric("center_y", 0.0);

                    double new_x = center_x + radius * cos(new_angle);
                    double new_y = center_y + radius * sin(new_angle);
                    node->setPosition(new_x, new_y);
                    node->getAttributes().setNumeric("angle", new_angle);
                    break;
                }
            }
        }
        else if (cmd.command_type == "SCALE_UP") {
            // 放大
            double factor = cmd.parameters.at("FACTOR");
            for (auto& node : graph.getNodes()) {
                if (node->getType() == CIRCLE) {
                    double current_radius = node->getRadius();
                    node->setRadius(current_radius * factor);
                }
            }
        }
    }

private:
    // 调用LLM API的辅助方法
    string callLLMAPI(const string& prompt) {
        try {
            cout << "Calling AIChater DeepSeek API..." << endl;
            return AIChater::callDeepseekChat(prompt);
        } catch (const exception& e) {
            cerr << "LLM API call failed: " << e.what() << endl;

            // 将错误信息也写入文件以便调试
            std::ofstream errorFile("error_log.txt", std::ios::app);
            if (errorFile.is_open()) {
                errorFile << "LLM API call failed: " << e.what() << std::endl;
                errorFile.close();
            }

            // 返回模拟响应作为fallback
            return "Geometry Elements List:\nCircle\nPoint\nGeometry Relations List:\non_circle - point_on_circle";
        }
    }

    // 解析LLM响应并创建几何图
    GeometryGraph parseLLMGeometryGraphResponse(const string& llm_response, const GeometryGraph& current_graph) {
        GeometryGraph new_graph;

        // 首先复制当前几何图的所有元素到新图
        for (const auto& node : current_graph.getNodes()) {
            int new_id = new_graph.addNode(node->getType());
            auto* new_node = new_graph.getNode(new_id);
            new_node->setPosition(node->getX(), node->getY());
            new_node->setRadius(node->getRadius());
            new_node->getAttributes() = node->getAttributes();
        }

        for (const auto& edge : current_graph.getEdges()) {
            new_graph.addEdge(edge->getNode1Id(), edge->getNode2Id(), edge->getRelationType());
            auto* new_edge = new_graph.getEdges().back().get();
            new_edge->getAttributes() = edge->getAttributes();
        }

        // 解析LLM响应中的几何元素更新
        istringstream response_stream(llm_response);
        string line;
        bool in_elements = false;
        bool in_relations = false;
        int element_counter = 1;

        while (getline(response_stream, line)) {
            // 去除行首尾空白
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.find("Geometry Elements List") != string::npos) {
                in_elements = true;
                in_relations = false;
                element_counter = 1;
                continue;
            }

            if (line.find("Geometry Relations List") != string::npos) {
                in_elements = false;
                in_relations = true;
                element_counter = 1;
                continue;
            }

            if (in_elements && !line.empty() && (isdigit(line[0]) || line.find("Point") == 0 || line.find("Circle") == 0 || line.find("Line") == 0)) {
                parseGeometryElement(line, new_graph, element_counter);
                element_counter++;
            }

            if (in_relations && !line.empty() && (isdigit(line[0]) || line.find("on_circle") != string::npos || line.find("distance") != string::npos)) {
                parseGeometryRelation(line, new_graph, element_counter);
                element_counter++;
            }
        }

        return new_graph;
    }

    // 解析几何元素行
    void parseGeometryElement(const string& line, GeometryGraph& graph, int element_id) {
        if (line.find("抛物线") != string::npos) {
            // 如果是新添加的抛物线，创建节点表示
            int point_id = graph.addNode(POINT);
            auto* point = graph.getNode(point_id);
            point->getAttributes().setText("type", "parabola");
            point->getAttributes().setText("name", "抛物线" + to_string(element_id));

            cout << "LLM解析: 添加抛物线元素" << endl;
        }
        else if (line.find("焦点") != string::npos) {
            // 添加焦点
            int point_id = graph.addNode(POINT);
            auto* point = graph.getNode(point_id);
            point->getAttributes().setText("type", "focus");
            point->getAttributes().setText("name", "焦点");

            cout << "LLM解析: 添加焦点元素" << endl;
        }
        else if (line.find("点(") != string::npos) {
            // 解析坐标点
            size_t left_paren = line.find("(");
            size_t right_paren = line.find(")");
            if (left_paren != string::npos && right_paren != string::npos) {
                string coords = line.substr(left_paren + 1, right_paren - left_paren - 1);
                size_t comma = coords.find(",");
                if (comma != string::npos) {
                    double x = stod(coords.substr(0, comma));
                    double y = stod(coords.substr(comma + 1));

                    int point_id = graph.addNode(POINT);
                    auto* point = graph.getNode(point_id);
                    point->setPosition(x, y);

                    cout << "LLM解析: 添加坐标点(" << x << ", " << y << ")" << endl;
                }
            }
        }
    }

    // 解析几何关系行
    void parseGeometryRelation(const string& line, GeometryGraph& graph, int relation_id) {
        if (line.find("on_circle") != string::npos || line.find("is_element_of") != string::npos) {
            // 查找相关的点元素和圆元素
            int point_node = -1, circle_node = -1;

            for (const auto& node : graph.getNodes()) {
                if (node->getAttributes().getText("type") == "focus" && point_node == -1) {
                    point_node = node->getId();
                }
                if (node->getAttributes().getText("type") == "parabola" && circle_node == -1) {
                    circle_node = node->getId();
                }
                // 如果没有找到特殊类型，使用默认的点圆匹配
                if (node->getType() == POINT && point_node == -1) {
                    point_node = node->getId();
                }
            }

            // 如果没有找到圆，使用第一个点作为圆的替代
            if (circle_node == -1 && !graph.getNodes().empty()) {
                circle_node = graph.getNodes()[0]->getId();
            }

            if (point_node != -1 && circle_node != -1 && point_node != circle_node) {
                graph.addEdge(point_node, circle_node, POINT_ON_CIRCLE);
                cout << "LLM parsing: Adding point-on-circle relation" << endl;
            }
        }
    }
};

// ========================
// 几何约束验证和修正引擎
// ========================

class GeometryConstraintEngine {
private:
    System constraint_system;
    LLMGeometryProcessor llm_processor;

public:
    // 将几何图转换为PlaneGCS约束系统
    bool setupConstraints(GeometryGraph& graph) {
        constraint_system.clear();

        // 收集所有变量
        vector<double*> all_variables;

        // 声明几何对象
        map<int, GCS::Point*> gcs_points;
        map<int, GCS::Circle*> gcs_circles;

        for (auto& node : graph.getNodes()) {
            int node_id = node->getId();

            switch (node->getType()) {
            case POINT: {
                all_variables.push_back(node->getPositionPtr().first);
                all_variables.push_back(node->getPositionPtr().second);
                gcs_points[node_id] = node->getPoint();
                break;
            }
            case CIRCLE: {
                all_variables.push_back(node->getPositionPtr().first);
                all_variables.push_back(node->getPositionPtr().second);
                all_variables.push_back(node->getRadiusPtr());
                gcs_circles[node_id] = node->getCircle();
                gcs_points[node_id] = &(node->getCircle()->center);
                break;
            }
            default:
                break;
            }
        }

        // 添加约束
        for (auto& edge : graph.getEdges()) {
            int node1_id = edge->getNode1Id();
            int node2_id = edge->getNode2Id();
            auto node1 = graph.getNode(node1_id);
            auto node2 = graph.getNode(node2_id);

            switch (edge->getRelationType()) {//跟据定理、解题步骤、答案三个部分，决定验证方法
            case P2P_DISTANCE: {
                double distance = edge->getAttributes().getNumeric("distance", 50.0);
                if (gcs_points[node1_id] && gcs_points[node2_id]) {
                    constraint_system.addConstraintP2PDistance(
                        *gcs_points[node1_id], *gcs_points[node2_id], &distance);
                }
                break;
            }
            case POINT_ON_CIRCLE: {
                if (gcs_points[node1_id] && gcs_circles[node2_id]) {
                    constraint_system.addConstraintPointOnCircle(
                        *gcs_points[node1_id], *gcs_circles[node2_id]);
                }
                break;
            }
                                // 其他约束类型的实现...
            default:
                break;
            }
        }

        // 声明未知变量
        constraint_system.declareUnknowns(all_variables);
        return true;
    }

    // 验证几何图正确性
    bool validateGeometry(GeometryGraph& graph) {
        if (!setupConstraints(graph)) {
            cout << "约束设置失败" << endl;
            return false;
        }

        constraint_system.initSolution(DogLeg);
        int result = constraint_system.solve(true, DogLeg);

        if (result == Success || result == Converged) {
            cout << "几何约束验证成功" << endl;
            return true;
        }
        else {
            cout << "几何约束验证失败 (错误码: " << result << ")" << endl;
            return false;
        }
    }

    // 自动修正几何图
    bool autoCorrectGeometry(GeometryGraph& graph) {
        cout << "开始自动修正几何图..." << endl;

        // 获取LLM修正建议
        string correction_command = "APPLY_AUTO_CORRECTION:OPTIMIZE_CONSTRAINTS";
        auto cmd = llm_processor.parseLLMResponse(correction_command);

        // 应用修正策略
        // 1. 调整求解精度
        constraint_system.convergence = 1e-5;
        constraint_system.maxIter = 100;

        // 2. 尝试不同算法
        vector<Algorithm> algorithms = { DogLeg, LevenbergMarquardt, BFGS };

        for (auto algorithm : algorithms) {
            cout << "尝试算法: " << algorithm << endl;
            constraint_system.initSolution(algorithm);
            int result = constraint_system.solve(true, algorithm);

            if (result == Success || result == Converged) {
                cout << "修正成功！" << endl;
                return true;
            }
        }

        cout << "自动修正失败，需要手动干预" << endl;
        return false;
    }
};

// ========================
// 智能几何动画演示系统
// ========================

class IntelligentGeometryAnimation {
private:
    GeometryGraph geometry_graph;
    LLMGeometryProcessor llm_processor;
    GeometryConstraintEngine constraint_engine;

public:
    // 初始化示例几何图：圆与动点
    void setupSampleGeometry() {
        cout << "Initializing sample geometry graph: Circle and moving point" << endl;

        // 创建圆心节点
        int center_id = geometry_graph.addNode(POINT);
        auto center_node = geometry_graph.getNode(center_id);
        center_node->setPosition(0.0, 0.0);
        center_node->getAttributes().setText("role", "center");

        // 创建圆节点
        int circle_id = geometry_graph.addNode(CIRCLE);
        auto circle_node = geometry_graph.getNode(circle_id);
        circle_node->setPosition(0.0, 0.0); // 圆心与圆心点重合
        circle_node->setRadius(50.0);
        circle_node->getAttributes().setText("role", "fixed_circle");

        // 创建圆上动点
        int moving_point_id = geometry_graph.addNode(POINT);
        auto moving_node = geometry_graph.getNode(moving_point_id);
        moving_node->setPosition(50.0, 0.0); // 初始位置在(50,0)
        moving_node->getAttributes().setText("role", "moving");
        moving_node->getAttributes().setNumeric("angle", 0.0);
        moving_node->getAttributes().setNumeric("radius", 50.0);
        moving_node->getAttributes().setNumeric("center_x", 0.0);
        moving_node->getAttributes().setNumeric("center_y", 0.0);

        // 创建几何关系边
        geometry_graph.addEdge(moving_point_id, circle_id, POINT_ON_CIRCLE);
        geometry_graph.addEdge(center_id, moving_point_id, P2P_DISTANCE);

        // 设置距离约束
        auto distance_edge = geometry_graph.getEdge(2); // 第二条边
        distance_edge->getAttributes().setNumeric("distance", 50.0);

        geometry_graph.print();
    }

      // 从解决方案表示生成动画关键帧的完整流程
    vector<string> generateAnimationKeyframes(const vector<string>& solution_contents) {
        cout << "\n=== 开始生成动画关键帧 ===" << endl;

        vector<string> all_keyframes;
        vector<AnimationCommand> all_animation_commands;

        // 保存当前的几何图状态
        GeometryGraph current_graph;
        copyGeometryStructure(geometry_graph, current_graph);

        cout << "初始几何图状态:" << endl;
        current_graph.print();

        // 为每个解决方案内容处理
        for (size_t i = 0; i < solution_contents.size(); i++) {
            cout << "\n--- 处理解决方案内容 " << (i+1) << " ---" << endl;
            cout << "内容: " << solution_contents[i] << endl;

            // 步骤1: LLM跟据当前几何图和新的内容NewContent，生成新的几何图
            GeometryGraph updated_graph = llm_processor.updateGeometryGraph(current_graph, solution_contents[i]);

            cout << "LLM生成的新几何图:" << endl;
            updated_graph.print();

            // 步骤2: 跟据几何图中的关系，对几何图进行验证
            bool is_valid = constraint_engine.validateGeometry(updated_graph);

            // 步骤3: 若验证不通过，则自动修正几何图
            if (!is_valid) {
                cout << "几何图验证失败，开始自动修正..." << endl;
                is_valid = constraint_engine.autoCorrectGeometry(updated_graph);

                if (!is_valid) {
                    cout << "自动修正失败，跳过此关键帧" << endl;
                    continue;
                }

                cout << "自动修正成功!" << endl;
            }

            cout << "修正后的几何图:" << endl;
            updated_graph.print();

            // 步骤4: 记录从旧的几何图到新的几何图的变化，将变化解析为动画指令
            AnimationCommand anim_cmd = llm_processor.parseAnimCommand(current_graph, updated_graph);
            all_animation_commands.push_back(anim_cmd);

            cout << "解析得到的动画指令: " << anim_cmd.command_type;
            if (!anim_cmd.parameters.empty()) {
                cout << " (参数: ";
                for (const auto& [key, value] : anim_cmd.parameters) {
                    cout << key << "=" << value << " ";
                }
                cout << ")";
            }
            cout << endl;

            // 更新当前几何图状态
            current_graph = std::move(updated_graph);
        }

        // 步骤5: 跟据动画指令生成关键帧
        cout << "\n--- 生成关键帧数据 ---" << endl;
        all_keyframes = llm_processor.generateKeyframes(all_animation_commands);

        // 保存关键帧到文件
        saveKeyframesToFile(all_keyframes, "generated_animation_keyframes.txt");

        cout << "\n=== 动画关键帧生成完成 ===" << endl;
        return all_keyframes;
    }

    // 保存关键帧到文件
    void saveKeyframesToFile(const vector<string>& keyframes, const string& filename) {
        ofstream file(filename);
        if (!file.is_open()) {
            cerr << "无法创建关键帧文件: " << filename << endl;
            return;
        }

        file << "# 智能几何图动画关键帧数据\n";
        file << "# 生成时间: ";

        // 获取当前时间
        auto now = chrono::system_clock::now();
        auto time_t_now = chrono::system_clock::to_time_t(now);
        file << ctime(&time_t_now);

        file << "# 数据格式: time center_x center_y point_x point_y radius\n";
        file << "# center: 圆心坐标\n";
        file << "# point: 动点坐标\n";
        file << "# radius: 圆半径\n";
        file << "\n";

        for (const string& keyframe : keyframes) {
            file << keyframe << "\n";
        }

        file.close();
        cout << "关键帧数据已保存到: " << filename << endl;
    }

    // 运行完整的解决方案到关键帧演示
    void runSolutionToKeyframesDemo() {
        cout << "\n=== 解决方案表示到关键帧演示 ===" << endl;

        // 初始几何图
        setupSampleGeometry();

        // 模拟解决方案内容
        vector<string> solution_contents = {
            "Move the point clockwise by 30 degrees",  // 将动点顺时针旋转30度
            "Continue rotating to 90 degree position",  // 继续旋转到90度位置
            "scale up circle to 1.2x",
            "Rotate to 180 degree position",  // 旋转到180度位置
            "Scale down to original size",  // 缩小回原始大小
            "Return to starting position"  // 回到起点位置
        };

        cout << "解决方案内容数量: " << solution_contents.size() << endl;
        cout << "解决方案内容:\n";
        for (size_t i = 0; i < solution_contents.size(); i++) {
            cout << "  " << (i+1) << ". " << solution_contents[i] << endl;
        }
        cout << endl;

        // 生成动画关键帧
        auto keyframes = generateAnimationKeyframes(solution_contents);

        cout << "\n=== 演示总结 ===" << endl;
        cout << "生成了 " << keyframes.size() << " 个关键帧" << endl;
        cout << "关键帧文件: generated_animation_keyframes.txt" << endl;
        cout << "动画时长: " << (keyframes.size() * 0.033) << " 秒" << endl;
        cout << "帧率: 30 FPS" << endl;
    }

    // 处理自然语言命令并生成下一个关键帧
    void generateNextKeyframe(GeometryGraph& target_graph, const string& natural_language_command) {
        cout << "\nProcessing natural language command: " << natural_language_command << endl;  // 处理自然语言命令

        // 1. LLM理解并解析命令
        string geometry_description = "Circle and moving point geometry: A fixed circle with a point moving on it";
        string llm_response = llm_processor.simulateLLMResponse(geometry_description, natural_language_command);
        cout << "LLM response: " << llm_response << endl;

        // 2. 解析LLM响应
        auto animation_command = llm_processor.parseLLMResponse(llm_response);

        // 3. 应用命令到几何图
        llm_processor.applyCommand(target_graph, animation_command);

        // 4. 验证几何正确性
        if (!constraint_engine.validateGeometry(target_graph)) {
            cout << "几何验证失败，尝试自动修正..." << endl;
            constraint_engine.autoCorrectGeometry(target_graph);
        }

        // 5. 返回更新后的几何图
        target_graph.print();
    }

    // 运行3次循环演示
    void runThreeCycleDemo() {
        cout << "\n=== 智能几何动画三循环演示 ===" << endl;
        cout << "初始几何图：" << endl;
        setupSampleGeometry();

        vector<string> commands = {
            "move point clockwise 30 degrees on circle",
            "继续顺时针移动到90度位置",
            "complete circular motion back to start"
        };

        for (int cycle = 0; cycle < 3; cycle++) {
            cout << "\n========= 循环 " << (cycle + 1) << " =========" << endl;

            for (size_t i = 0; i < commands.size(); i++) {
                cout << "\n--- 关键帧 " << (cycle * commands.size() + i + 1) << " ---" << endl;

                try {
                    //auto updated_graph = generateNextKeyframe(commands[i]);
                    generateNextKeyframe(geometry_graph, commands[i]);

                    // 显示当前几何状态
                    cout << "当前几何状态：" << endl;
                    for (auto& node : geometry_graph.getNodes()) {
                        if (node->getAttributes().getText("role") == "moving") {
                            cout << "动点位置: (" << node->getX() << ", " << node->getY() << ")" << endl;
                            cout << "角度: " << node->getAttributes().getNumeric("angle") * 180 / M_PI << "度" << endl;
                        }
                    }

                }
                catch (const exception& e) {
                    cerr << "错误: " << e.what() << endl;
                }
            }
        }

        cout << "\n=== 三循环演示完成 ===" << endl;
    }
};
