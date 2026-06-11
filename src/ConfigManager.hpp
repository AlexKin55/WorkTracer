#pragma once

#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

struct AppConfig {
    int interval_seconds = 20;
    std::string output_dir = "./worktracer_data";
    std::string log_file_name = "daily_accumulated.log";
    bool enable_logging = true;
};

class ConfigManager {
private:
    AppConfig cfg;

public:
    bool load(std::string_view config_path) {
        try {
            if (!fs::exists(config_path)) {
                std::cout << "[WorkTracer Config] Файл не найден. Создан дефолтный конфиг." << std::endl;
                save_default(std::string(config_path));
                return true;
            }

            YAML::Node node = YAML::LoadFile(std::string(config_path));
            if (node["interval_seconds"]) {
                cfg.interval_seconds = node["interval_seconds"].as<int>();
            }
            if (node["output_dir"]) {
                cfg.output_dir = node["output_dir"].as<std::string>();
            }
            if (node["log_file_name"]) {
                cfg.log_file_name = node["log_file_name"].as<std::string>();
            }
            if (node["enable_logging"]) {
                cfg.enable_logging = node["enable_logging"].as<bool>();
            }
            if (!fs::exists(cfg.output_dir)) {
                fs::create_directories(cfg.output_dir);
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
    void save_default(const std::string& config_path) {
        std::ofstream fout(config_path);
        fout << "interval_seconds: 20\n";
        fout << "output_dir: \"./worktracer_data\"\n";
        fout << "log_file_name: \"daily_accumulated.log\"\n";
        fout << "enable_logging: true\n";
        fout.close();
        fs::create_directories(cfg.output_dir);
    }
};
