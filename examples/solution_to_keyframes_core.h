#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <any>
#include "../aichater/AIChater.h"

using namespace std;

// 几何属性类
class GeometryAttributes {
private:
    map<string, any> attributes_;

public:
    void setText(const string& key, const string& value) {
        attributes_[key] = value;
    }

    string getText(const string& key) const {
        auto it = attributes_.find(key);
        if (it != attributes_.end()) {
            try {
                return any_cast<string>(it->second);
            } catch (...) {
                return "";
            }
        }
        return "";
    }

    bool hasText(const string& key) const {
        auto it = attributes_.find(key);
        if (it != attributes_.end()) {
            try {
                any_cast<string>(it->second);
                return true;
            } catch (...) {
                return false;
            }
        }
        return false;
    }

    void setNumeric(const string& key, double value) {
        attributes_[key] = value;
    }

    double getNumeric(const string& key, double defaultValue=0.0) const {
        auto it = attributes_.find(key);
        if (it != attributes_.end()) {
            try {
                return any_cast<double>(it->second);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    bool hasNumeric(const string& key) const {
        auto it = attributes_.find(key);
        if (it != attributes_.end()) {
            try {
                any_cast<double>(it->second);
                return true;
            } catch (...) {
                return false;
            }
        }
        return false;
    }

    map<string, any>& getAllAttributes() { return attributes_; }
    const map<string, any>& getAllAttributes() const { return attributes_; }
};

// 动画指令结构
struct AnimationCommand {
    string command_type;                    // 指令类型
    map<string, string> parameters;         // 通用参数
    string element_id;                      // 受影响的元素ID（如果有）
    string element_name;                    // 受影响的元素名称（如果有）

    // 用于存储具体属性的变化
    struct PropertyChange {
        string property_name;               // 属性名称
        string old_value;                   // 旧值
        string new_value;                   // 新值
    };
    vector<PropertyChange> property_changes; // 属性变化列表

    AnimationCommand() = default;
    AnimationCommand(const string& type) : command_type(type) {}
};

// 几何图节点类型
enum GeometryNodeType {
    POINT,
    CIRCLE,
    LINE,
    PARABOLA,
    FOCUS,
    FOMULA,
    FUNCTION,
    DERIVATIVE
};

// 几何图边类型
enum GeometryRelationType {
    P2P_DISTANCE,
    POINT_ON_CIRCLE,
    TANGENT,
    PARALLEL,
    PERPENDICULAR,
    ELEMENT_OF,
    POINT_ON_CURVE
};

// 几何图节点
class GeometryNode {
private:
    int id_;
    GeometryNodeType type_;
    double x_, y_;
    double radius_;
    GeometryAttributes attributes_;

public:
    GeometryNode(int id, GeometryNodeType type)
        : id_(id), type_(type), x_(0), y_(0), radius_(1.0) {}

    int getId() const { return id_; }
    GeometryNodeType getType() const { return type_; }

    double getX() const { return x_; }
    double getY() const { return y_; }
    double getRadius() const { return radius_; }

    void setPosition(double x, double y) { x_ = x; y_ = y; }
    void setRadius(double radius) { radius_ = radius; }

    GeometryAttributes& getAttributes() { return attributes_; }
    const GeometryAttributes& getAttributes() const { return attributes_; }
};

// 几何图边
class GeometryEdge {
private:
    int id_;
    int node1Id_, node2Id_;
    GeometryRelationType relationType_;
    GeometryAttributes attributes_;

public:
    GeometryEdge(int id, int node1, int node2, GeometryRelationType type)
        : id_(id), node1Id_(node1), node2Id_(node2), relationType_(type) {}

    int getId() const { return id_; }
    int getNode1Id() const { return node1Id_; }
    int getNode2Id() const { return node2Id_; }
    GeometryRelationType getRelationType() const { return relationType_; }

    GeometryAttributes& getAttributes() { return attributes_; }
    const GeometryAttributes& getAttributes() const { return attributes_; }
};

// 几何图结构
class GeometryGraph {
private:
    vector<unique_ptr<GeometryNode>> nodes_;
    vector<unique_ptr<GeometryEdge>> edges_;
    int next_edge_id_;

public:
    GeometryGraph() : next_edge_id_(1) {}

    // 移动构造函数
    GeometryGraph(GeometryGraph&&) = default;
    GeometryGraph& operator=(GeometryGraph&&) = default;

    // 禁用拷贝构造函数和拷贝赋值运算符
    GeometryGraph(const GeometryGraph&) = delete;
    GeometryGraph& operator=(const GeometryGraph&) = delete;

    int addNode(GeometryNodeType type);
    int addEdge(int node1Id, int node2Id, GeometryRelationType type);

    GeometryNode* getNode(int id);
    const GeometryNode* getNode(int id) const;
    const vector<unique_ptr<GeometryNode>>& getNodes() const { return nodes_; }
    const vector<unique_ptr<GeometryEdge>>& getEdges() const { return edges_; }

    void print() const;
    string toString() const;
};

// 几何约束引擎
class GeometryConstraintEngine {
public:
    bool validateGeometry(const GeometryGraph& graph) {
        // 简单验证逻辑
        return !graph.getNodes().empty();
    }

    bool autoCorrectGeometry(GeometryGraph& graph) {
        // 简单修正逻辑
        return true;
    }
};

// 参数抽取器 - 从解决方案文本中抽取几何元素的参数
class ParameterExtractor {
private:
    // 模型库：存储节点类型的参数匹配正则表达式
    struct NodePattern {
        GeometryNodeType node_type;
        vector<string> param_names;      // 参数名称列表
        string regex_pattern;            // 正则表达式模式
    };

    // 约束库：存储边类型的参数匹配正则表达式
    struct EdgePattern {
        GeometryRelationType relation_type;
        vector<string> param_names;      // 参数名称列表
        string regex_pattern;            // 正则表达式模式
    };

    vector<NodePattern> node_patterns_;
    vector<EdgePattern> edge_patterns_;

public:
    ParameterExtractor();
    ~ParameterExtractor() = default;

    // 初始化默认的正则表达式模式库
    void initializeDefaultPatterns();

    // 从解决方案文本中抽取参数并更新几何图
    void extractParameters(GeometryGraph& graph, const string& solution_text);

private:
    // 为节点抽取参数
    void extractNodeParameters(GeometryNode* node, const string& solution_text);

    // 为边抽取参数
    void extractEdgeParameters(GeometryEdge* edge, const string& solution_text,
                               const GeometryGraph& graph);

    // 根据节点类型查找匹配的模式
    const NodePattern* findNodePattern(GeometryNodeType type) const;

    // 根据关系类型查找匹配的模式
    const EdgePattern* findEdgePattern(GeometryRelationType type) const;
};

// 解决方案到动画关键帧处理器
class SolutionToKeyframesProcessor {
private:
    GeometryGraph geometry_graph;
    GeometryConstraintEngine constraint_engine;
    ParameterExtractor parameter_extractor;  // 参数抽取器
    vector<AnimationCommand> all_animation_commands;
    vector<string> all_keyframes;

    void copyGeometryStructure(const GeometryGraph& source, GeometryGraph& target);

public:
    SolutionToKeyframesProcessor();

    // 核心流程：从解决方案内容生成动画关键帧
    vector<string> generateAnimationKeyframes(const vector<string>& solution_contents);

    // 保存功能
    void saveAnimationCommandsToFile(const vector<AnimationCommand>& commands, const string& filename);
    void saveKeyframesToFile(const vector<string>& keyframes, const string& filename);
    void saveGeometryGraphWithAnimationCommand(const GeometryGraph& graph, const string& line_content, int line_index, const string& filename, const AnimationCommand* anim_cmd = nullptr);
    void clearGeometryGraphFile(const string& filename);

private:
    // LLM处理相关
    class LLMGeometryProcessor {
    public:
        AIChater ai_chater;

    public:
        LLMGeometryProcessor();

        GeometryGraph updateGeometryGraph(const GeometryGraph& current_graph, const string& new_content);
        AnimationCommand parseAnimCommand(const GeometryGraph& old_graph, const GeometryGraph& new_graph);
        vector<string> generateKeyframes(const vector<AnimationCommand>& commands);

    private:
        string callLLMAPI(const string& prompt);
        GeometryGraph parseLLMGeometryGraphResponse(const string& llm_response, const GeometryGraph& current_graph);
        static string anyToString(const std::any& value);  // 辅助函数
    };

    LLMGeometryProcessor llm_processor;

    // 初始化示例几何图
    void setupSampleGeometry();
};