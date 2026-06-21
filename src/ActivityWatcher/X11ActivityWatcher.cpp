#include "X11ActivityWatcher.hpp"
#include <X11/Xatom.h>
#include <thread>

X11ActivityWatcher::X11ActivityWatcher(Display* x11Display, int timeoutSec)
    : m_display(x11Display), m_intervalSec(timeoutSec) {
    if (m_display) {
        m_rootWindow = DefaultRootWindow(m_display);
        XSelectInput(m_display, m_rootWindow, PropertyChangeMask);
        m_lastWindowId = queryActiveWindowId();
    }
    m_lastTriggerTime = std::chrono::steady_clock::now();
}

std::string X11ActivityWatcher::queryActiveWindowId() {
    if (!m_display) {
        return "0";
    }

    Atom actualType;
    int actualFormat;
    unsigned long nitems, bytesAfter;
    unsigned char* prop = nullptr;
    
    Atom activeAtom = XInternAtom(m_display, "_NET_ACTIVE_WINDOW", True);
    if (XGetWindowProperty(m_display, m_rootWindow, activeAtom, 0, 1, False, XA_WINDOW,
                            &actualType, &actualFormat, &nitems, &bytesAfter, &prop) != Success || !prop) {
        return "0";
    }
    Window activeWinow = *(Window*)prop;
    XFree(prop);
    return std::to_string(activeWinow);
}

WatcherEvent X11ActivityWatcher::waitNextEvent() {
    while (!m_shouldStop) {
        while (m_display && XPending(m_display) > 0) {
            XEvent event;
            XNextEvent(m_display, &event);
            
            if (event.type == PropertyNotify) {
                Atom activeAtom = XInternAtom(m_display, "_NET_ACTIVE_WINDOW", True);
                if (event.xproperty.atom == activeAtom) {
                    std::string currentWindow = queryActiveWindowId();
                    if (currentWindow != m_lastWindowId && currentWindow != "0") {
                        m_lastWindowId = currentWindow;
                        m_lastTriggerTime = std::chrono::steady_clock::now();
                        return WatcherEvent::FocusChanged;
                    }
                }
            }
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastTriggerTime).count();
        
        if (elapsed >= m_intervalSec) {
            m_lastTriggerTime = now;
            m_lastWindowId = queryActiveWindowId(); 
            return WatcherEvent::Timeout;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return WatcherEvent::Interrupted;
}

