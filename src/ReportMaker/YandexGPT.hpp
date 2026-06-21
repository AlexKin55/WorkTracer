#pragma once
#include "IAIengine.hpp"

#include <string>

class YandexGpt : public IAiEngine {
private:
    std::string m_folderId;
    std::string m_apiKey;

public:
    YandexGpt(std::string_view folderId, std::string_view apiKey);
    ~YandexGpt() override;

    std::string generateSummary(std::string_view logContent) override;
};
