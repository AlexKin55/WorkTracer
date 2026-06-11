#pragma once
#include <string>
#include <chrono>
#include <X11/Xlib.h>

enum class WatcherEvent {
    Timeout,
    FocusChanged,
    Interrupted
};

class ActivityWatcher {
private:
    Display* display;
    Window root_window;
    int interval_seconds;
    std::string last_window_id;
    std::chrono::steady_clock::time_point last_trigger_time;

    static volatile bool should_stop;

    std::string query_active_window_id();
public:

    ActivityWatcher(Display* x11_display, int timeout_sec);
    WatcherEvent wait_next_event();

    static void handle_signal(int signal) {
        should_stop = true;
    }
};
