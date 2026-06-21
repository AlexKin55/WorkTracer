#include "MutterActivityWatcher.hpp"

#include <QDBusInterface>
#include <QDBusMessage>
#include <QTimer>

namespace {
    std::string QueryMutterActiveWindow() {
    QDBusInterface eshell(
        "org.gnome.Shell",
        "/org/gnome/Shell",
        "org.gnome.Shell"
    );

    QDBusMessage reply = eshell.call("Eval", "global.display.focus_window ? global.display.focus_window.get_title() : ''");
    if (reply.arguments().count() > 1) {
        return reply.arguments().at(1).toString().toStdString();
    }
    return "";
}
}

MutterActivityWatcher::MutterActivityWatcher(int timeoutSec) 
    : m_intervalSec(timeoutSec), m_pendingEvent(WatcherEvent::Interrupted) {
    m_lastTriggerTime = std::chrono::steady_clock::now();
    m_lastWindowTitle = QueryMutterActiveWindow();
}

void MutterActivityWatcher::pollMutterFocus() {
    std::string currentWindow = QueryMutterActiveWindow();
    
    if (!currentWindow.empty() && currentWindow != m_lastWindowTitle) {
        m_lastWindowTitle = currentWindow;
        m_lastTriggerTime = std::chrono::steady_clock::now();
        m_pendingEvent = WatcherEvent::FocusChanged;
        m_loop.quit();
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastTriggerTime).count();
    
    if (elapsed >= m_intervalSec) {
        m_lastTriggerTime = now;
        m_pendingEvent = WatcherEvent::Timeout;
        m_loop.quit();
    }
}

WatcherEvent MutterActivityWatcher::waitNextEvent() {
    while (!m_shouldStop) {
        m_pendingEvent = WatcherEvent::Interrupted;

        QTimer poolTimer;
        connect(&poolTimer, &QTimer::timeout, this, &MutterActivityWatcher::pollMutterFocus);
        poolTimer.start(250);

        m_loop.exec();

        if (m_pendingEvent != WatcherEvent::Interrupted) {
            return m_pendingEvent;
        }
    }
    return WatcherEvent::Interrupted;
}
