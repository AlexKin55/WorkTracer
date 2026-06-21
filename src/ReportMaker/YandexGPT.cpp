#include "YandexGPT.hpp"

#include <iostream>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    if (response) {
        response->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }
    return 0;
}

}

YandexGpt::YandexGpt(std::string_view folderId, std::string_view apiKey) 
    : m_folderId(folderId), m_apiKey(apiKey) {
    curl_global_init(CURL_GLOBAL_ALL);
}

YandexGpt::~YandexGpt() {
    curl_global_cleanup();
}

std::string YandexGpt::generateSummary(std::string_view logContent) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return {};
    }

    std::string url = "https://llm.api.cloud.yandex.net/foundationModels/v1/completion";
    std::string system_prompt = "Ты — умный ассистент WorkTracer. Перед тобой текстовый дамп экранов разработчика, собранный за рабочий день. Твоя задача — составить краткое структурированное резюме на русском языке: какие файлы и функции правились (ориентируйся по именам процессов вроде code, chrome), какие темы или документация изучались. Игнорируй интерфейсный мусор. Пиши что файлы правились если содержимое их различается, если не различается то пиши что файлы изучались";

    json json_payload;
    json_payload["modelUri"] = "gpt://" + m_folderId + "/yandexgpt/latest";
    json_payload["completionOptions"]["stream"] = false;
    json_payload["completionOptions"]["temperature"] = 0.3;
    json_payload["completionOptions"]["maxTokens"] = 2000;
    
    json_payload["messages"] = json::array({
        {{"role", "system"}, {"text", system_prompt}},
        {{"role", "user"}, {"text", logContent}}
    });

    std::string request_body = json_payload.dump();

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string auth_header = "Authorization: Api-Key " + m_apiKey;
    headers = curl_slist_append(headers, auth_header.c_str());

    std::string response_data;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "[WorkTracer AI Error] Network error: " << curl_easy_strerror(res) << std::endl;
        return {};
    }

    if (http_code != 200) {
        std::cerr << "[WorkTracer AI Error] Yandex Cloud returned HTTP status: " << http_code << std::endl;
        std::cerr << "[Response Body]: " << response_data << std::endl;
        return {};
    }

    try {
        json parsed_res = json::parse(response_data);
        
        if (parsed_res.contains("result") && 
            parsed_res["result"].contains("alternatives") && 
            !parsed_res["result"]["alternatives"].empty()) {
            
            return parsed_res["result"]["alternatives"][0]["message"]["text"].get<std::string>();
        } else {
            std::cerr << "[WorkTracer AI Error] Unexpected JSON format from Yandex API." << std::endl;
            return {};
        }
    } catch (const json::exception& e) {
        std::cerr << "[WorkTracer AI Error] JSON parsing failed: " << e.what() << std::endl;
        return {};
    }
}
