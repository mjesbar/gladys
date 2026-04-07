#include "globalhotkeymonitor_x11.h"
#include <QGuiApplication>
#include <QNativeInterface/QX11Application>
#include <QDebug>
#include <QX11Info>

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <xcb/xcb.h>
#endif

GlobalHotkeyMonitorX11::GlobalHotkeyMonitorX11(QObject *parent)
    : QObject(parent)
{
#ifdef Q_OS_LINUX
    Display *display = QX11Info::display();
    if (display) {
        unsigned int modifiers = ControlMask;
        KeyCode keycode = XKeysymToKeycode(display, XK_I);

        if (keycode != 0) {
            XGrabKey(display,
                     keycode,
                     modifiers,
                     DefaultRootWindow(display),
                     True, // owner_events
                     GrabModeAsync, // pointer_mode
                     GrabModeAsync // keyboard_mode
            );
            qDebug() << "Global hotkey Ctrl+I grabbed.";
        } else {
            qWarning() << "Failed to get keycode for 'I'.";
        }
    } else {
        qWarning() << "Could not get X11 display.";
    }
#else
    qDebug() << "Global hotkey not supported on this OS.";
#endif
}

GlobalHotkeyMonitorX11::~GlobalHotkeyMonitorX11() {
#ifdef Q_OS_LINUX
    Display *display = QX11Info::display();
    if (display) {
        unsigned int modifiers = ControlMask;
        KeyCode keycode = XKeysymToKeycode(display, XK_I);

        if (keycode != 0) {
            XUngrabKey(display, keycode, modifiers, DefaultRootWindow(display));
            qDebug() << "Global hotkey Ctrl+I released.";
        }
    }
#endif
}

bool GlobalHotkeyMonitorX11::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) {
#ifdef Q_OS_LINUX
    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t *event = static_cast<xcb_generic_event_t *>(message);
        if (event->response_type == XCB_KEY_PRESS) {
            xcb_key_press_event_t *kp = static_cast<xcb_key_press_event_t *>(message);
            if (kp->detail == XKeysymToKeycode(QX11Info::display(), XK_I) && (kp->state & ControlMask)) {
                emit hotkeyPressed();
                *result = 1; // Event handled
                return true;
            }
        }
    }
#endif
    return QObject::nativeEventFilter(eventType, message, result);
}
