#pragma once

#include "PlaneGCS.h"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include "aichater/AIChater.h"

using namespace std;

// 动画指令结构
struct AnimationCommand {
    string command_type;
    map<string, string> parameters;

    AnimationCommand() = default;
    AnimationCommand(const string& type) : command_type(type) {}
};

// 几何图节点类型
enum GeometryNodeType {
    POINT,
    CIRCLE,
    LINE
};

// 几何图边类型
enum GeometryRelationType {
    P2P_DISTANCE,
    POINT_ON_CIRCLE,
    TANGENT,
    PARALLEL,
    PERPENDICULAR
};

// 几何图节点
class GeometryNode {
private:
    int id_;
    GeometryNodeType type_;
    double x_, y_;
    double radius_;
    map<string, any> attributes_;

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

    map<string, any>& getAttributes() { return attributes_; }
    const map<string, any>& getAttributes() const { return attributes_; }
};

// 几何图边
class GeometryEdge {
private:
    int node1Id_, node2Id_;
    GeometryRelationType relationType_;
    map<string, any> attributes_;

public:
    GeometryEdge(int node1, int node2, GeometryRelationType type)
        : node1Id_(node1), node2Id_(node2), relationType_(type) {}

    int getNode1Id() const { return node1Id_; }
    int getNode2Id() const { return node2Id_; }
    GeometryRelationType getRelationType() const { return relationType_; }

    map<string, any>& getAttributes() { return attributes_; }
    const map<string, any>& getAttributes() const { return attributes_; }
};

// 几何图结构
class GeometryGraph {
private:
    vector<unique_ptr<GeometryNode>> nodes_;
    vector<unique_ptr<GeometryEdge>> edges_;

public:
    // 移动构造函数
    GeometryGraph() = default;
    GeometryGraph(GeometryGraph&&) = default;
    GeometryGraph& operator=(GeometryGraph&&) = default;

    // 禁用拷贝构造函数和拷贝赋值运算符
    GeometryGraph(const GeometryGraph&) = delete;
    GeometryGraph& operator=(const GeometryGraph&) = delete;

    int addNode(GeometryNodeType type);
    void addEdge(int node1Id, int node2Id, GeometryRelationType type);

    GeometryNode* getNode(int id);
    const vector<unique_ptr<GeometryNode>>& getNodes() const { return nodes_; }
    const vector<unique_ptr<GeometryEdge>>& getEdges() const { return edges_; }

    void print() const;
};

// 智能几何动画系统
class IntelligentGeometryAnimation {
private:
    unique_ptr<PlaneGCS> constraint_engine;
    vector<AnimationCommand> all_animation_commands;
    vector<vector<double>> all_keyframes;

    void copyGeometryStructure(const GeometryGraph& source, GeometryGraph& target);

public:
    IntelligentGeometryAnimation();

    void generateIntelligentAnimation(const GeometryGraph& geometry_graph,
                                      const vector<string>& solution_contents);

    void saveKeyframesToFile(const vector<vector<double>>& keyframes, const string& filename);
};

// LLM几何图处理器
class LLMGeometryProcessor {
private:
    AIChater ai_chater;

public:
    LLMGeometryProcessor();

    GeometryGraph updateGeometryGraph(const GeometryGraph& current_graph, const string& new_content);
    AnimationCommand parseAnimCommand(const GeometryGraph& old_graph, const GeometryGraph& new_graph);
    vector<vector<double>> generateKeyframes(const vector<AnimationCommand>& commands);

private:
    string callLLMAPI(const string& prompt);
    GeometryGraph parseLLMGeometryGraphResponse(const string& llm_response, const GeometryGraph& current_graph);
};