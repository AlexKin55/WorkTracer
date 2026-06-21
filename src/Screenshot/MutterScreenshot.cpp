#include "MutterScreenshot.hpp"

#include <QDateTime>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDir>
#include <QFile>

MutterScreenshot::MutterScreenshot() {}
MutterScreenshot::~MutterScreenshot() {}

bool MutterScreenshot::captureToMemory(cv::Mat& image) {

    QString tempPath = "/dev/shm/gnome_win_scr_" + QString::number(QDateTime::currentMSecsSinceEpoch()) + ".png";
    QDBusInterface mutter(
        "org.gnome.Shell", 
        "/org/gnome/Shell/Screenshot", 
        "org.gnome.Shell.Screenshot"
    );
    
    QDBusMessage reply = mutter.call("ScreenshotWindow", true, false, false, tempPath);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        return false;
    }

    if (reply.type() == QDBusMessage::ErrorMessage || 
        !reply.arguments().at(0).toBool() || 
        !QFile::exists(tempPath)) 
    {
        return false;
    }

    return !image.empty();
}

std::string MutterScreenshot::getActiveWindowId() { 
    return "Wayland_Mutter_Active"; 
}

std::string MutterScreenshot::getActiveWindowProcessName() { 
    return "Unknown_Mutter_Process"; 
}
