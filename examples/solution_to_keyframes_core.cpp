#include "solution_to_keyframes_core.h"
#include <ctime>
#include <set>
#include <tuple>
#include "../src/Util.h"

// ================ GeometryGraph 实现 ================

int GeometryGraph::addNode(GeometryNodeType type) {
    int node_id = static_cast<int>(nodes_.size()) + 1;
    nodes_.push_back(make_unique<GeometryNode>(node_id, type));
    return node_id;
}

int GeometryGraph::addEdge(int node1Id, int node2Id, GeometryRelationType type) {
    int edge_id = next_edge_id_++;
    edges_.push_back(make_unique<GeometryEdge>(edge_id, node1Id, node2Id, type));
    return edge_id;
}

GeometryNode* GeometryGraph::getNode(int id) {
    for (auto& node : nodes_) {
        if (node->getId() == id) return node.get();
    }
    return nullptr;
}

const GeometryNode* GeometryGraph::getNode(int id) const {
    for (const auto& node : nodes_) {
        if (node->getId() == id) return node.get();
    }
    return nullptr;
}

void GeometryGraph::print() const {
    cout << toString() << endl;
}

string GeometryGraph::toString() const {
    stringstream ss;
    ss << "Geometry Elements List:\n";

    // 添加节点信息
    int element_id = 1;
    for (const auto& node : nodes_) {
        ss << element_id << ". ";
        switch (node->getType()) {
            case POINT: ss << "Point"; break;
            case CIRCLE: ss << "Circle"; break;
            case LINE: ss << "Line"; break;
        }
        if (node->getAttributes().hasText("name")) {
            ss << " (" << node->getAttributes().getText("name") << ")";
        }
        ss << "\n";
        element_id++;
    }

    ss << "Geometry Relations List:\n";

    // 添加边信息
    int relation_id = 1;
    for (const auto& edge : edges_) {
        ss << relation_id << ". ";
        switch (edge->getRelationType()) {
            case P2P_DISTANCE: ss << "distance_constraint"; break;
            case POINT_ON_CIRCLE: ss << "on_circle"; break;
            case TANGENT: ss << "tangent"; break;
            case PARALLEL: ss << "parallel"; break;
            case PERPENDICULAR: ss << "perpendicular"; break;
        }
        ss << "\n";

        const auto* startNode = getNode(edge->getNode1Id());
        const auto* endNode = getNode(edge->getNode2Id());

        if (startNode && startNode->getAttributes().hasText("name")) {
            ss << "- Start: " << startNode->getAttributes().getText("name") << "\n";
        } else {
            ss << "- Start: Element" << edge->getNode1Id() << "\n";
        }

        if (endNode && endNode->getAttributes().hasText("name")) {
            ss << "- Target: " << endNode->getAttributes().getText("name") << "\n";
        } else {
            ss << "- Target: Element" << edge->getNode2Id() << "\n";
        }

        relation_id++;
    }

    return ss.str();
}

// ================ SolutionToKeyframesProcessor 实现 ================

SolutionToKeyframesProcessor::SolutionToKeyframesProcessor() {
    setupSampleGeometry();
}

void SolutionToKeyframesProcessor::setupSampleGeometry() {
    cout << "初始化示例几何图: Circle and moving point" << endl;

    // 创建圆心节点
    int center_id = geometry_graph.addNode(GeometryNodeType::POINT);
    auto center_node = geometry_graph.getNode(center_id);
    center_node->setPosition(0.0, 0.0);
    center_node->getAttributes().setText("role", "center");
    center_node->getAttributes().setText("name", "center_point");

    // 创建圆节点
    int circle_id = geometry_graph.addNode(GeometryNodeType::CIRCLE);
    auto circle_node = geometry_graph.getNode(circle_id);
    circle_node->setPosition(0.0, 0.0);
    circle_node->setRadius(50.0);
    circle_node->getAttributes().setText("role", "fixed_circle");
    circle_node->getAttributes().setText("name", "reference_circle");

    // 创建圆上动点
    int moving_point_id = geometry_graph.addNode(GeometryNodeType::POINT);
    auto moving_node = geometry_graph.getNode(moving_point_id);
    moving_node->setPosition(50.0, 0.0);
    moving_node->getAttributes().setText("role", "moving");
    moving_node->getAttributes().setText("name", "reference_point");
    moving_node->getAttributes().setNumeric("angle", 0.0);
    moving_node->getAttributes().setNumeric("radius", 50.0);
    moving_node->getAttributes().setNumeric("center_x", 0.0);
    moving_node->getAttributes().setNumeric("center_y", 0.0);

    // 创建几何关系边
    geometry_graph.addEdge(moving_point_id, circle_id, GeometryRelationType::POINT_ON_CIRCLE);
    geometry_graph.addEdge(center_id, moving_point_id, GeometryRelationType::P2P_DISTANCE);
}

void SolutionToKeyframesProcessor::copyGeometryStructure(const GeometryGraph& source, GeometryGraph& target) {
    // 复制节点
    for (const auto& node : source.getNodes()) {
        int new_id = target.addNode(node->getType());
        auto* new_node = target.getNode(new_id);
        new_node->setPosition(node->getX(), node->getY());
        new_node->setRadius(node->getRadius());
        new_node->getAttributes().getAllAttributes() = node->getAttributes().getAllAttributes();
    }

    // 复制边
    for (const auto& edge : source.getEdges()) {
        target.addEdge(edge->getNode1Id(), edge->getNode2Id(), edge->getRelationType());
    }
}

vector<string> SolutionToKeyframesProcessor::generateAnimationKeyframes(const vector<string>& solution_contents) {
    cout << "\n=== 开始生成动画关键帧 ===" << endl;

    vector<string> all_keyframes;
    vector<AnimationCommand> all_animation_commands;

    // 保存当前的几何图状态
    copyGeometryStructure(geometry_graph, *(llm_processor.ai_chater.state.geometryGraph));

    cout << "初始几何图状态:" << endl;
    llm_processor.ai_chater.state.geometryGraph->print();

    // 为每个解决方案内容处理
    for (size_t i = 0; i < solution_contents.size(); i++) {
        cout << "\n--- 处理解决方案内容 " << (i+1) << " ---" << endl;
        cout << "内容: " << solution_contents[i] << endl;

        // 步骤1: LLM跟据当前几何图和新的内容NewContent，生成新的几何图
        GeometryGraph updated_graph = llm_processor.updateGeometryGraph(*(llm_processor.ai_chater.state.geometryGraph), solution_contents[i]);

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
        AnimationCommand anim_cmd = llm_processor.parseAnimCommand(*(llm_processor.ai_chater.state.geometryGraph), updated_graph);
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

        // 保存动画指令到文件
        saveAnimationCommandsToFile(all_animation_commands, "animation_commands.txt");

        // 更新当前几何图状态
        *(llm_processor.ai_chater.state.geometryGraph) = std::move(updated_graph);

        // 检查更新的几何图
        cout << "--(updated_graph)--number of nodes: " << llm_processor.ai_chater.state.geometryGraph->getNodes().size() << endl;
        cout << "--(updated_graph)--number of edges: " << llm_processor.ai_chater.state.geometryGraph->getEdges().size() << endl;
    }

    // 步骤5: 跟据动画指令生成关键帧
    cout << "\n--- 生成关键帧数据 ---" << endl;
    auto generated_keyframes = llm_processor.generateKeyframes(all_animation_commands);
    all_keyframes = generated_keyframes;

    // 保存关键帧到文件
    saveKeyframesToFile(all_keyframes, "generated_animation_keyframes.txt");

    cout << "\n=== 动画关键帧生成完成 ===" << endl;
    return all_keyframes;
}

void SolutionToKeyframesProcessor::saveAnimationCommandsToFile(const vector<AnimationCommand>& commands, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "无法创建动画指令文件: " << filename << endl;
        return;
    }

    file << "# 智能几何图动画指令\n";
    file << "# 生成时间: ";

    // 获取当前时间
    auto now = chrono::system_clock::now();
    auto time_t_now = chrono::system_clock::to_time_t(now);
    file << ctime(&time_t_now);

    file << "\n# 动画指令总数: " << commands.size() << "\n\n";

    for (size_t i = 0; i < commands.size(); ++i) {
        const auto& cmd = commands[i];
        file << "指令 " << (i + 1) << ":\n";
        file << "  类型: " << cmd.command_type << "\n";

        if (!cmd.parameters.empty()) {
            file << "  参数:\n";
            for (const auto& [key, value] : cmd.parameters) {
                file << "    " << key << " = " << value << "\n";
            }
        }
        file << "\n";
    }

    file << "# 动画指令结束\n";
    file.close();

    cout << "动画指令已保存到文件: " << filename << endl;
}

void SolutionToKeyframesProcessor::saveKeyframesToFile(const vector<string>& keyframes, const string& filename) {
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

    file << "\n# 关键帧总数: " << keyframes.size() << "\n\n";

    for (size_t i = 0; i < keyframes.size(); ++i) {
        file << keyframes[i] << "\n";
    }

    file << "\n# 关键帧数据结束\n";
    file.close();

    cout << "关键帧数据已保存到文件: " << filename << endl;
}

// ================ LLMGeometryProcessor 实现 ================

SolutionToKeyframesProcessor::LLMGeometryProcessor::LLMGeometryProcessor() {
    // 初始化AI聊天器
}

string SolutionToKeyframesProcessor::LLMGeometryProcessor::callLLMAPI(const string& prompt) {
    return ai_chater.callDeepseekChat(prompt);
}

GeometryGraph SolutionToKeyframesProcessor::LLMGeometryProcessor::updateGeometryGraph(const GeometryGraph& current_graph, const string& new_content) {
    // 构建LLM提示词（从原代码提取）
    stringstream prompt;
    prompt << ai_chater.taskInstructionPrompts_[TaskInstruction::GEO_GRAPH_EXTRACT] << "\n";
    prompt << ai_chater.taskDetailPrompts_[TaskDetail::GEO_GRAPH_EXTRACT_DETAIL] << "\n";
    prompt << ai_chater.taskOutputFormatPrompts_[OutputFormat::GEO_GRAPH_EXTRACT_OUTPUT_FORMAT] << "\n";
    prompt << ai_chater.taskExamplePrompts_[TaskExample::GEO_GRAPH_EXTRACT_EXAMPLE] << "\n";
    ai_chater.questionPrompts_[QUESTION::GEO_GRAPH_EXTRACT_QUESTION] = ai_chater.getQuestion(ai_chater.state, new_content);
    prompt << ai_chater.questionPrompts_[QUESTION::GEO_GRAPH_EXTRACT_QUESTION] << "\n";

    cout << "Sending prompt to LLM..." << endl;
    string llm_response = callLLMAPI(prompt.str());
    cout << "LLM response: " << llm_response << endl;

    return parseLLMGeometryGraphResponse(llm_response, current_graph);
}

GeometryGraph SolutionToKeyframesProcessor::LLMGeometryProcessor::parseLLMGeometryGraphResponse(const string& llm_response, const GeometryGraph& current_graph) {
    GeometryGraph new_graph;

    // 首先复制当前几何图的所有元素（节点和边）到新图
    for (const auto& node : current_graph.getNodes()) {
        int new_id = new_graph.addNode(node->getType());
        auto* new_node = new_graph.getNode(new_id);
        new_node->setPosition(node->getX(), node->getY());
        new_node->setRadius(node->getRadius());
        new_node->getAttributes().getAllAttributes() = node->getAttributes().getAllAttributes();
    }

    for (const auto& edge : current_graph.getEdges()) {
        new_graph.addEdge(edge->getNode1Id(), edge->getNode2Id(), edge->getRelationType());
    }

    // 建立现有边的集合，用于检测重复边
    // 使用三元组 (node1_id, node2_id, relation_type) 来唯一标识一条边
    set<tuple<int, int, GeometryRelationType>> existing_edges;
    for (const auto& edge : new_graph.getEdges()) {
        existing_edges.insert(make_tuple(edge->getNode1Id(), edge->getNode2Id(), edge->getRelationType()));
    }

    // 解析LLM响应中的几何元素更新
    // 查找**NewGraph:**部分
    size_t new_graph_pos = llm_response.find("**NewGraph**:");
    if (new_graph_pos == string::npos) {
        cout << "未找到LLM响应中的**NewGraph:**部分" << endl;
        return new_graph;
    }

    // 提取**NewGraph:**之后的内容
    size_t content_start = llm_response.find("\n", new_graph_pos);
    if (content_start == string::npos) {
        return new_graph;
    }
    content_start++;

    // 不需要查找结束标记，直接使用剩余的全部内容
    string response_content = llm_response.substr(content_start);

    // 解析响应内容
    istringstream response_stream(response_content);
    string line;
    bool in_elements = false;
    bool in_relations = false;
    int element_counter = 1;
    map<string, int> name_to_id_map;

    // 建立现有节点的名称映射
    for (const auto& node : new_graph.getNodes()) {
        if (node->getAttributes().hasText("name")) {
            name_to_id_map[node->getAttributes().getText("name")] = node->getId();
        }
    }

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
            continue;
        }

        // 解析几何元素行，例如："1. Point (center_point)"
        if (in_elements && !line.empty()) {
            size_t dot_pos = line.find('.');
            if (dot_pos != string::npos && dot_pos < line.length() - 1) {
                string element_part = line.substr(dot_pos + 1);
                element_part.erase(0, element_part.find_first_not_of(" \t"));
                element_part.erase(element_part.find_last_not_of(" \t") + 1);

                cout << "解析元素行: " << element_part << endl;

                // 提取元素类型
                GeometryNodeType geom_type = POINT; // 默认类型
                if (element_part.find("Circle") != string::npos || element_part.find("circle") != string::npos) {
                    geom_type = CIRCLE;
                } else if (element_part.find("Line") != string::npos || element_part.find("line") != string::npos) {
                    geom_type = LINE;
                } else if (element_part.find("Point") != string::npos || element_part.find("point") != string::npos) {
                    geom_type = POINT;
                } else if (element_part.find("Parabola") != string::npos || element_part.find("parabola") != string::npos) {
                    geom_type = POINT; // 将抛物线作为点处理
                } else if (element_part.find("Focus") != string::npos || element_part.find("focus") != string::npos) {
                    geom_type = POINT;
                } else {
                    geom_type = POINT; // 其他类型作为点处理
                }

                // 提取元素名称（括号中的部分）
                string element_name;
                size_t open_paren = element_part.find('(');
                size_t close_paren = element_part.find(')');
                if (open_paren != string::npos && close_paren != string::npos && close_paren > open_paren) {
                    element_name = element_part.substr(open_paren + 1, close_paren - open_paren - 1);
                    element_name.erase(0, element_name.find_first_not_of(" \t"));
                    element_name.erase(element_name.find_last_not_of(" \t") + 1);
                } else {
                    // 如果没有括号，使用整个部分作为名称
                    element_name = element_part;
                }

                cout << "解析元素: 类型=" << (geom_type == CIRCLE ? "Circle" : geom_type == LINE ? "Line" : "Point")
                     << ", 名称=" << element_name << endl;

                // 检查节点是否已存在
                auto existing_it = name_to_id_map.find(element_name);
                if (existing_it == name_to_id_map.end()) {
                    // 创建新节点
                    int node_id = new_graph.addNode(geom_type);
                    auto* new_node = new_graph.getNode(node_id);
                    if (new_node) {
                        new_node->getAttributes().setText("name", element_name);
                        // 设置默认位置和属性
                        new_node->setPosition(0.0, 0.0);
                        if (geom_type == CIRCLE) {
                            new_node->setRadius(50.0);
                        }
                        cout << "创建新节点: ID=" << node_id << ", 名称=" << element_name << endl;
                        // 更新名称映射
                        name_to_id_map[element_name] = node_id;
                    }
                } else {
                    // 节点已存在，更新节点类型和属性
                    int node_id = existing_it->second;
                    auto* existing_node = new_graph.getNode(node_id);
                    if (existing_node && existing_node->getType() != geom_type) {
                        cout << "更新节点类型: ID=" << node_id << ", 旧类型="
                             << (existing_node->getType() == CIRCLE ? "Circle" : existing_node->getType() == LINE ? "Line" : "Point")
                             << ", 新类型="
                             << (geom_type == CIRCLE ? "Circle" : geom_type == LINE ? "Line" : "Point") << endl;
                        // 注意：GeometryNode的类型在构造时设置，这里可能需要重新创建节点或修改设计
                        // 当前仅更新名称属性
                        existing_node->getAttributes().setText("name", element_name);
                    } else {
                        cout << "节点已存在且类型匹配: " << element_name << " (ID=" << node_id << ")" << endl;
                    }
                }
                element_counter++;
            }
        }

        // 解析几何关系行，例如："1. Rotation"
        if (in_relations && !line.empty()) {
            size_t dot_pos = line.find('.');
            if (dot_pos != string::npos && dot_pos < line.length() - 1) {
                string relation_part = line.substr(dot_pos + 1);
                relation_part.erase(0, relation_part.find_first_not_of(" \t"));
                relation_part.erase(relation_part.find_last_not_of(" \t") + 1);

                cout << "解析关系: " << relation_part << endl;

                // 解析关系类型
                GeometryRelationType rel_type = P2P_DISTANCE; // 默认类型
                if (relation_part.find("on_circle") != string::npos) {
                    rel_type = POINT_ON_CIRCLE;
                } else if (relation_part.find("tangent") != string::npos) {
                    rel_type = TANGENT;
                } else if (relation_part.find("perpendicular") != string::npos) {
                    rel_type = PERPENDICULAR;
                } else if (relation_part.find("parallel") != string::npos) {
                    rel_type = PARALLEL;
                } else if (relation_part.find("distance") != string::npos || relation_part.find("distance_constraint") != string::npos) {
                    rel_type = P2P_DISTANCE;
                } else if (relation_part.find("rotation") != string::npos) {
                    rel_type = P2P_DISTANCE; // 旋转关系作为距离约束处理
                } else if (relation_part.find("is_element_of") != string::npos) {
                    rel_type = P2P_DISTANCE;
                } else {
                    rel_type = P2P_DISTANCE;
                }

                // 继续读取下一行来获取Start信息
                string start_line, end_line;
                string start_node_name, end_node_name;

                // 读取Start行
                if (getline(response_stream, start_line)) {
                    start_line.erase(0, start_line.find_first_not_of(" \t\r\n"));
                    start_line.erase(start_line.find_last_not_of(" \t\r\n") + 1);
                    if (start_line.find("Start:") != string::npos || start_line.find("- Start:") != string::npos) {
                        size_t colon_pos = start_line.find("Start:");
                        if (colon_pos != string::npos) {
                            start_node_name = start_line.substr(colon_pos + 6);
                            start_node_name.erase(0, start_node_name.find_first_not_of(" \t"));
                            start_node_name.erase(start_node_name.find_last_not_of(" \t") + 1);
                        }
                    }
                }

                // 读取End/Target行
                if (getline(response_stream, end_line)) {
                    end_line.erase(0, end_line.find_first_not_of(" \t\r\n"));
                    end_line.erase(end_line.find_last_not_of(" \t\r\n") + 1);
                    if (end_line.find("- Target:") != string::npos || end_line.find("Target:") != string::npos) {
                        size_t colon_pos = end_line.find("Target:");
                        if (colon_pos != string::npos) {
                            end_node_name = end_line.substr(colon_pos + 7);
                            end_node_name.erase(0, end_node_name.find_first_not_of(" \t"));
                            end_node_name.erase(end_node_name.find_last_not_of(" \t") + 1);
                        }
                    } else if (end_line.find("- End:") != string::npos || end_line.find("End:") != string::npos) {
                        size_t colon_pos = end_line.find("End:");
                        if (colon_pos != string::npos) {
                            end_node_name = end_line.substr(colon_pos + 4);
                            end_node_name.erase(0, end_node_name.find_first_not_of(" \t"));
                            end_node_name.erase(end_node_name.find_last_not_of(" \t") + 1);
                        }
                    }
                }

                // 根据名称映射创建或更新关系边
                auto start_it = name_to_id_map.find(start_node_name);
                auto end_it = name_to_id_map.find(end_node_name);

                cout << "关系: " << relation_part << ", Start: '" << start_node_name << "' ("
                     << (start_it != name_to_id_map.end() ? "找到" : "未找到") << "), End: '" << end_node_name
                     << "' (" << (end_it != name_to_id_map.end() ? "找到" : "未找到") << ")" << endl;

                if (start_it != name_to_id_map.end() && end_it != name_to_id_map.end()) {
                    int start_id = start_it->second;
                    int end_id = end_it->second;

                    // 检查边是否已存在
                    auto edge_key = make_tuple(start_id, end_id, rel_type);
                    auto edge_it = existing_edges.find(edge_key);

                    if (edge_it == existing_edges.end()) {
                        // 边不存在，添加新边
                        new_graph.addEdge(start_id, end_id, rel_type);
                        existing_edges.insert(edge_key);
                        cout << "成功添加关系边: 节点" << start_id << " <-> 节点" << end_id << endl;
                    } else {
                        // 边已存在，更新边属性（当前设计下边类型不可变，但可以更新属性）
                        cout << "边已存在，更新边属性: 节点" << start_id << " <-> 节点" << end_id << endl;
                        // 可以在这里添加边的属性更新逻辑
                    }
                } else {
                    cout << "无法添加关系 - 节点名称映射失败" << endl;
                }
            }
        }
    }

    cout << "解析完成. 新图有 " << new_graph.getNodes().size() << " 个节点, "
         << new_graph.getEdges().size() << " 条边" << endl;

    return new_graph;
}

AnimationCommand SolutionToKeyframesProcessor::LLMGeometryProcessor::parseAnimCommand(const GeometryGraph& old_graph, const GeometryGraph& new_graph) {
    cout << "比较几何图差异，解析动画指令..." << endl;

    AnimationCommand cmd;

    // 比较几何图，检测变化
    if (old_graph.getNodes().size() != new_graph.getNodes().size()) {
        cmd.command_type = "MODIFY_STRUCTURE";
        cout << "检测到结构变化：节点数量改变" << endl;
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
            cout << "检测到位置变化: 节点" << old_node->getId()
                 << " 从(" << old_x << "," << old_y
                 << ") 到(" << new_x << "," << new_y << ")" << endl;

            // 检查是否为圆周运动
            if (new_node->getAttributes().getText("role") == "moving" &&
                new_node->getAttributes().hasText("angle")) {

                // 圆周运动
                double old_angle = old_node->getAttributes().getNumeric("angle", 0.0);
                double new_angle = new_node->getAttributes().getNumeric("angle", 0.0);
                double angle_diff = new_angle - old_angle;
                double radius = new_node->getAttributes().getNumeric("radius", 50.0);

                cmd.command_type = "ROTATE_ON_CIRCLE";
                cmd.parameters["ANGLE_DELTA"] = to_string(angle_diff * 180.0 / M_PI);
                cmd.parameters["RADIUS"] = to_string(radius);

                cout << "解析为圆周运动: 角度变化=" << angle_diff * 180.0 / M_PI
                     << "°, 半径=" << radius << endl;
            } else {
                // 线性移动
                cmd.command_type = "MOVE_LINEAR";
                cmd.parameters["DELTA_X"] = to_string(dx);
                cmd.parameters["DELTA_Y"] = to_string(dy);

                cout << "解析为线性移动: dx=" << dx << ", dy=" << dy << endl;
            }
            return cmd;
        }

        // 检测半径变化
        if (new_node->getType() == CIRCLE) {
            double old_radius = old_node->getRadius();
            double new_radius = new_node->getRadius();

            if (abs(new_radius - old_radius) > 1e-6) {
                cmd.command_type = "SCALE_RADIUS";
                cmd.parameters["OLD_RADIUS"] = to_string(old_radius);
                cmd.parameters["NEW_RADIUS"] = to_string(new_radius);
                cmd.parameters["FACTOR"] = to_string(new_radius / old_radius);

                cout << "检测到半径变化: 节点" << new_node->getId()
                     << " 从" << old_radius << " 到 " << new_radius << endl;
                return cmd;
            }
        }
    }

    // 检测边的变化（新增或删除的关系）
    if (old_graph.getEdges().size() != new_graph.getEdges().size()) {
        cmd.command_type = "MODIFY_STRUCTURE";
        cout << "检测到结构变化：边数量改变" << endl;
        return cmd;
    }

    // 如果没有检测到变化，返回默认命令
    cmd.command_type = "NO_CHANGE";
    cout << "未检测到几何图变化" << endl;
    return cmd;
}

vector<string> SolutionToKeyframesProcessor::LLMGeometryProcessor::generateKeyframes(const vector<AnimationCommand>& commands) {
    vector<string> keyframes;

    // 为每个动画指令生成关键帧数据
    for (size_t i = 0; i < commands.size(); ++i) {
        stringstream keyframe_ss;

        // 生成关键帧数据（时间、x、y坐标等）
        keyframe_ss << "Keyframe " << (i+1) << ": ";
        keyframe_ss << "t=" << static_cast<double>(i);
        keyframe_ss << ", x=" << (50.0 * cos(i * 3.1415926535 / 6));
        keyframe_ss << ", y=" << (50.0 * sin(i * 3.1415926535 / 6));

        keyframes.push_back(keyframe_ss.str());
    }

    return keyframes;
}