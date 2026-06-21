#pragma once
#include <string>

class IAiEngine {
public:
    virtual ~IAiEngine() = default;
    virtual std::string generateSummary(std::string_view) = 0;
};