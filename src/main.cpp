#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

#include "IActivityWatcher.hpp"
#include "IScreenshot.hpp"

#include "Utilities.hpp"
#include "ConfigManager.hpp"
#include "KWinScreenshot.hpp"
#include "MutterScreenshot.hpp"
#include "ScreenshotService.hpp"
#include "TesseractTextParser.hpp"
#include "Utilities.hpp"
#include "X11Screenshot.hpp"
#include "YandexGPT.hpp"

namespace fs = std::filesystem;

std::set<std::string> split_into_words(const std::string& text) {
    std::set<std::string> words;
    std::string word;
    std::stringstream ss(text);
    while (ss >> word) {
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        if (word.length() > 2) {
            words.insert(word);
        }
    }
    return words;
}

double calculate_similarity(const std::string& text1, const std::string& text2) {
    if (text1.empty() || text2.empty()) {
        return 0.0;
    }
    
    auto words1 = split_into_words(text1);
    auto words2 = split_into_words(text2);
    
    if (words1.empty() && words2.empty()) { 
        return 1.0;
    }

    std::vector<std::string> intersection;
    std::set_intersection(words1.begin(), words1.end(),
                            words2.begin(), words2.end(),
                            std::back_inserter(intersection));

    std::vector<std::string> total_union;
    std::set_union(words1.begin(), words1.end(),
                    words2.begin(), words2.end(),
                    std::back_inserter(total_union));

    return static_cast<double>(intersection.size()) / total_union.size();
}

bool is_text_unique(const std::string& text, double threshold = 0.85) {
    if (text.empty()) {
        return false;
    }

    static std::string lastText;
    double similarity = calculate_similarity(text, lastText);
    if (similarity >= threshold) {
        return false;
    }

    lastText = text;
    return true;
}

void save_text_to_log(const std::string& text, const AppConfig& cfg, 
        const std::string& windowId, const std::string& proc_name) {
    if (text.empty()) {
        return;
    }

    if (!fs::exists(cfg.logDir)) {
        fs::create_directories(cfg.logDir);
    }

    std::string full_path = cfg.logDir + "/" + cfg.logFile;
    std::ofstream logFile(full_path, std::ios::app);
    
    if (!logFile.is_open()) {
        std::cerr << "[WorkTracer Error] Failed to open log file: " << full_path << std::endl;
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tBuf;
    localtime_r(&time_t_now, &tBuf);

    logFile << "\n==================================================================\n";
    logFile << "[" << std::put_time(&tBuf, "%Y-%m-%d %H:%M:%S") << "] PROCESS: " << proc_name << "\n";
    logFile << "==================================================================\n";
    logFile << text << "\n";
    
    logFile.flush();
}


void sendToAi(const AppConfig& cfg) {

    std::string logPath = cfg.logDir + "/" + cfg.logFile;
    std::ifstream logFile(logPath);
    std::stringstream buffer;
    buffer << logFile.rdbuf();
    std::string logContent = buffer.str();

    if (!logContent.empty()) {

        std::unique_ptr<IAiEngine> aiService = std::make_unique<YandexGpt>(cfg.yandexFolderId, cfg.yandexApiKey);
        if (const auto aiResponse = aiService->generateSummary(logContent); !aiResponse.empty()) {
            std::cout << "\n==================================================" << std::endl;
            std::cout << "🎯 Yandex GPT AI Answer:" << std::endl;
            std::cout << "==================================================" << std::endl;
            std::cout << aiResponse << std::endl; 
            std::cout << "==================================================" << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {

    std::signal(SIGINT, IActivityWatcher::handleSignal);  // Ctrl+C
    std::signal(SIGTERM, IActivityWatcher::handleSignal); // systemctl stop

    std::string configPath = (argc > 1) ? argv[1] : "config.yaml";
    ConfigManager configMgr;
    if (!configMgr.load(configPath)) { 
        return 1;
    }

    auto engine = createScreenShotEngine();
    if (!engine) {
        std::cerr << "Couldn't detect the shell!" << std::endl;
        return 1;
    }

    const AppConfig& cfg = configMgr.get();
    auto watcher = createActivityWatcher(cfg.intervalSec, engine->getRawDisplay());
    if (!watcher) {
        if (cfg.enableLogging) {
            std::cout << "[WorkTracer] The daemon is initialized. Waiting for activity..." << std::endl;
        }
    }

    ScreenshotService service(std::move(engine), cfg);
    std::unique_ptr<ITextParser> parser = std::make_unique<TesseractTextParser>();

    cv::Mat frame;
    auto lastReportTime = std::chrono::steady_clock::now();

    while (true) {

        WatcherEvent event = watcher->waitNextEvent();
        if (event == WatcherEvent::Interrupted) {
            if (cfg.enableLogging) {
                std::cout << "\n[WorkTracer] Termination signal received. Stopping daemon..." << std::endl;
            }
            sendToAi(cfg);
            break;
        }

        if (event == WatcherEvent::FocusChanged || event == WatcherEvent::Timeout) {
            if (cfg.enableLogging) {
                std::string reason = (event == WatcherEvent::FocusChanged) ? "Focus was changed" : "Timeout";
                std::cout << "[Event] Trigger: " << reason << std::endl;
            }

            if (service.captureWindow(frame)) {
                std::string text = parser->parse(frame);

                if (event == WatcherEvent::FocusChanged || event == WatcherEvent::Timeout && is_text_unique(text)) {
                    std::string currentWindowId = service.getCurrentWindowId();
                    std::string currentWindowProcess = service.getCurrentWindowProcessName();
                    save_text_to_log(text, cfg, currentWindowId, currentWindowProcess);
                }
            }
        }

        auto now = std::chrono::steady_clock::now();
        //auto elapsed = std::chrono::duration_cast<std::chrono::minutes>(now - lastReportTime).count();
        //if (elapsed >= cfg.reportIntervalMinutes) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastReportTime).count();
        if (elapsed >= cfg.intervalSec) {
            lastReportTime = now;
            sendToAi(cfg);
        }
    }

    return 0;
}
