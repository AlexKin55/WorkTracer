#include "KWinScreenshot.hpp"
#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>
#include <QDateTime>
#include <QFile>

KWinScreenshot::KWinScreenshot() {}
KWinScreenshot::~KWinScreenshot() {}

bool KWinScreenshot::captureToMemory(cv::Mat& image) {

    QString tempPath = "/dev/shm/gnome_win_scr_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".png";
    QDBusInterface kwin(
        "org.kde.KWin",
        "/Screenshot",
        "org.kde.KWin.Screenshot"
    );

    int mask = 1; 
    QDBusReply<QString> reply = kwin.call("screenshotWindow", tempPath, mask);

    if (!reply.isValid() || reply.value().isEmpty()) {
        reply = kwin.call("screenshotWindow", tempPath);
    }

    if (!reply.isValid() || reply.value().isEmpty()) {
        return false;
    }

    image = cv::imread(tempPath.toStdString(), cv::IMREAD_UNCHANGED);
    QFile::remove(tempPath);

    return !image.empty();
}

std::string KWinScreenshot::getActiveWindowId() { 
    return "Wayland_KWin_Active"; 
}

std::string KWinScreenshot::getActiveWindowProcessName() { 
    return "Unknown_KWin_Process"; 
}
