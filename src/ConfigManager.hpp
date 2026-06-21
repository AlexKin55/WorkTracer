#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <string_view>
#include <string>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

struct AppConfig {
    int intervalSec = 20;
    int reportIntervalMinutes = 5;
    std::string logDir = "./log_data";
    std::string logFile = "daily_accumulated.log";
    bool enableLogging = true;
    std::unordered_set<std::string> ignoredProcesses;
    std::string yandexFolderId;
    std::string yandexApiKey;
};

class ConfigManager {
private:
    AppConfig cfg;

public:
    bool load(std::string_view configPath) {
        try {
            if (!fs::exists(configPath)) {
                std::cout << "[WorkTracer Config] Couldn't config file. Default config file was created." << std::endl;
                save_default(std::string(configPath));
                return true;
            }

            YAML::Node node = YAML::LoadFile(std::string(configPath));
            if (node["intervalSec"]) {
                cfg.intervalSec = node["intervalSec"].as<int>();
            }
            if (node["reportIntervalMinutes"]) {
                cfg.reportIntervalMinutes = node["reportIntervalMinutes"].as<int>();
            }
            if (node["logDir"]) {
                cfg.logDir = node["logDir"].as<std::string>();
            }
            if (node["logFile"]) {
                cfg.logFile = node["logFile"].as<std::string>();
            }
            if (node["enableLogging"]) {
                cfg.enableLogging = node["enableLogging"].as<bool>();
            }
            if (node["yandexFolderId"]) {
                cfg.yandexFolderId = node["yandexFolderId"].as<std::string>();
            }
            if (node["yandexApiKey"]) {
                cfg.yandexApiKey = node["yandexApiKey"].as<std::string>();
            }          
            if (node["ignoredProcesses"] && node["ignoredProcesses"].IsSequence()) {
                cfg.ignoredProcesses.clear();
                for (const auto& node : node["ignoredProcesses"]) {
                    std::string process = node.as<std::string>();
                    std::transform(process.begin(), process.end(), process.begin(), ::tolower);
                    cfg.ignoredProcesses.insert(process);
                }
            }               
            if (!fs::exists(cfg.logDir)) {
                fs::create_directories(cfg.logDir);
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "[WorkTracer Config Error] " << e.what() << std::endl;
            return false;
        }
    }

    const AppConfig& get() const {
        return cfg;
    }

private:
    void save_default(const std::string& configPath) {
        std::ofstream fout(configPath);
        fout << "intervalSec: 20\n";
        fout << "logDir: \"./worktracer_data\"\n";
        fout << "logFile: \"daily_accumulated.log\"\n";
        fout << "enableLogging: true\n";
        fout.close();
        fs::create_directories(cfg.logDir);
    }
};
