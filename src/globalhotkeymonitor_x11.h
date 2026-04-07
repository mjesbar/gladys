#ifndef GLOBALHOTKEYMONITOR_X11_H
#define GLOBALHOTKEYMONITOR_X11_H

#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QDebug>

// Correct Qt6 Native X11 includes
#include <QGuiApplication>
#include <QNativeInterface>
#include <QNativeInterface/QX11Application>
#include <QX11Info>

#ifdef Q_OS_LINUX
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <xcb/xcb.h>
#endif

class GlobalHotkeyMonitorX11 : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT

signals:
    void hotkeyPressed();

public:
    explicit GlobalHotkeyMonitorX11(QObject *parent = nullptr);
    ~GlobalHotkeyMonitorX11() override;

#ifdef Q_OS_LINUX
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
#endif
};

#endif // GLOBALHOTKEYMONITOR_X11_H