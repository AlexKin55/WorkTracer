#pragma once
#include "IActivityWatcher.hpp"

#include <QObject>
#include <QEventLoop>
#include <chrono>

class MutterActivityWatcher : public QObject, public IActivityWatcher {
    Q_OBJECT
private:
    int m_intervalSec;
    std::string m_lastWindowTitle;
    std::chrono::steady_clock::time_point m_lastTriggerTime;
    QEventLoop m_loop;
    WatcherEvent m_pendingEvent;

public:
    MutterActivityWatcher(int timeoutSec);
    ~MutterActivityWatcher() override = default;
    WatcherEvent waitNextEvent() override;

private slots:
    void pollMutterFocus();
};

