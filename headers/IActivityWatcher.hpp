#pragma once
#include <string>

enum class WatcherEvent {
    Timeout,
    FocusChanged,
    Interrupted
};

class IActivityWatcher {
protected:
    inline static volatile bool m_shouldStop = false;

public:
    virtual ~IActivityWatcher() = default;
    virtual WatcherEvent waitNextEvent() = 0;
    
    static void handleSignal(int) {
        m_shouldStop = true;
    }
};

