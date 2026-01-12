#include "solution_to_keyframes_core.h"
#include <ctime>
#include <set>
#include <tuple>
#include <regex>
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

        // 打印节点的所有参数
        const auto& attrs = node->getAttributes().getAllAttributes();
        if (!attrs.empty()) {
            ss << "  Parameters:\n";
            for (const auto& [key, value] : attrs) {
                // 尝试作为数值打印
                try {
                    double num_val = std::any_cast<double>(value);
                    ss << "    " << key << " (numeric): " << num_val << "\n";
                } catch (...) {
                    try {
                        string text_val = std::any_cast<string>(value);
                        ss << "    " << key << " (text): " << text_val << "\n";
                    } catch (...) {
                        ss << "    " << key << " (unknown)\n";
                    }
                }
            }
        }
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
            case ELEMENT_OF: ss << "is_element_of"; break;
            case POINT_ON_CURVE: ss << "point_on_curve"; break;
        }
        ss << "\n";

        const auto* startNode = getNode(edge->getNode1Id());
        const auto* endNode = getNode(edge->getNode2Id());

        if (startNode && startNode->getAttributes().hasText("name")) {
            ss << "- Source: " << startNode->getAttributes().getText("name") << "\n";
        } else {
            ss << "- Source: Element" << edge->getNode1Id() << "\n";
        }

        if (endNode && endNode->getAttributes().hasText("name")) {
            ss << "- Target: " << endNode->getAttributes().getText("name") << "\n";
        } else {
            ss << "- Target: Element" << edge->getNode2Id() << "\n";
        }

        // 打印边的所有参数
        const auto& attrs = edge->getAttributes().getAllAttributes();
        if (!attrs.empty()) {
            ss << "  Parameters:\n";
            for (const auto& [key, value] : attrs) {
                try {
                    double num_val = std::any_cast<double>(value);
                    ss << "    " << key << " (numeric): " << num_val << "\n";
                } catch (...) {
                    try {
                        string text_val = std::any_cast<string>(value);
                        ss << "    " << key << " (text): " << text_val << "\n";
                    } catch (...) {
                        ss << "    " << key << " (unknown)\n";
                    }
                }
            }
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
    //cout << "初始化示例几何图: Circle and moving point" << endl;

    //// 创建圆心节点
    //int center_id = geometry_graph.addNode(GeometryNodeType::POINT);
    //auto center_node = geometry_graph.getNode(center_id);
    //center_node->setPosition(0.0, 0.0);
    //center_node->getAttributes().setText("role", "center");
    //center_node->getAttributes().setText("name", "center_point");

    //// 创建圆节点
    //int circle_id = geometry_graph.addNode(GeometryNodeType::CIRCLE);
    //auto circle_node = geometry_graph.getNode(circle_id);
    //circle_node->setPosition(0.0, 0.0);
    //circle_node->setRadius(50.0);
    //circle_node->getAttributes().setText("role", "fixed_circle");
    //circle_node->getAttributes().setText("name", "reference_circle");

    //// 创建圆上动点
    //int moving_point_id = geometry_graph.addNode(GeometryNodeType::POINT);
    //auto moving_node = geometry_graph.getNode(moving_point_id);
    //moving_node->setPosition(50.0, 0.0);
    //moving_node->getAttributes().setText("role", "moving");
    //moving_node->getAttributes().setText("name", "reference_point");
    //moving_node->getAttributes().setNumeric("angle", 0.0);
    //moving_node->getAttributes().setNumeric("radius", 50.0);
    //moving_node->getAttributes().setNumeric("center_x", 0.0);
    //moving_node->getAttributes().setNumeric("center_y", 0.0);

    //// 创建几何关系边
    //geometry_graph.addEdge(moving_point_id, circle_id, GeometryRelationType::POINT_ON_CIRCLE);
    //geometry_graph.addEdge(center_id, moving_point_id, GeometryRelationType::P2P_DISTANCE);
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

    // 在程序开始时清空几何图记录文件
    clearGeometryGraphFile("geometry_graphs_evolution.txt");

    // 几何图计数器，用于记录生成的几何图序号
    int graph_counter = 0;

    // 保存当前的几何图状态
    copyGeometryStructure(geometry_graph, *(llm_processor.ai_chater.state.geometryGraph));

    cout << "初始几何图状态:" << endl;
    llm_processor.ai_chater.state.geometryGraph->print();

    // 保存初始几何图
    saveGeometryGraphWithAnimationCommand(*(llm_processor.ai_chater.state.geometryGraph), "初始状态", graph_counter++, "geometry_graphs_evolution.txt");

    // 为每个解决方案内容处理
    for (size_t i = 0; i < solution_contents.size(); i++) {
        cout << "\n--- 处理解决方案内容 " << (i+1) << " ---" << endl;
        cout << "内容: " << solution_contents[i] << endl;

        // 将 solution_contents[i] 按行分割，逐行处理而不是一股脑处理
        vector<string> content_lines;
        stringstream ss(solution_contents[i]);
        string line;
        while (getline(ss, line)) {
            // 去除行首尾空白字符
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            // 跳过空行
            if (!line.empty()) {
                content_lines.push_back(line);
            }
        }

        // 逐行处理：每行生成一个几何图，并生成对应的关键帧
        // line_idx 从 1 开始，0 表示当前小节的标题
        for (size_t line_idx = 1; line_idx < content_lines.size(); line_idx++) {
            cout << "\n=== 处理第 " << (line_idx + 1) << " 行 ===" << endl;
            cout << "行内容: " << content_lines[line_idx] << endl;

            // 步骤1: LLM跟据当前几何图和新的内容NewContent，生成新的几何图（概念几何图，不带参数）
            GeometryGraph updated_graph = llm_processor.updateGeometryGraph(*(llm_processor.ai_chater.state.geometryGraph), content_lines[line_idx]);

            cout << "LLM生成的新几何图（概念）:" << endl;
            updated_graph.print();

            // 步骤1.5: 从当前行内容中抽取参数，更新几何图
            parameter_extractor.extractParameters(updated_graph, content_lines[line_idx]);

            cout << "参数抽取后的几何图:" << endl;
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

            // 保存几何图和对应的动画指令到文件
            saveGeometryGraphWithAnimationCommand(updated_graph, content_lines[line_idx], graph_counter++, "geometry_graphs_evolution.txt", &anim_cmd);

            // 更新当前几何图状态
            *(llm_processor.ai_chater.state.geometryGraph) = std::move(updated_graph);

            // 检查更新的几何图
            cout << "当前几何图节点数: " << llm_processor.ai_chater.state.geometryGraph->getNodes().size() << endl;
            cout << "当前几何图边数: " << llm_processor.ai_chater.state.geometryGraph->getEdges().size() << endl;
        }

        // 每处理完一个 solution_content，保存一次动画指令
        if (!all_animation_commands.empty()) {
            saveAnimationCommandsToFile(all_animation_commands, "animation_commands.txt");
        }
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

        if (!cmd.element_id.empty()) {
            file << "  元素ID: " << cmd.element_id << "\n";
        }
        if (!cmd.element_name.empty()) {
            file << "  元素名称: " << cmd.element_name << "\n";
        }

        if (!cmd.parameters.empty()) {
            file << "  参数:\n";
            for (const auto& [key, value] : cmd.parameters) {
                file << "    " << key << " = " << value << "\n";
            }
        }

        if (!cmd.property_changes.empty()) {
            file << "  属性变化:\n";
            for (const auto& change : cmd.property_changes) {
                file << "    " << change.property_name
                     << ": " << change.old_value
                     << " -> " << change.new_value << "\n";
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

    // 写入JSON格式的关键帧数据
    file << "[\n";

    for (size_t i = 0; i < keyframes.size(); ++i) {
        file << keyframes[i];
        if (i < keyframes.size() - 1) {
            file << ",";
        }
        file << "\n";
    }

    file << "]\n";
    file.close();

    cout << "JSON关键帧数据已保存到文件: " << filename << " (" << keyframes.size() << " 帧)" << endl;
}

void SolutionToKeyframesProcessor::clearGeometryGraphFile(const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "无法清空几何图文件: " << filename << endl;
        return;
    }

    file << "# 几何图演化记录\n";
    file << "# 生成时间: ";

    // 获取当前时间
    auto now = chrono::system_clock::now();
    auto time_t_now = chrono::system_clock::to_time_t(now);
    file << ctime(&time_t_now);

    file << "# ========================================\n\n";
    file.close();

    cout << "几何图文件已清空并初始化: " << filename << endl;
}

void SolutionToKeyframesProcessor::saveGeometryGraphWithAnimationCommand(const GeometryGraph& graph, const string& line_content, int line_index, const string& filename, const AnimationCommand* anim_cmd) {
    ofstream file(filename, ios::app);
    if (!file.is_open()) {
        cerr << "无法打开几何图文件进行追加: " << filename << endl;
        return;
    }

    file << "========================================\n";
    file << "几何图 #" << line_index << "\n";
    file << "输入行: " << line_content << "\n";
    file << "========================================\n";

    file << graph.toString();

    // 如果提供了动画指令，也保存到文件中
    if (anim_cmd != nullptr) {
        file << "----------------------------------------\n";
        file << "对应的动画指令:\n";
        file << "  类型: " << anim_cmd->command_type << "\n";

        if (!anim_cmd->element_id.empty()) {
            file << "  元素ID: " << anim_cmd->element_id << "\n";
        }
        if (!anim_cmd->element_name.empty()) {
            file << "  元素名称: " << anim_cmd->element_name << "\n";
        }

        if (!anim_cmd->parameters.empty()) {
            file << "  参数:\n";
            for (const auto& [key, value] : anim_cmd->parameters) {
                file << "    " << key << " = " << value << "\n";
            }
        }

        if (!anim_cmd->property_changes.empty()) {
            file << "  属性变化:\n";
            for (const auto& change : anim_cmd->property_changes) {
                file << "    " << change.property_name
                     << ": " << change.old_value
                     << " -> " << change.new_value << "\n";
            }
        }
        file << "----------------------------------------\n";
    }

    file << "\n";
    file.close();

    cout << "几何图 #" << line_index << " 已追加到文件: " << filename << endl;
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
    //size_t new_graph_pos = llm_response.find("**NewGraph**:");
    //if (new_graph_pos == string::npos) {
    //    cout << "未找到LLM响应中的**NewGraph:**部分" << endl;
    //    return new_graph;
    //}

    // 提取**NewGraph:**之后的内容
    // 直接提取全部的 LLM 相应内容
    //size_t content_start = llm_response.find("\n", new_graph_pos);
    size_t content_start = 0;
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
                    geom_type = PARABOLA; // 将抛物线作为点处理
                } else if (element_part.find("Focus") != string::npos || element_part.find("focus") != string::npos) {
                    geom_type = FOCUS;
                } else if (element_part.find("Formula") != string::npos || element_part.find("formula") != string::npos || element_part.find("Distance") || element_part.find("distance")) {
                    geom_type = FOMULA;
                } else if (element_part.find("Function") != string::npos || element_part.find("function") != string::npos) {
                    geom_type = FUNCTION;
                } else if (element_part.find("Derivative") != string::npos || element_part.find("derivative") != string::npos) {
                    geom_type = DERIVATIVE;
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

                cout << "解析元素: 类型=" << (geom_type == CIRCLE ? "Circle" : geom_type == LINE ? "Line" : geom_type == POINT ? "Point" : geom_type == PARABOLA ? "Parabola" : geom_type == FOCUS ? "Focus" : geom_type == FOMULA ? "Fomula" : "Point")
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

        // 解析几何关系行，例如："1. is_element_of"
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
                    rel_type = ELEMENT_OF;
                } else if (relation_part.find("point_on_curve") != string::npos) {
                    rel_type = POINT_ON_CURVE;
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
                    if (start_line.find("Source:") != string::npos || start_line.find("- Source:") != string::npos) {
                        size_t colon_pos = start_line.find("Source:");
                        if (colon_pos != string::npos) {
                            start_node_name = start_line.substr(colon_pos + 7);
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

                cout << "关系: " << relation_part << ", Source: '" << start_node_name << "' ("
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
    cout << "\n=== 比较几何图差异，解析动画指令 ===" << endl;

    AnimationCommand cmd;

    // 构建节点名称到节点的映射
    map<string, const GeometryNode*> old_nodes_by_name, new_nodes_by_name;
    for (const auto& node : old_graph.getNodes()) {
        if (node->getAttributes().hasText("name")) {
            old_nodes_by_name[node->getAttributes().getText("name")] = node.get();
        }
    }
    for (const auto& node : new_graph.getNodes()) {
        if (node->getAttributes().hasText("name")) {
            new_nodes_by_name[node->getAttributes().getText("name")] = node.get();
        }
    }

    // ========== 规则1: 检测 ADD_ELEMENT（新增节点） ==========
    for (const auto& [name, new_node] : new_nodes_by_name) {
        if (old_nodes_by_name.find(name) == old_nodes_by_name.end()) {
            cmd.command_type = "ADD_ELEMENT";
            cmd.element_name = name;
            cmd.element_id = to_string(new_node->getId());
            cout << "检测到新增节点: " << name << " (ID: " << new_node->getId() << ", 类型: ";
            switch (new_node->getType()) {
                case POINT: cout << "Point"; break;
                case CIRCLE: cout << "Circle"; break;
                case LINE: cout << "Line"; break;
            }
            cout << ")" << endl;
            return cmd;
        }
    }

    // ========== 规则2: 检测 REMOVE_ELEMENT（删除节点） ==========
    for (const auto& [name, old_node] : old_nodes_by_name) {
        if (new_nodes_by_name.find(name) == new_nodes_by_name.end()) {
            cmd.command_type = "REMOVE_ELEMENT";
            cmd.element_name = name;
            cmd.element_id = to_string(old_node->getId());
            cout << "检测到删除节点: " << name << " (ID: " << old_node->getId() << ")" << endl;
            return cmd;
        }
    }

    // ========== 规则3: 检测 SCALE_RADIUS（半径缩放） ==========
    // 必须在位置变化检测之前，因为缩放可能导致位置计算变化
    for (const auto& [name, new_node] : new_nodes_by_name) {
        auto old_it = old_nodes_by_name.find(name);
        if (old_it == old_nodes_by_name.end()) continue;

        const GeometryNode* old_node = old_it->second;

        if (new_node->getType() == CIRCLE && old_node->getType() == CIRCLE) {
            double old_radius = old_node->getRadius();
            double new_radius = new_node->getRadius();

            if (abs(new_radius - old_radius) > 1e-6) {
                // 检查圆心是否保持不变
                double old_center_x = old_node->getX();
                double old_center_y = old_node->getY();
                double new_center_x = new_node->getX();
                double new_center_y = new_node->getY();

                double center_distance = sqrt(
                    pow(new_center_x - old_center_x, 2) +
                    pow(new_center_y - old_center_y, 2)
                );

                if (center_distance < 1e-6) {
                    cmd.command_type = "SCALE_RADIUS";
                    cmd.element_name = name;
                    cmd.element_id = to_string(new_node->getId());
                    cmd.parameters["OLD_RADIUS"] = to_string(old_radius);
                    cmd.parameters["NEW_RADIUS"] = to_string(new_radius);
                    cmd.parameters["FACTOR"] = to_string(new_radius / old_radius);

                    AnimationCommand::PropertyChange change;
                    change.property_name = "radius";
                    change.old_value = to_string(old_radius);
                    change.new_value = to_string(new_radius);
                    cmd.property_changes.push_back(change);

                    cout << "检测到半径缩放: 节点 " << name
                         << " 半径从 " << old_radius << " 变为 " << new_radius
                         << " (缩放因子: " << (new_radius / old_radius) << ")" << endl;
                    return cmd;
                }
            }
        }
    }

    // ========== 规则4: 检测 ROTATE_ON_CIRCLE（圆周运动） ==========
    // 必须在 MOVE_LINEAR 之前，因为圆周运动优先级更高
    for (const auto& [name, new_node] : new_nodes_by_name) {
        auto old_it = old_nodes_by_name.find(name);
        if (old_it == old_nodes_by_name.end()) continue;

        const GeometryNode* old_node = old_it->second;

        // 只有点类型才能进行圆周运动
        if (new_node->getType() != POINT || old_node->getType() != POINT) {
            continue;
        }

        // 检测位置变化
        double old_x = old_node->getX();
        double old_y = old_node->getY();
        double new_x = new_node->getX();
        double new_y = new_node->getY();

        double distance = sqrt(pow(new_x - old_x, 2) + pow(new_y - old_y, 2));

        if (distance > 1e-6) {
            // 检查在 old_graph 和 new_graph 中，该点是否都与同一个圆存在 POINT_ON_CIRCLE 关系
            int old_circle_id = -1;
            int new_circle_id = -1;
            bool old_on_circle = false;
            bool new_on_circle = false;

            // 在 old_graph 中查找 POINT_ON_CIRCLE 关系
            for (const auto& edge : old_graph.getEdges()) {
                if (edge->getNode1Id() == old_node->getId() &&
                    edge->getRelationType() == POINT_ON_CIRCLE) {
                    old_circle_id = edge->getNode2Id();
                    old_on_circle = true;
                    break;
                }
                if (edge->getNode2Id() == old_node->getId() &&
                    edge->getRelationType() == POINT_ON_CIRCLE) {
                    old_circle_id = edge->getNode1Id();
                    old_on_circle = true;
                    break;
                }
            }

            // 在 new_graph 中查找 POINT_ON_CIRCLE 关系
            for (const auto& edge : new_graph.getEdges()) {
                if (edge->getNode1Id() == new_node->getId() &&
                    edge->getRelationType() == POINT_ON_CIRCLE) {
                    new_circle_id = edge->getNode2Id();
                    new_on_circle = true;
                    break;
                }
                if (edge->getNode2Id() == new_node->getId() &&
                    edge->getRelationType() == POINT_ON_CIRCLE) {
                    new_circle_id = edge->getNode1Id();
                    new_on_circle = true;
                    break;
                }
            }

            // 如果在两个图中都存在 POINT_ON_CIRCLE 关系，且关联的是同一个圆
            if (old_on_circle && new_on_circle && old_circle_id == new_circle_id) {
                cmd.command_type = "ROTATE_ON_CIRCLE";
                cmd.element_name = name;
                cmd.element_id = to_string(new_node->getId());

                // 尝试获取角度和半径参数
                double old_angle = old_node->getAttributes().getNumeric("angle", 0.0);
                double new_angle = new_node->getAttributes().getNumeric("angle", 0.0);
                double radius = new_node->getAttributes().getNumeric("radius", 50.0);

                double angle_diff = new_angle - old_angle;

                cmd.parameters["ANGLE_DELTA"] = to_string(angle_diff * 180.0 / M_PI);
                cmd.parameters["RADIUS"] = to_string(radius);
                cmd.parameters["CIRCLE_ID"] = to_string(old_circle_id);

                AnimationCommand::PropertyChange angle_change;
                angle_change.property_name = "angle";
                angle_change.old_value = to_string(old_angle);
                angle_change.new_value = to_string(new_angle);
                cmd.property_changes.push_back(angle_change);

                AnimationCommand::PropertyChange x_change, y_change;
                x_change.property_name = "x";
                x_change.old_value = to_string(old_x);
                x_change.new_value = to_string(new_x);
                cmd.property_changes.push_back(x_change);

                y_change.property_name = "y";
                y_change.old_value = to_string(old_y);
                y_change.new_value = to_string(new_y);
                cmd.property_changes.push_back(y_change);

                cout << "检测到圆周运动: 节点 " << name
                     << " 角度变化=" << (angle_diff * 180.0 / M_PI)
                     << "°, 半径=" << radius << endl;
                return cmd;
            }
        }
    }

    // ========== 规则5: 检测 MOVE_LINEAR（线性移动） ==========
    for (const auto& [name, new_node] : new_nodes_by_name) {
        auto old_it = old_nodes_by_name.find(name);
        if (old_it == old_nodes_by_name.end()) continue;

        const GeometryNode* old_node = old_it->second;

        // 检测位置变化
        double old_x = old_node->getX();
        double old_y = old_node->getY();
        double new_x = new_node->getX();
        double new_y = new_node->getY();

        double dx = new_x - old_x;
        double dy = new_y - old_y;
        double distance = sqrt(dx * dx + dy * dy);

        if (distance > 1e-6) {
            cmd.command_type = "MOVE_LINEAR";
            cmd.element_name = name;
            cmd.element_id = to_string(new_node->getId());
            cmd.parameters["DELTA_X"] = to_string(dx);
            cmd.parameters["DELTA_Y"] = to_string(dy);

            AnimationCommand::PropertyChange x_change, y_change;
            x_change.property_name = "x";
            x_change.old_value = to_string(old_x);
            x_change.new_value = to_string(new_x);
            cmd.property_changes.push_back(x_change);

            y_change.property_name = "y";
            y_change.old_value = to_string(old_y);
            y_change.new_value = to_string(new_y);
            cmd.property_changes.push_back(y_change);

            cout << "检测到线性移动: 节点 " << name
                 << " 从(" << old_x << "," << old_y
                 << ") 到(" << new_x << "," << new_y
                 << "), dx=" << dx << ", dy=" << dy << endl;
            return cmd;
        }
    }

    // ========== 规则6: 检测 ADD_ATTRIBUTE（添加新属性） ==========
    for (const auto& [name, new_node] : new_nodes_by_name) {
        auto old_it = old_nodes_by_name.find(name);
        if (old_it == old_nodes_by_name.end()) continue;

        const GeometryNode* old_node = old_it->second;

        const auto& old_attrs = old_node->getAttributes().getAllAttributes();
        const auto& new_attrs = new_node->getAttributes().getAllAttributes();

        // 检查新增的属性
        for (const auto& [attr_name, new_attr_value] : new_attrs) {
            if (old_attrs.find(attr_name) == old_attrs.end()) {
                cmd.command_type = "ADD_ATTRIBUTE";
                cmd.element_name = name;
                cmd.element_id = to_string(new_node->getId());

                AnimationCommand::PropertyChange change;
                change.property_name = attr_name;
                change.old_value = "";
                change.new_value = anyToString(new_attr_value);
                cmd.property_changes.push_back(change);

                cout << "检测到添加属性: 节点 " << name
                     << " 新增属性 " << attr_name << " = "
                     << anyToString(new_attr_value) << endl;
                return cmd;
            }
        }
    }

    // ========== 规则7: 检测 MODIFY_ATTRIBUTE（属性修改） ==========
    // 排除位置属性（x, y）和形状属性（radius），这些已在前面处理
    const set<string> excluded_attrs = {"x", "y", "radius", "name"};

    for (const auto& [name, new_node] : new_nodes_by_name) {
        auto old_it = old_nodes_by_name.find(name);
        if (old_it == old_nodes_by_name.end()) continue;

        const GeometryNode* old_node = old_it->second;

        const auto& old_attrs = old_node->getAttributes().getAllAttributes();
        const auto& new_attrs = new_node->getAttributes().getAllAttributes();

        // 检查属性值变化
        for (const auto& [attr_name, new_attr_value] : new_attrs) {
            // 跳过排除的属性
            if (excluded_attrs.find(attr_name) != excluded_attrs.end()) {
                continue;
            }

            auto old_attr_it = old_attrs.find(attr_name);
            if (old_attr_it != old_attrs.end()) {
                string old_val_str = anyToString(old_attr_it->second);
                string new_val_str = anyToString(new_attr_value);

                if (old_val_str != new_val_str) {
                    cmd.command_type = "MODIFY_ATTRIBUTE";
                    cmd.element_name = name;
                    cmd.element_id = to_string(new_node->getId());

                    AnimationCommand::PropertyChange change;
                    change.property_name = attr_name;
                    change.old_value = old_val_str;
                    change.new_value = new_val_str;
                    cmd.property_changes.push_back(change);

                    cout << "检测到属性修改: 节点 " << name
                         << " 属性 " << attr_name
                         << " 从 " << old_val_str << " 变为 " << new_val_str << endl;
                    return cmd;
                }
            }
        }
    }

    // ========== 规则8: 检测 MODIFY_STRUCTURE（结构变化 - 边） ==========
    if (old_graph.getEdges().size() != new_graph.getEdges().size()) {
        cmd.command_type = "MODIFY_STRUCTURE";
        cmd.parameters["old_node_count"] = to_string(old_graph.getNodes().size());
        cmd.parameters["new_node_count"] = to_string(new_graph.getNodes().size());
        cmd.parameters["old_edge_count"] = to_string(old_graph.getEdges().size());
        cmd.parameters["new_edge_count"] = to_string(new_graph.getEdges().size());
        cout << "检测到结构变化（边）: 边数量从 " << old_graph.getEdges().size()
             << " 变为 " << new_graph.getEdges().size() << endl;
        return cmd;
    }

    // ========== 规则9: 默认 NO_CHANGE（无变化） ==========
    cmd.command_type = "NO_CHANGE";
    cout << "未检测到几何图变化" << endl;
    return cmd;
}

// 辅助函数：将any类型转换为字符串
string SolutionToKeyframesProcessor::LLMGeometryProcessor::anyToString(const std::any& value) {
    try {
        return std::any_cast<string>(value);
    } catch (...) {
        try {
            return to_string(std::any_cast<double>(value));
        } catch (...) {
            return "<unknown>";
        }
    }
}

vector<string> SolutionToKeyframesProcessor::LLMGeometryProcessor::generateKeyframes(const vector<AnimationCommand>& commands) {
    cout << "\n=== 生成JSON格式关键帧 ===" << endl;
    vector<string> keyframes;

    int frame_index = 0;
    int frames_per_command = 10;  // 每个动画指令生成10帧

    // 为每个动画指令生成关键帧数据
    for (size_t cmd_idx = 0; cmd_idx < commands.size(); ++cmd_idx) {
        const auto& cmd = commands[cmd_idx];

        cout << "处理动画指令 " << (cmd_idx + 1) << ": " << cmd.command_type << endl;

        if (cmd.command_type == "NO_CHANGE") {
            // 如果没有变化，仍然生成一个关键帧保持连续性
            stringstream ss;
            ss << "  {\n";
            ss << "    \"frameIndex\": " << frame_index++ << ",\n";
            ss << "    \"elementId\": \"none\",\n";
            ss << "    \"elementType\": \"none\",\n";
            ss << "    \"properties\": {\n";
            ss << "      \"action\": \"no_change\"\n";
            ss << "    }\n";
            ss << "  }";
            keyframes.push_back(ss.str());
            continue;
        }

        // 为当前指令生成多个帧
        for (int frame = 0; frame < frames_per_command; frame++) {
            double progress = (double)frame / (frames_per_command - 1);  // 0.0 到 1.0
            stringstream ss;

            ss << "  {\n";
            ss << "    \"frameIndex\": " << frame_index++ << ",\n";
            ss << "    \"elementId\": \"" << cmd.element_id << "\",\n";
            ss << "    \"elementType\": \"" << cmd.command_type << "\",\n";
            ss << "    \"properties\": {\n";
            ss << "      \"elementName\": \"" << cmd.element_name << "\",\n";
            ss << "      \"progress\": " << progress << ",\n";

            // 添加通用参数
            if (!cmd.parameters.empty()) {
                for (auto it = cmd.parameters.begin(); it != cmd.parameters.end(); ++it) {
                    if (it != cmd.parameters.begin()) ss << ",\n";
                    ss << "      \"" << it->first << "\": \"" << it->second << "\"";
                }
                ss << "\n";
            }

            // 添加属性变化的插值信息
            if (!cmd.property_changes.empty()) {
                ss << "    },\n";
                ss << "    \"propertyChanges\": [\n";

                for (size_t i = 0; i < cmd.property_changes.size(); ++i) {
                    const auto& change = cmd.property_changes[i];

                    ss << "      {\n";
                    ss << "        \"propertyName\": \"" << change.property_name << "\",\n";
                    ss << "        \"oldValue\": \"" << change.old_value << "\",\n";
                    ss << "        \"newValue\": \"" << change.new_value << "\",\n";

                    // 计算插值
                    if (change.old_value.empty()) {
                        ss << "        \"currentValue\": \"" << change.new_value << "\"\n";
                    } else {
                        try {
                            double old_val = stod(change.old_value);
                            double new_val = stod(change.new_value);
                            double current_val = old_val + (new_val - old_val) * progress;
                            ss << "        \"currentValue\": \"" << current_val << "\"\n";
                        } catch (...) {
                            // 如果不是数值，使用简单切换
                            ss << "        \"currentValue\": \"" << (progress < 0.5 ? change.old_value : change.new_value) << "\"\n";
                        }
                    }

                    ss << "      }";
                    if (i < cmd.property_changes.size() - 1) ss << ",";
                    ss << "\n";
                }

                ss << "    ]\n";
            } else {
                ss << "    }\n";
            }

            ss << "  }";

            keyframes.push_back(ss.str());
        }
    }

    cout << "生成了 " << keyframes.size() << " 个关键帧" << endl;
    return keyframes;
}

// ================ ParameterExtractor 实现 ================

ParameterExtractor::ParameterExtractor() {
    initializeDefaultPatterns();
}

void ParameterExtractor::initializeDefaultPatterns() {
    // 初始化模型库 - 节点参数的正则表达式模式

    // 点的参数模式 - 匹配多种坐标格式
    // 模式1: 匹配 LaTeX 格式 "$A(2,6)$" 或 "$B(-3,1)$"
    NodePattern point_latex_pattern;
    point_latex_pattern.node_type = POINT;
    point_latex_pattern.param_names = {"x", "y"};
    // 匹配格式: "$A(2,6)$" 或 "$B(-3,1)$" 或 "$F(0,1)$"
    point_latex_pattern.regex_pattern = R"(\$([A-Z])\(([\d\.\-]+),\s*([\d\.\-]+)\)\$)";
    node_patterns_.push_back(point_latex_pattern);

    // 模式2: 匹配不含$符号的简单坐标 "F(0,1)" 或 "F(0, 1)"
    NodePattern point_pattern;
    point_pattern.node_type = POINT;
    point_pattern.param_names = {"x", "y"};
    // 匹配格式: "F(0,1)" - 最简单且最常见的坐标表示
    point_pattern.regex_pattern = R"([A-Z]\(([\d\.\-]+),\s*([\d\.\-]+)\))";
    node_patterns_.push_back(point_pattern);

    // 模式3: 匹配坐标表达式格式 "$(x, \frac{x^2}{4})$" - 专门匹配M点这类参数化坐标
    NodePattern point_M_coord_pattern;
    point_M_coord_pattern.node_type = POINT;
    point_M_coord_pattern.param_names = {"x_expr", "y_expr"};
    // 匹配格式: "point $M$ ... with coordinates $(x, \frac{x^2}{4})$"
    // 这个模式会匹配完整的句子结构，提取出坐标表达式
    point_M_coord_pattern.regex_pattern = R"(point\s+\$([A-Z])\.*\s+(with\s+coordinates\s+)?\$\(([^,]+),\s*([^\)]+)\)\$)";
    node_patterns_.push_back(point_M_coord_pattern);

    // 模式4: 匹配任意坐标表达式格式 "$(x, \frac{x^2}{4})$" - 通用模式
    NodePattern point_coord_expr_pattern;
    point_coord_expr_pattern.node_type = POINT;
    point_coord_expr_pattern.param_names = {"coord_expr"};
    // 匹配格式: "$(x, \frac{x^2}{4})$" 或 "$(x, y)$" - 包含字母和表达式的坐标
    point_coord_expr_pattern.regex_pattern = R"(\$\(.*\)\$)";
    node_patterns_.push_back(point_coord_expr_pattern);

    // 圆的参数模式
    NodePattern circle_pattern;
    circle_pattern.node_type = CIRCLE;
    circle_pattern.param_names = {"center_x", "center_y", "radius"};
    // 匹配格式如: "Circle C1 with center at (0, 0) and radius 50"
    circle_pattern.regex_pattern = R"(circle[\s\w*]*center[^)]*([\d.]+)[^)]*([\d.]+)[^)]*\)[^)]*radius[\s:]*([\d.]+))";
    node_patterns_.push_back(circle_pattern);

    // 线的参数模式
    NodePattern line_pattern;
    line_pattern.node_type = LINE;
    line_pattern.param_names = {"x1", "y1", "x2", "y2"};
    // 匹配格式如: "Line L1 from (0, 0) to (10, 10)"
    line_pattern.regex_pattern = R"(line[\s\w]*\([^)]*([\d.]+)[^)]*([\d.]+)[^)]*\)[^)]*\([^)]*([\d.]+)[^)]*([\d.]+)\))";
    node_patterns_.push_back(line_pattern);

    // 抛物线的参数模式 - 匹配方程格式（多种格式）
    NodePattern parabola_pattern;
    parabola_pattern.node_type = PARABOLA;
    parabola_pattern.param_names = {"equation"};
    // 匹配任何包含 x^2 的 LaTeX 公式
    // 格式如: "$x^2 = 4y$" 或 "$y = x^2/4$"
    // 注意：这个模式会匹配整个公式，包括 $...$
    parabola_pattern.regex_pattern = R"(\$[^\$]*[xX]\^2[^\$]*\$)";
    node_patterns_.push_back(parabola_pattern);

    // 抛物线的第二种模式 - 匹配包含 "parabola" 关键词的文本
    NodePattern parabola_keyword_pattern;
    parabola_keyword_pattern.node_type = PARABOLA;
    parabola_keyword_pattern.param_names = {"equation_text"};
    // 匹配: "parabola $x^2 = 4y$" 中的完整部分
    parabola_keyword_pattern.regex_pattern = R"(([Pp]arabola[^\$]*)?\$([^\$]+)\$)";
    node_patterns_.push_back(parabola_keyword_pattern);

    // 焦点的参数模式 - 匹配 "$F(0,1)$" 格式
    NodePattern focus_pattern;
    focus_pattern.node_type = FOCUS;
    focus_pattern.param_names = {"x", "y"};
    // 匹配格式: "focus is $F(0,1)$"
    focus_pattern.regex_pattern = R"(\$F\(([\d\.\-]+),\s*([\d\.\-]+)\)\$)";
    node_patterns_.push_back(focus_pattern);

    // 公式的参数模式 - 匹配 LaTeX 公式
    NodePattern fomula_pattern;
    fomula_pattern.node_type = FOMULA;
    fomula_pattern.param_names = {"expression"};
    // 匹配格式如: "$f(x) = |MF| + |MA| + |MB|$"
    fomula_pattern.regex_pattern = R"(\$f\([^\)]+\)\s*=\s*[^\$]+\$)";
    node_patterns_.push_back(fomula_pattern);

    // 坐标表达式的模式 - 匹配 "$(x, \frac{x^2}{4})$" 格式
    NodePattern coord_expr_pattern;
    coord_expr_pattern.node_type = POINT;
    coord_expr_pattern.param_names = {"coord_expr"};
    // 匹配格式: "Let point $M$ lie on the parabola with coordinates $(x, \frac{x^2}{4})$"
    // 这个模式匹配: "point" + $ + 字母 + $ + ... + $( + 表达式 + )$
    coord_expr_pattern.regex_pattern = R"(point\s+\$([A-Z])\$\s+.*?with\s+coordinates\s+\$\((.*?)\)\$)";
    node_patterns_.push_back(coord_expr_pattern);

    // 初始化约束库 - 边参数的正则表达式模式

    // 距离值模式 - 匹配 "|MF| = 2.096" 格式
    EdgePattern distance_pattern;
    distance_pattern.relation_type = P2P_DISTANCE;
    distance_pattern.param_names = {"distance_value"};
    // 匹配格式: "|MF| = 2.096"
    distance_pattern.regex_pattern = R"(\|[A-Z]+\|\s*=\s*([\d\.]+))";
    edge_patterns_.push_back(distance_pattern);

    // x 坐标值模式 - 匹配 "x ≈ -2.09384"
    EdgePattern x_value_pattern;
    x_value_pattern.relation_type = P2P_DISTANCE;
    x_value_pattern.param_names = {"x_value"};
    // 匹配格式: "x \approx -2.09384" 或 "x ≈ -2.09384"
    x_value_pattern.regex_pattern = R"(x\s*\\approx\s*([\d\.\-]+))";
    edge_patterns_.push_back(x_value_pattern);

    // 点在圆上的约束
    EdgePattern on_circle_pattern;
    on_circle_pattern.relation_type = POINT_ON_CIRCLE;
    on_circle_pattern.param_names = {"angle"};  // 点在圆上的角度参数
    // 匹配格式如: "point on circle at angle 45 degrees"
    on_circle_pattern.regex_pattern = R"(angle[\w\s]*([\d.]+)[\w\s]*)degrees?)";
    edge_patterns_.push_back(on_circle_pattern);

    // 相切约束
    EdgePattern tangent_pattern;
    tangent_pattern.relation_type = TANGENT;
    tangent_pattern.param_names = {};  // 相切通常不需要数值参数
    tangent_pattern.regex_pattern = R"(tangent)";
    edge_patterns_.push_back(tangent_pattern);

    // 平行约束
    EdgePattern parallel_pattern;
    parallel_pattern.relation_type = PARALLEL;
    parallel_pattern.param_names = {};  // 平行通常不需要数值参数
    parallel_pattern.regex_pattern = R"(parallel)";
    edge_patterns_.push_back(parallel_pattern);

    // 垂直约束
    EdgePattern perpendicular_pattern;
    perpendicular_pattern.relation_type = PERPENDICULAR;
    perpendicular_pattern.param_names = {};  // 垂直通常不需要数值参数
    perpendicular_pattern.regex_pattern = R"(perpendicular)";
    edge_patterns_.push_back(perpendicular_pattern);
}

const ParameterExtractor::NodePattern* ParameterExtractor::findNodePattern(GeometryNodeType type) const {
    for (const auto& pattern : node_patterns_) {
        if (pattern.node_type == type) {
            return &pattern;
        }
    }
    return nullptr;
}

const ParameterExtractor::EdgePattern* ParameterExtractor::findEdgePattern(GeometryRelationType type) const {
    for (const auto& pattern : edge_patterns_) {
        if (pattern.relation_type == type) {
            return &pattern;
        }
    }
    return nullptr;
}

void ParameterExtractor::extractNodeParameters(GeometryNode* node, const string& solution_text) {
    if (!node) return;

    // 获取节点名称
    string node_name = node->getAttributes().getText("name");
    if (node_name.empty()) {
        cout << "  节点 " << node->getId() << " 没有名称，跳过参数抽取" << endl;
        return;
    }

    cout << "为节点 " << node->getId() << " (名称: " << node_name << ") 抽取参数:" << endl;

    // 提取节点名称中的标识字母（如从 "focus_F" 提取 "F"，从 "point_M" 提取 "M"，从 "point_A" 提取 "A"）
    auto extract_letter = [](const string& name) -> string {
        // 首先检查是否有下划线分隔的格式
        size_t underscore_pos = name.find_last_of('_');
        if (underscore_pos != string::npos && underscore_pos < name.length() - 1) {
            string suffix = name.substr(underscore_pos + 1);
            // 如果后缀是单个大写字母，返回它
            if (suffix.length() == 1 && suffix[0] >= 'A' && suffix[0] <= 'Z') {
                return suffix;
            }
            // 如果后缀有多个字符，取第一个字符（例如 "parabola_x2_4y" 中的某些情况）
            if (!suffix.empty() && suffix[0] >= 'A' && suffix[0] <= 'Z') {
                return string(1, suffix[0]);
            }
        }
        // 如果名称本身就是单个大写字母
        if (name.length() == 1 && name[0] >= 'A' && name[0] <= 'Z') {
            return name;
        }
        // 尝试从名称开头提取大写字母（例如 "FocusPoint" 中的 "F"）
        for (char c : name) {
            if (c >= 'A' && c <= 'Z') {
                return string(1, c);
            }
        }
        // 否则返回空字符串
        return "";
    };

    string node_letter = extract_letter(node_name);
    cout << "  提取的节点标识字母: '" << node_letter << "'" << endl;

    // 尝试所有匹配该节点类型的模式
    bool found_match = false;

    for (const auto& pattern : node_patterns_) {
        if (pattern.node_type != node->getType()) {
            continue;
        }

        try {
            // 编译正则表达式
            regex pattern_regex(pattern.regex_pattern, regex_constants::icase);

            // 使用 regex_iterator 查找所有匹配
            auto words_begin = sregex_iterator(solution_text.begin(), solution_text.end(), pattern_regex);
            auto words_end = sregex_iterator();

            // 遍历所有匹配，找到包含节点名称的匹配
            for (sregex_iterator it = words_begin; it != words_end; ++it) {
                smatch match = *it;
                string matched_text = match.str();

                // 检查匹配的文本是否包含节点标识
                bool matches_node = false;

                // 对于 PARABOLA、FOMULA 等特殊类型，直接使用第一个匹配
                if (node->getType() == PARABOLA || node->getType() == FOMULA) {
                    matches_node = true;
                } else if (!node_letter.empty()) {
                    // 检查是否包含节点字母
                    matches_node = matched_text.find(node_letter) != string::npos;

                    // 特殊处理：对于LaTeX格式的坐标 "$A(2,6)$"，检查捕获组1的字母是否匹配
                    if (match.size() > 1 && !node_letter.empty()) {
                        try {
                            string captured_letter = match[1].str();
                            if (captured_letter == node_letter) {
                                matches_node = true;
                                cout << "  捕获组字母匹配: '" << captured_letter << "' == '" << node_letter << "'" << endl;
                            }
                        } catch (...) {
                            // 捕获组访问失败，使用原来的匹配逻辑
                        }
                    }
                } else {
                    // 如果没有提取到字母，检查是否包含完整节点名称或关键词
                    string matched_text_lower = matched_text;
                    string node_name_lower = node_name;
                    transform(matched_text_lower.begin(), matched_text_lower.end(), matched_text_lower.begin(), ::tolower);
                    transform(node_name_lower.begin(), node_name_lower.end(), node_name_lower.begin(), ::tolower);

                    // 检查是否包含节点名称或节点名称的关键部分
                    matches_node = matched_text_lower.find(node_name_lower) != string::npos;

                    // 特殊处理：如果节点名称包含 "parabola"、"focus" 等关键词，也匹配
                    if (!matches_node) {
                        if (node_name_lower.find("parabola") != string::npos &&
                            matched_text_lower.find("parabola") != string::npos) {
                            matches_node = true;
                        } else if (node_name_lower.find("focus") != string::npos &&
                                   matched_text_lower.find("focus") != string::npos) {
                            matches_node = true;
                        }
                    }
                }

                if (matches_node) {
                    cout << "  使用模式: " << pattern.regex_pattern << endl;
                    cout << "  匹配文本: " << matched_text << endl;

                    // 提取捕获组的参数
                    // 对于LaTeX格式 "$A(2,6)$"，捕获组1是字母A，捕获组2和3是坐标
                    // 对于简单格式 "F(0,1)"，捕获组1和2是坐标
                    // 对于 "point $M$ ... with coordinates $(x, \frac{x^2}{4})$"，捕获组1是M，捕获组2是表达式
                    size_t param_index = 1;  // 第0个匹配是整个字符串，从第1个开始是捕获组

                    // 如果第一个捕获组是单个字母（节点标识符），则跳过它
                    if (match.size() > 1) {
                        string first_capture = match[1].str();
                        if (first_capture.length() == 1 && first_capture[0] >= 'A' && first_capture[0] <= 'Z') {
                            cout << "  检测到节点标识符在捕获组1: '" << first_capture << "'，跳过" << endl;
                            param_index = 2;  // 从捕获组2开始提取坐标
                        }
                    }

                    for (const auto& param_name : pattern.param_names) {
                        if (param_index < match.size()) {
                            string param_value_str = match[param_index].str();
                            try {
                                double param_value = stod(param_value_str);
                                node->getAttributes().setNumeric(param_name, param_value);
                                cout << "    " << param_name << " = " << param_value << endl;
                            } catch (...) {
                                // 如果无法转换为数值，存储为文本
                                node->getAttributes().setText(param_name, param_value_str);
                                cout << "    " << param_name << " = " << param_value_str << " (text)" << endl;
                            }
                            param_index++;
                        }
                    }
                    found_match = true;
                    break;  // 找到匹配后停止
                }
            }

            if (found_match) {
                break;  // 找到匹配的模式后停止
            }
        } catch (const regex_error& e) {
            cout << "  正则表达式错误: " << e.what() << " 模式: " << pattern.regex_pattern << endl;
        }
    }

    if (!found_match) {
        cout << "  未在解决方案文本中找到匹配的参数模式 (节点类型: " << node->getType() << ", 名称: " << node_name << ")" << endl;
    }
}

void ParameterExtractor::extractEdgeParameters(GeometryEdge* edge, const string& solution_text,
                                               const GeometryGraph& graph) {
    if (!edge) return;

    // 获取边的两个节点
    const auto* node1 = graph.getNode(edge->getNode1Id());
    const auto* node2 = graph.getNode(edge->getNode2Id());

    if (!node1 || !node2) {
        return;
    }

    string node1_name = node1->getAttributes().getText("name");
    string node2_name = node2->getAttributes().getText("name");

    if (node1_name.empty() || node2_name.empty()) {
        return;
    }

    cout << "为边 " << edge->getId() << " (" << node1_name << " <-> " << node2_name << ") 抽取参数:" << endl;

    // 提取节点名称中的标识字母（如从 "focus_F" 提取 "F"，从 "point_M" 提取 "M"）
    auto extract_letter = [](const string& name) -> string {
        size_t underscore_pos = name.find_last_of('_');
        if (underscore_pos != string::npos && underscore_pos < name.length() - 1) {
            string suffix = name.substr(underscore_pos + 1);
            // 如果后缀是单个大写字母，返回它
            if (suffix.length() == 1 && suffix[0] >= 'A' && suffix[0] <= 'Z') {
                return suffix;
            }
        }
        // 如果名称本身就是单个大写字母
        if (name.length() == 1 && name[0] >= 'A' && name[0] <= 'Z') {
            return name;
        }
        // 否则返回空字符串
        return "";
    };

    string letter1 = extract_letter(node1_name);
    string letter2 = extract_letter(node2_name);

    if (letter1.empty() || letter2.empty()) {
        cout << "  无法提取节点标识字母" << endl;
        return;
    }

    cout << "  节点标识: " << letter1 << " 和 " << letter2 << endl;

    // 尝试所有匹配该关系类型的模式
    bool found_match = false;

    for (const auto& pattern : edge_patterns_) {
        if (pattern.relation_type != edge->getRelationType()) {
            continue;
        }

        // 如果模式没有参数名，跳过
        if (pattern.param_names.empty()) {
            continue;
        }

        try {
            // 编译正则表达式
            regex pattern_regex(pattern.regex_pattern, regex_constants::icase);

            // 使用 regex_iterator 查找所有匹配
            auto words_begin = sregex_iterator(solution_text.begin(), solution_text.end(), pattern_regex);
            auto words_end = sregex_iterator();

            // 遍历所有匹配，找到对应节点的距离值
            for (sregex_iterator it = words_begin; it != words_end; ++it) {
                smatch match = *it;
                string matched_text = match.str();

                // 检查匹配的文本是否包含两个节点的标识字母（任意顺序）
                bool has_letter1 = matched_text.find(letter1) != string::npos;
                bool has_letter2 = matched_text.find(letter2) != string::npos;

                if (has_letter1 && has_letter2) {
                    cout << "  使用模式: " << pattern.regex_pattern << endl;
                    cout << "  匹配文本: " << matched_text << endl;

                    // 提取捕获组的参数
                    size_t param_index = 1;
                    for (const auto& param_name : pattern.param_names) {
                        if (param_index < match.size()) {
                            string param_value_str = match[param_index].str();
                            try {
                                double param_value = stod(param_value_str);
                                edge->getAttributes().setNumeric(param_name, param_value);
                                cout << "    " << param_name << " = " << param_value << endl;
                            } catch (...) {
                                edge->getAttributes().setText(param_name, param_value_str);
                                cout << "    " << param_name << " = " << param_value_str << " (text)" << endl;
                            }
                            param_index++;
                        }
                    }
                    found_match = true;
                    break;  // 找到匹配后停止
                }
            }

            if (found_match) {
                break;  // 找到匹配的模式后停止
            }
        } catch (const regex_error& e) {
            cout << "  正则表达式错误: " << e.what() << " 模式: " << pattern.regex_pattern << endl;
        }
    }

    if (!found_match) {
        // 不输出警告，因为边可能不需要参数
        // cout << "  未在解决方案文本中找到匹配的参数模式" << endl;
    }
}

void ParameterExtractor::extractParameters(GeometryGraph& graph, const string& solution_text) {
    cout << "\n=== 从解决方案文本中抽取参数 ===" << endl;
    cout << "解决方案文本: " << solution_text << endl << endl;

    // 为每个节点抽取参数
    cout << "--- 抽取节点参数 ---" << endl;
    for (auto& node : graph.getNodes()) {
        extractNodeParameters(node.get(), solution_text);
    }

    // 为每条边抽取参数
    cout << "\n--- 抽取边参数 ---" << endl;
    for (auto& edge : graph.getEdges()) {
        extractEdgeParameters(edge.get(), solution_text, graph);
    }

    cout << "=== 参数抽取完成 ===\n" << endl;
}