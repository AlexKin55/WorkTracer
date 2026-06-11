#include <thread>

#include <X11/Xatom.h>

#include "ActivityWatcher.hpp"

volatile bool ActivityWatcher::should_stop = false;

std::string ActivityWatcher::query_active_window_id() {
    if (!display) {
        return "0";
    }

    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* prop = nullptr;
    
    Atom active_atom = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
    if (XGetWindowProperty(display, root_window, active_atom, 0, 1, False, XA_WINDOW,
                            &actual_type, &actual_format, &nitems, &bytes_after, &prop) != Success || !prop) {
        return "0";
    }
    Window active_win = *(Window*)prop;
    XFree(prop);

    return std::to_string(active_win);
}

ActivityWatcher::ActivityWatcher(Display* x11_display, int timeout_sec) 
    : display(x11_display), interval_seconds(timeout_sec) {
    
    root_window = DefaultRootWindow(display);
    XSelectInput(display, root_window, PropertyChangeMask);
    
    last_window_id = query_active_window_id();
    last_trigger_time = std::chrono::steady_clock::now();
}

WatcherEvent ActivityWatcher::wait_next_event() {
    while (!should_stop) {
        while (XPending(display) > 0) {
            XEvent event;
            XNextEvent(display, &event);
            
            if (event.type == PropertyNotify) {
                Atom active_atom = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
                if (event.xproperty.atom == active_atom) {
                    std::string current_win = query_active_window_id();
                    
                    if (current_win != last_window_id && current_win != "0") {
                        last_window_id = current_win;
                        last_trigger_time = std::chrono::steady_clock::now();
                        return WatcherEvent::FocusChanged;
                    }
                }
            }
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_trigger_time).count();
        
        if (elapsed >= interval_seconds) {
            last_trigger_time = now;
            last_window_id = query_active_window_id(); 
            return WatcherEvent::Timeout;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return WatcherEvent::Interrupted;
};
