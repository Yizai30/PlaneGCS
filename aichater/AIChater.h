#pragma once

/**
* 《Beyond Prompt Content: Enhancing LLM Performance via Content-Format Integrated Prompt Optimization》
*/

#include <string>
#include <map>
#include <sstream>
#include "../models_for_test/IterationState.h"

enum Database {
	MODEL_EXTRACT_DATABASE,
	GEO_GRAPH_EXTRACT_DATABASE
};

enum TaskInstruction {
	MODEL_EXTRACT,
	LAYOUT_TYPE_EXTRACT,
	GEO_GRAPH_EXTRACT
};

enum TaskDetail {
	MODEL_EXTRACT_DESCRIPTION,
	LAYOUT_TYPE_EXTRACT_DESCRIPTION,
	GEO_GRAPH_EXTRACT_DETAIL
};

enum OutputFormat {
	MODEL_EXTRACT_OUTPUT_FORMAT,
	LAYOUT_TYPE_EXTRACT_OUTPUT_FORMAT,
	GEO_GRAPH_EXTRACT_OUTPUT_FORMAT
};

enum TaskExample {
	GEO_GRAPH_EXTRACT_EXAMPLE
};

enum QUESTION {
	GEO_GRAPH_EXTRACT_QUESTION
};

class AIChater {
public:
	AIChater() {
		// TODO向量查询几何图
		std::string modelDatabase;
		loadJsonFileToString("layout_generator_v1/Resources/Doc/model_template.json", modelDatabase);
		taskDatabasePrompts_ = {
			{Database::GEO_GRAPH_EXTRACT_DATABASE, modelDatabase}
		};
		taskInstructionPrompts_ = { // 用角色提示词调动模型在特定领域的知识
			//{TaskInstruction::GEO_GRAPH_EXTRACT, "### TaskInstruction\n你是一位资深的几何学专家，请分析以下数学问题内容，构建完整的几何图结构。"},
			{TaskInstruction::GEO_GRAPH_EXTRACT, "### TaskInstruction\nAs an experienced geometer, please analyze the following mathematical problem and model it as a complete geometric structure."},
		};
		taskDetailPrompts_ = {
			{TaskDetail::GEO_GRAPH_EXTRACT_DETAIL, buildStateToPrompt(state)},
		};
		taskOutputFormatPrompts_ = {
			{OutputFormat::MODEL_EXTRACT_OUTPUT_FORMAT, "class Model{std::string type_;std::list<std::string> identifier_;std::map<std::string, std::any> properties_;std::list<ConstraintRule> constraints_;}"},
			{OutputFormat::LAYOUT_TYPE_EXTRACT_OUTPUT_FORMAT, "class LayoutStrategy{std::list<std::string> elementIdentifier_;std::string strategy_;}"},
			{OutputFormat::GEO_GRAPH_EXTRACT_OUTPUT_FORMAT, getGeoGraphFormat()}
		};
		taskExamplePrompts_ = {
			{TaskExample::GEO_GRAPH_EXTRACT_EXAMPLE, R"(### TaskExample
**NewContent**:
- Parabola: $x^2 = 4y$ is a special form of conic section
**CurGraph**:
Geometry Elements List:
Geometry Relations List:
**NewGraph**:
Geometry Elements List:
1.Parabola
Geometry Relations List:

**NewContent**:
- Focus: For parabola $x^2 = 4y$, the focus is $F(0,1)$
**CurGraph**:
Geometry Elements List:
1.Parabola
Geometry Relations List:
**NewGraph**:
Geometry Elements List:
1.Parabola
2.Focus
Geometry Relations List:
1.is_element_of
- Start: Focus
- Target: Parabola)"}
		};
		questionPrompts_ = {
			{QUESTION::GEO_GRAPH_EXTRACT_QUESTION, getQuestion(state, newContent)}
		};
	}
	static std::string callDeepseekChat(std::string promptString);
	// 调用AI的函数
		//std::list<Model> getModelsWithAI(std::string preprocessText);
		//std::list<LayoutStrategy> getLayoutStrategiesWithAI(std::string preprocessText);

	// 引用 IterationState
	IterationState state;
	// 引用 NewContent 作为 question
	std::string newContent;
//private:
	// 调用AI前的配置
	std::string modelTemplates_;//Model的定义
	std::map<Database, std::string>
		taskDatabasePrompts_;//数据库定义提示词
	
	// PROMPT
	std::map<TaskInstruction, std::string> taskInstructionPrompts_;// ### TaskInstruction
	std::map<TaskDetail, std::string>
		taskDetailPrompts_;// ### TaskDetail
	std::map<OutputFormat, std::string>
		taskOutputFormatPrompts_;// ### OutputFormat
	std::map<TaskExample, std::string>
		taskExamplePrompts_; // ### TaskExample
	std::map<QUESTION, std::string>
		questionPrompts_; // ### Question

	// 调用AI的缓存
	std::string modelStr_;//字符串表示的模型抽取结果
private:
	// 辅助函数
	static bool loadJsonFileToString(const std::string& filename, std::string& content);

	// 代码拼接state变量到prompts中
	std::string buildStateToPrompt(IterationState& state) {
		std::stringstream prompt;

		// 固定部分
		prompt << "### TaskDetail\n";
		//prompt << "添加迭代上下文\n";

		// 动态部分
		//if (state.confidence > 0.0) {
		//	prompt << "=== 当前分析状态 ===\n";
		//	prompt << "📊 置信度: " << state.confidence << "\n";

			/*if (!state.elementCounts.empty()) {
				prompt << "🔍 已识别几何元素: ";
				for (const auto& [element, count] : state.elementCounts) {
					prompt << element << "x" << count << " ";
				}
				prompt << "\n";
			}*/

			/*if (!state.missingElements.empty()) {
				prompt << "❓ 需要补充: ";
				for (const auto& missing : state.missingElements) {
					prompt << missing << " ";
				}
				prompt << "\n";
			}*/

		//	prompt << "💡 请运用你的几何学专长，补充完整的几何图结构。\n\n";
		//}

		//prompt << "=== 几何图构建要求 ===\n";
		//prompt << "请识别所有几何元素及其关系，包括但不限于：\n";
		prompt << "Please identify all geometric elements and their relationships, including but not limited to:\n";
		prompt << "- Any geometric shapes (parabola, circle, ellipse, line, etc.)\n";
		prompt << "- Any geometric objects (points, focus, tangent points, intersection points, etc.)\n";
		prompt << "- Any geometric relations (on..., tangent, intersecting, parallel, perpendicular, etc.)\n";
		prompt << "- Any mathematical concepts (theorems, formulas, equations, distances, etc.)\n\n";

		return prompt.str();
	}

	// 几何图输出格式，Important 几何图不解析输出列表里的title
	std::string getGeoGraphFormat() {
		std::stringstream prompt;

		prompt << "### OutputFormat\n";
		//prompt << "请以自然语言描述你的几何图构建结果，格式如下：\n\n";
		prompt << "Describe the result of the construction of the geometric structure in natural language in the following format:\n\n";
		prompt << "Geometry Elements List:\n";
		//prompt << "   对于每个几何元素，请说明：\n";
		prompt << "   For each geometry element, specify:\n";
		//prompt << "   - 元素名称（用于标识）\n";
		prompt << "   - geometry element name (Used for identification)\n";
		//prompt << "   - 几何类型\n";
		//prompt << "   - 描述和性质\n\n";
		prompt << "Geometry Relations List:\n";
		//prompt << "   对于每个关系，请说明：\n";
		prompt << "   For each relationship, please specify:\n";
		//prompt << "   - 关系名称（用于标识）\n";
		prompt << "   - relation name (Used for identification)\n";
		//prompt << "   - 起始元素\n";
		prompt << "   - start element\n";
		//prompt << "   - 目标元素\n";
		prompt << "   - target element\n";
		//prompt << "   - 关系描述\n\n";

		return prompt.str();
	}

	std::string getQuestion(IterationState& state, std::string newContent);
};
