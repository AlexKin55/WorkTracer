#include "X11Screenshot.hpp"

#include <fstream>

#include <X11/Xatom.h>
#include <X11/Xutil.h>

Window X11Screenshot::get_active_window_id() {
    if (!display) {
        return 0;
    }
    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* prop = nullptr;
    Window root = DefaultRootWindow(display);
    
    Atom active_atom = XInternAtom(display, "_NET_ACTIVE_WINDOW", True);
    if (XGetWindowProperty(display, root, active_atom, 0, 1, False, XA_WINDOW,
            &actual_type, &actual_format, &nitems, &bytes_after, &prop) != Success || !prop) {
        return 0;
    }
    Window active_win = *(Window*)prop;
    XFree(prop);
    return active_win;
}


X11Screenshot::X11Screenshot() {
    display = XOpenDisplay(nullptr);
}

X11Screenshot::~X11Screenshot() {
    if (display) {
        XCloseDisplay(display);
    }
}

Display* X11Screenshot::get_raw_display() { return display; }

bool X11Screenshot::capture_to_memory(cv::Mat& out_image) {
    Window win_id = get_active_window_id();
    if (win_id == 0) {
        return false;
    }

    XWindowAttributes g;
    if (!XGetWindowAttributes(display, win_id, &g)) {
        return false;
    }

    XImage* x_image = XGetImage(display, win_id, 0, 0, g.width, g.height, AllPlanes, ZPixmap);
    if (!x_image) {
        return false;
    }

    cv::Mat wrapped_img(g.height, g.width, CV_8UC4, x_image->data, x_image->bytes_per_line);
    wrapped_img.copyTo(out_image);

    XDestroyImage(x_image);
    return true;
}

std::string X11Screenshot::get_active_window_process_name() {
    Window win_id = get_active_window_id();
    if (win_id == 0) {
        return "unknown";
    }

    Atom actual_type;
    int actual_format;
    unsigned long nitems, bytes_after;
    unsigned char* prop = nullptr;

    Atom pid_atom = XInternAtom(display, "_NET_WM_PID", True);
    if (XGetWindowProperty(display, win_id, pid_atom, 0, 1, False, XA_CARDINAL,
                           &actual_type, &actual_format, &nitems, &bytes_after, &prop) != Success || !prop) {
        return "unknown";
    }

    unsigned long pid = *(unsigned long*)prop;
    XFree(prop);

    std::string comm_path = "/proc/" + std::to_string(pid) + "/comm";
    std::ifstream comm_file(comm_path);
    std::string process_name = "unknown";
    
    if (comm_file.is_open()) {
        std::getline(comm_file, process_name);
    }

    return process_name;
}

std::string X11Screenshot::get_active_window_id_str() {
    return std::to_string(get_active_window_id());
}

