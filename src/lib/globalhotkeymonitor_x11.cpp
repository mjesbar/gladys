#include <QDebug>
#include <QSocketNotifier>
#include "globalhotkeymonitor_x11.h"

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#include <X11/keysym.h>

// Handle Xlib/Qt conflicts
#ifdef Status
#undef Status
#endif

GlobalHotkeyMonitorX11::GlobalHotkeyMonitorX11(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_notifier(nullptr)
{
    Display *display = XOpenDisplay(NULL);
    m_display = static_cast<void*>(display);
    if (display) {
        KeyCode keycode = XKeysymToKeycode(display, XK_I);
        unsigned int modifiers = ControlMask;
        Window root = DefaultRootWindow(display);

        XGrabKey(display, keycode, modifiers, root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, keycode, modifiers | Mod2Mask, root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, keycode, modifiers | LockMask, root, True, GrabModeAsync, GrabModeAsync);
        XGrabKey(display, keycode, modifiers | Mod2Mask | LockMask, root, True, GrabModeAsync, GrabModeAsync);

        int fd = ConnectionNumber(display);
        m_notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
        connect(m_notifier, &QSocketNotifier::activated, this, &GlobalHotkeyMonitorX11::handleX11Event);
        
        qDebug() << "Global hotkey Ctrl+I grabbed.";
    } else {
        qWarning() << "Failed to open X display.";
    }
}

GlobalHotkeyMonitorX11::~GlobalHotkeyMonitorX11() {
    if (m_display) {
        XCloseDisplay(static_cast<Display*>(m_display));
    }
}

void GlobalHotkeyMonitorX11::handleX11Event() {
    Display *display = static_cast<Display*>(m_display);
    if (!display) return;

    XEvent event;
    while (XPending(display)) {
        XNextEvent(display, &event);
        if (event.type == KeyPress) {
            emit hotkeyPressed();
        }
    }
}
#else
GlobalHotkeyMonitorX11::GlobalHotkeyMonitorX11(QObject *parent) : QObject(parent) {}
GlobalHotkeyMonitorX11::~GlobalHotkeyMonitorX11() {}
void GlobalHotkeyMonitorX11::handleX11Event() {}
#endif
