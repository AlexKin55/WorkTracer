#pragma once
#include "IActivityWatcher.hpp"

#include <QObject>
#include <QEventLoop>
#include <chrono>

class KWinActivityWatcher : public QObject, public IActivityWatcher {
    Q_OBJECT
private:
    int m_intervalSec;
    std::string m_lastWindowId;
    std::chrono::steady_clock::time_point m_lastTriggerTime;
    QEventLoop m_loop;
    WatcherEvent m_pendingEvent;

public:
    KWinActivityWatcher(int timeoutSec);
    ~KWinActivityWatcher() override = default;
    WatcherEvent waitNextEvent() override;

private slots:
    void onActiveWindowChanged(const QString& windowId);
    void onTimeout();
};
