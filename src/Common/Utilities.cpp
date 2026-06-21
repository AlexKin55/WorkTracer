
#include "Utilities.hpp"

#include "X11ActivityWatcher.hpp"
#include "KWinActivityWatcher.hpp"
#include "MutterActivityWatcher.hpp"


#include "KWinScreenshot.hpp"
#include "MutterScreenshot.hpp"
#include "X11Screenshot.hpp"

#include <cstdlib>
#include <algorithm>

std::unique_ptr<IActivityWatcher> createActivityWatcher(int timeoutSec, Display* x11Display) {
    const char* session_env = std::getenv("XDG_SESSION_TYPE");
    std::string session = session_env ? session_env : "";
    std::transform(session.begin(), session.end(), session.begin(), ::tolower);

    if (session == "x11" && x11Display) {
        return std::make_unique<X11ActivityWatcher>(x11Display, timeoutSec);
    } 
    
    if (session == "wayland") {
        const char* desktopEnv = std::getenv("XDG_CURRENT_DESKTOP");
        std::string desktop = desktopEnv ? desktopEnv : "";
        std::transform(desktop.begin(), desktop.end(), desktop.begin(), ::toupper);

        if (desktop.find("KDE") != std::string::npos) {
            return std::make_unique<KWinActivityWatcher>(timeoutSec);
        }
        if (desktop.find("GNOME") != std::string::npos) {
            return std::make_unique<MutterActivityWatcher>(timeoutSec);
        }
    }

    return nullptr;
}

std::unique_ptr<IScreenshot> createScreenShotEngine() {

    const char* sessionTypeEnv = std::getenv("XDG_SESSION_TYPE");
    std::string sessionType = sessionTypeEnv ? sessionTypeEnv : "";
    std::transform(sessionType.begin(), sessionType.end(), sessionType.begin(), ::tolower);

    if (sessionType == "x11") {
        return std::make_unique<X11Screenshot>();
    }
    
    if (sessionType == "wayland") {
        const char* desktopEnv = std::getenv("XDG_CURRENT_DESKTOP");
        std::string desktop = desktopEnv ? desktopEnv : "";
        
        std::transform(desktop.begin(), desktop.end(), desktop.begin(), ::toupper);

        if (desktop.find("KDE") != std::string::npos) {
            return std::make_unique<KWinScreenshot>();
        } 
        
        if (desktop.find("GNOME") != std::string::npos) {
            return std::make_unique<MutterScreenshot>();
        }
    }

    return nullptr; 
}
