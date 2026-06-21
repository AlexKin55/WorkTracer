#pragma once
#include "IActivityWatcher.hpp"

#include <X11/Xlib.h>
#include <chrono>

class X11ActivityWatcher : public IActivityWatcher {
private:
    Display* m_display;
    Window m_rootWindow;
    int m_intervalSec;
    std::string m_lastWindowId;
    std::chrono::steady_clock::time_point m_lastTriggerTime;

    std::string queryActiveWindowId();

public:
    X11ActivityWatcher(Display* x11Display, int timeoutSec);
    ~X11ActivityWatcher() override = default;
    WatcherEvent waitNextEvent() override;
};
