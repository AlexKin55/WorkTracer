#include "X11Screenshot.hpp"

#include <fstream>

#include <X11/Xatom.h>
#include <X11/Xutil.h>

namespace {

Window getCurrentWindowId(Display* display) {
    if (!display) {
        return 0;
    }
    Atom actualType;
    int actualFormat;
    unsigned long nitems, bytesAfter;
    unsigned char* prop = nullptr;
    Window root = DefaultRootWindow(display);
    
    Atom activeAtom = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
    if (XGetWindowProperty(display, root, activeAtom, 0, 1, False, XA_WINDOW,
            &actualType, &actualFormat, &nitems, &bytesAfter, &prop) != Success || !prop) {
        return 0;
    }
    Window activeWinow = *(Window*)prop;
    XFree(prop);
    return activeWinow;
}

}


X11Screenshot::X11Screenshot() {
    m_display = XOpenDisplay(nullptr);
}

X11Screenshot::~X11Screenshot() {
    if (m_display) {
        XCloseDisplay(m_display);
    }
}

Display* X11Screenshot::getRawDisplay() {
    return m_display;
}

bool X11Screenshot::captureToMemory(cv::Mat& image) {
    Window winId = getCurrentWindowId(m_display);
    if (winId == 0) {
        return false;
    }

    XWindowAttributes g;
    if (!XGetWindowAttributes(m_display, winId, &g)) {
        return false;
    }

    XImage* xImage = XGetImage(m_display, winId, 0, 0, g.width, g.height, AllPlanes, ZPixmap);
    if (!xImage) {
        return false;
    }

    cv::Mat imageWrapper(g.height, g.width, CV_8UC4, xImage->data, xImage->bytes_per_line);
    imageWrapper.copyTo(image);

    XDestroyImage(xImage);
    return true;
}

std::string X11Screenshot::getActiveWindowProcessName() {
    Window winId = getCurrentWindowId(m_display);
    if (winId == 0) {
        return "unknown";
    }

    Atom actualType;
    int actualFormat;
    unsigned long nitems, bytesAfter;
    unsigned char* prop = nullptr;

    Atom pidAtom = XInternAtom(m_display, "_NET_WM_PID", True);
    if (XGetWindowProperty(m_display, winId, pidAtom, 0, 1, False, XA_CARDINAL,
                           &actualType, &actualFormat, &nitems, &bytesAfter, &prop) != Success || !prop) {
        return "unknown";
    }

    unsigned long pid = *(unsigned long*)prop;
    XFree(prop);

    std::string commPath = "/proc/" + std::to_string(pid) + "/comm";
    std::ifstream commFile(commPath);
    std::string processName = "unknown";
    
    if (commFile.is_open()) {
        std::getline(commFile, processName);
    }

    return processName;
}

std::string X11Screenshot::getActiveWindowId() {
    return std::to_string(getCurrentWindowId(m_display));
}

