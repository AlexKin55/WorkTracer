#include "ScreenshotService.hpp"

ScreenshotService::ScreenshotService(std::unique_ptr<IScreenshot> screenshot_engine, bool enable_logging)
    : engine(std::move(screenshot_engine)), verbose(enable_logging) {
}

bool ScreenshotService::capture_window(cv::Mat& out_frame) {
    if (!engine) {
        return false;
    }

    std::string win_id = engine->get_active_window_id_str();
    if (win_id == "0" || win_id.empty()) {
        return false;
    }

    bool success = engine->capture_to_memory(out_frame);
    
    if (success && verbose) {
        std::cout << "[ScreenshotService] Успешно захвачено окно ID: " << win_id 
                    << " (" << out_frame.cols << "x" << out_frame.rows << " px в RAM)" << std::endl;
    }
    return success;
}

std::string ScreenshotService::get_current_window_id() {
    return engine ? engine->get_active_window_id_str() : "0";
}

std::string ScreenshotService::get_current_window_process_name() {
    return engine ? engine->get_active_window_process_name() : "unknown";
}

