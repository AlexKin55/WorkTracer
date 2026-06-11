#include <iostream>
#include <csignal>
#include <fstream>
#include <iomanip>
#include <filesystem>

#include "X11Screenshot.hpp"
#include "ScreenshotService.hpp"
#include "ConfigManager.hpp"
#include "TesseractTextParser.hpp"
#include "ActivityWatcher.hpp"

namespace fs = std::filesystem;

void save_text_to_log(const std::string& text, const AppConfig& cfg, 
        const std::string& window_id, const std::string& proc_name) {
    if (text.empty()) {
        return;
    }

    if (!fs::exists(cfg.output_dir)) {
        fs::create_directories(cfg.output_dir);
    }

    std::string full_path = cfg.output_dir + "/" + cfg.log_file_name;
    std::ofstream log_file(full_path, std::ios::app);
    
    if (!log_file.is_open()) {
        std::cerr << "[WorkTracer Error] Failed to open log file: " << full_path << std::endl;
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    localtime_r(&time_t_now, &tm_buf);

    log_file << "\n==================================================================\n";
    log_file << "[" << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << "] PROCESS: " << proc_name << "\n";
    log_file << "==================================================================\n";
    log_file << text << "\n";
    
    log_file.flush();
}

int main() {

    std::signal(SIGINT, ActivityWatcher::handle_signal);  // Ctrl+C
    std::signal(SIGTERM, ActivityWatcher::handle_signal); // systemctl stop

    ConfigManager config_mgr;
    if (!config_mgr.load("config.yaml")) { 
        return 1;
    }

    const AppConfig& cfg = config_mgr.get();

    auto x11_engine = std::make_unique<X11Screenshot>();
    Display* display = x11_engine->get_raw_display();
    if (!display) {
        std::cerr << "X11 display is not accessible!" << std::endl;
        return 1;
    }

    ScreenshotService service(std::move(x11_engine), cfg.enable_logging);
    std::unique_ptr<ITextParser> parser = std::make_unique<TesseractTextParser>();

    ActivityWatcher watcher(display, cfg.interval_seconds);

    if (cfg.enable_logging) {
        std::cout << "[WorkTracer] The daemon is initialized. Waiting for activity..." << std::endl;
    }

    cv::Mat frame;

    while (true) {
        WatcherEvent event = watcher.wait_next_event();

        if (event == WatcherEvent::Interrupted) {
            if (cfg.enable_logging) {
                std::cout << "\n[WorkTracer] Termination signal received. Stopping daemon..." << std::endl;
            }
            break;
        }

        if (event == WatcherEvent::FocusChanged || event == WatcherEvent::Timeout) {
            if (cfg.enable_logging) {
                std::string reason = (event == WatcherEvent::FocusChanged) ? "Focus was changed" : "Timeout";
                std::cout << "[Event] Trigger: " << reason << std::endl;
            }

            if (service.capture_window(frame)) {
                std::string text = parser->parse(frame);
                std::string current_win_id = service.get_current_window_id();
                std::string current_win_process = service.get_current_window_process_name();
                save_text_to_log(text, cfg, current_win_id, current_win_process);
            }
        }
    }

    return 0;
}
