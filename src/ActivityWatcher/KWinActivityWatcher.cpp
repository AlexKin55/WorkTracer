#include "KWinActivityWatcher.hpp"

#include <QDBusConnection>
#include <QTimer>

KWinActivityWatcher::KWinActivityWatcher(int timeoutSec) 
    : m_intervalSec(timeoutSec), m_pendingEvent(WatcherEvent::Interrupted) {
    
    m_lastTriggerTime = std::chrono::steady_clock::now();

    QDBusConnection::sessionBus().connect("org.kde.KWin", "/KWin", "org.kde.KWin", "activeWindowChanged",
        this, SLOT(onActiveWindowChanged(QString))
    );
}

void KWinActivityWatcher::onActiveWindowChanged(const QString& windowId) {
    std::string current_win = windowId.toStdString();
    if (current_win != m_lastWindowId && !current_win.empty()) {
        m_lastWindowId = current_win;
        m_lastTriggerTime = std::chrono::steady_clock::now();
        m_pendingEvent = WatcherEvent::FocusChanged;
        m_loop.quit();
    }
}

void KWinActivityWatcher::onTimeout() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastTriggerTime).count();
    
    if (elapsed >= m_intervalSec) {
        m_lastTriggerTime = now;
        m_pendingEvent = WatcherEvent::Timeout;
        m_loop.quit();
    }
}

WatcherEvent KWinActivityWatcher::waitNextEvent() {
    while (!m_shouldStop) {
        m_pendingEvent = WatcherEvent::Interrupted;

        QTimer checkTimer;
        connect(&checkTimer, &QTimer::timeout, this, &KWinActivityWatcher::onTimeout);
        checkTimer.start(500);

        m_loop.exec();
        if (m_pendingEvent != WatcherEvent::Interrupted) {
            return m_pendingEvent;
        }
    }
    return WatcherEvent::Interrupted;
}

