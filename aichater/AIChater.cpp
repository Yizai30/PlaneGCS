#include "AIChater.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <locale>
#include <codecvt>
#include <chrono>
#include <ctime>
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include <boost/locale/encoding_utf.hpp>
#include "../examples/llm_geometry_animation.h"  // 包含GeometryGraph完整定义

// 修复的UTF-16到UTF-8转换函数
static std::string wstringToUTF8(const std::wstring& wstr) {
    std::string result;
    for (size_t i = 0; i < wstr.length(); ) {
        wchar_t wc = wstr[i];

        // 检查是否是UTF-16代理对的高位
        if (wc >= 0xD800 && wc <= 0xDBFF) {
            // 高位代理
            if (i + 1 < wstr.length()) {
                wchar_t lowSurrogate = wstr[i+1];
                if (lowSurrogate >= 0xDC00 && lowSurrogate <= 0xDFFF) {
                    // 有效的代理对
                    uint32_t codePoint = 0x10000 + ((wc & 0x3FF) << 10) + (lowSurrogate & 0x3FF);

                    // 4字节UTF-8
                    result += static_cast<char>(0xF0 | (codePoint >> 18));
                    result += static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
                    result += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
                    result += static_cast<char>(0x80 | (codePoint & 0x3F));

                    i += 2;
                    continue;
                }
            }
            // 无效的高位代理，跳过
            i++;
        } else if (wc >= 0xDC00 && wc <= 0xDFFF) {
            // 单独的低位代理，跳过
            i++;
        } else if (wc < 0x80) {
            // ASCII字符 (1字节)
            result += static_cast<char>(wc);
            i++;
        } else if (wc < 0x800) {
            // 2字节UTF-8
            result += static_cast<char>(0xC0 | (wc >> 6));
            result += static_cast<char>(0x80 | (wc & 0x3F));
            i++;
        } else {
            // 3字节UTF-8 (支持大部分中文字符)
            result += static_cast<char>(0xE0 | (wc >> 12));
            result += static_cast<char>(0x80 | ((wc >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (wc & 0x3F));
            i++;
        }
    }
    return result;
}

// 简化的UTF-8到宽字符串转换 - 修复边界检查错误
static std::wstring utf8ToWstring(const std::string& str) {
    std::wstring result;
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = str[i];
        if (c < 0x80) {
            // ASCII字符
            result += static_cast<wchar_t>(c);
            i++;
        } else if ((c & 0xE0) == 0xC0) {
            // 2字节UTF-8
            if (i + 1 < str.length()) {
                unsigned char c2 = str[i+1];
                if ((c2 & 0xC0) == 0x80) {
                    wchar_t wc = ((c & 0x1F) << 6) | (c2 & 0x3F);
                    result += wc;
                } else {
                    result += L'?';
                }
            } else {
                result += L'?';
                break;
            }
            i += 2;
        } else if ((c & 0xF0) == 0xE0) {
            // 3字节UTF-8 (中文)
            if (i + 2 < str.length()) {
                unsigned char c2 = str[i+1];
                unsigned char c3 = str[i+2];
                if (((c2 & 0xC0) == 0x80) && ((c3 & 0xC0) == 0x80)) {
                    uint32_t codePoint = ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
                    if (codePoint <= 0xFFFF) {
                        result += static_cast<wchar_t>(codePoint);
                    } else {
                        result += L'?';
                    }
                } else {
                    result += L'?';
                }
            } else {
                result += L'?';
                break;
            }
            i += 3;
        } else if ((c & 0xF8) == 0xF0) {
            // 4字节UTF-8 (emoji)
            if (i + 3 < str.length()) {
                unsigned char c2 = str[i+1];
                unsigned char c3 = str[i+2];
                unsigned char c4 = str[i+3];
                if (((c2 & 0xC0) == 0x80) && ((c3 & 0xC0) == 0x80) && ((c4 & 0xC0) == 0x80)) {
                    uint32_t codePoint = ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
                    if (codePoint <= 0x10FFFF && codePoint > 0xFFFF) {
                        // UTF-16代理对
                        codePoint -= 0x10000;
                        wchar_t highSurrogate = static_cast<wchar_t>(0xD800 + ((codePoint >> 10) & 0x3FF));
                        wchar_t lowSurrogate = static_cast<wchar_t>(0xDC00 + (codePoint & 0x3FF));
                        result += highSurrogate;
                        result += lowSurrogate;
                    } else if (codePoint <= 0xFFFF) {
                        result += static_cast<wchar_t>(codePoint);
                    } else {
                        result += L'?';
                    }
                } else {
                    result += L'?';
                }
            } else {
                result += L'?';
                break;
            }
            i += 4;
        } else {
            // 无效UTF-8
            result += L'?';
            i++;
        }
    }
    return result;
}

std::string AIChater::callDeepseekChat(std::string promptString) {
    std::string ret;
    static int roundCounter = 1;

    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = *std::localtime(&time_t);
    char timestamp[64];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &tm);

    // Create round-specific log file
    std::string roundLogFile = "llm_round_" + std::to_string(roundCounter) + "_" + timestamp + ".txt";
    std::ofstream roundFile(roundLogFile, std::ios::binary);

    if (roundFile.is_open()) {
        roundFile << "=== Round " << roundCounter << " - " << timestamp << " ===\n\n";
        roundFile << "INPUT:\n" << promptString << "\n\n";
        roundFile.flush();
    }

    std::cout << "Sending prompt: " << promptString << std::endl;

    // 设置API Key和API URL
    const utility::string_t apiKey = U("sk-6ec2c1e1466642bc817ee99f06cdd77c"); // 替换为你的API Key
    const utility::string_t apiUrl = U("https://api.deepseek.com/chat/completions");

    // 创建HTTP客户端配置并设置超时
    web::http::client::http_client_config config;
    config.set_timeout(utility::seconds(60)); // 设置(单位秒)超时

    // 禁用SSL证书验证以解决SSL错误
    config.set_validate_certificates(false);

    // 使用配置创建HTTP客户端
    web::http::client::http_client client(apiUrl, config);

    // 构建请求体 - 使用改进的UTF-8处理
    web::json::value request;
    request[U("model")] = web::json::value(U("deepseek-chat")); // 使用DeepSeek模型
    web::json::value messages = web::json::value::array();
    web::json::value userMessage;
    userMessage[U("role")] = web::json::value(U("user"));

    // 简单的ASCII转换，避免UTF-16代理对问题
    std::wstring widePrompt;
    for (char c : promptString) {
        if ((c & 0x80) == 0) {
            // ASCII字符
            widePrompt += static_cast<wchar_t>(c);
        } else {
            // 非ASCII字符替换为?
            widePrompt += L'?';
        }
    }

    userMessage[U("content")] = web::json::value(widePrompt);
    messages[0] = userMessage;
    request[U("messages")] = messages;
    request[U("temperature")] = web::json::value(0.0);

    // 创建HTTP请求
    web::http::http_request req(web::http::methods::POST);
    req.headers().add(U("Authorization"), U("Bearer ") + apiKey);
    req.headers().add(U("Content-Type"), U("application/json; charset=utf-8"));
    req.set_body(request);

    // 发送请求并处理响应
    client.request(req)
        .then([=](web::http::http_response response) {
        return response.extract_json();
            })
        .then([&](web::json::value jsonResponse) {
        // Write raw JSON response to round file
        if (roundFile.is_open()) {
            std::string rawJson = utility::conversions::to_utf8string(jsonResponse.serialize());
            roundFile << "RAW_JSON:\n" << rawJson << "\n\n";
        }

        if (jsonResponse.has_field(U("choices")))
        {
            auto choices = jsonResponse[U("choices")].as_array();
            if (choices.size() > 0)
            {
                auto content = choices[0][U("message")][U("content")].as_string();

                // 简单的ASCII转换，避免UTF-16代理对问题
                std::string utf8_result;
                for (wchar_t wc : content) {
                    if (wc < 128) {
                        utf8_result += static_cast<char>(wc);
                    } else {
                        utf8_result += '?';
                    }
                }

                ret = utf8_result;

                std::cout << "LLM response length: " << ret.length() << std::endl;
                std::cout << "LLM response content: " << ret << std::endl;

                // Write response to round-specific log file
                if (roundFile.is_open()) {
                    roundFile << "OUTPUT:\n" << ret << "\n\n";
                    roundFile << "=== End of Round " << roundCounter << " ===\n";
                    roundFile.close();
                    std::cout << "Round " << roundCounter << " log written to " << roundLogFile << std::endl;
                }

                roundCounter++;
            } else {
                // Handle case where no choices are available
                if (roundFile.is_open()) {
                    roundFile << "OUTPUT:\n[ERROR: No choices in LLM response]\n\n";
                    roundFile << "=== End of Round " << roundCounter << " ===\n";
                    roundFile.close();
                }
                roundCounter++;
            }
        } else {
            // Handle case where no choices field in response
            if (roundFile.is_open()) {
                roundFile << "OUTPUT:\n[ERROR: No choices field in LLM response]\n\n";
                roundFile << "=== End of Round " << roundCounter << " ===\n";
                roundFile.close();
            }
            roundCounter++;
        }
            })
        .wait();

    return ret;
}

// 加载JSON文件到字符串
bool AIChater::loadJsonFileToString(const std::string& filename, std::string& content) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        // 如果文件不存在，使用默认的几何图模板
        content = R"({
  "geometry_templates": {
    "point": {"type": "point", "properties": ["x", "y"]},
    "line": {"type": "line", "properties": ["start", "end"]},
    "circle": {"type": "circle", "properties": ["center", "radius"]},
    "parabola": {"type": "parabola", "properties": ["focus", "directrix"]},
    "focus": {"type": "focus", "properties": ["x", "y"]}
  },
  "relation_templates": {
    "point_on_circle": {"type": "spatial", "description": "点在圆上"},
    "tangent": {"type": "geometric", "description": "相切"},
    "perpendicular": {"type": "geometric", "description": "垂直"},
    "parallel": {"type": "geometric", "description": "平行"},
    "distance": {"type": "metric", "description": "距离约束"}
  }
})";
        std::cout << "Warning: Template file " << filename << " not found, using default geometry graph template" << std::endl;
        return false;
    }

    try {
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cout << "Failed to read file: " << e.what() << std::endl;
        content = "{}";
        return false;
    }
}

// 实现 getQuestion 函数
std::string AIChater::getQuestion(IterationState& state, std::string newContent) {
    std::stringstream prompt;

    prompt << "**NewContent**:\n";
    prompt << newContent << "\n";

    prompt << "**CurGraph**:\n";
    if (state.geometryGraph) {
        prompt << state.geometryGraph->toString() << "\n";
    } else {
        prompt << "(GeoGraph is Empty)\n";
    }

    prompt << "**NewGraph**:\n";

    return prompt.str();
}
