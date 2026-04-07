#ifndef GLOBALHOTKEYMONITOR_X11_H
#define GLOBALHOTKEYMONITOR_X11_H

#include <QObject>
#include <QSocketNotifier>

class GlobalHotkeyMonitorX11 : public QObject {
    Q_OBJECT

public:
    explicit GlobalHotkeyMonitorX11(QObject *parent = nullptr);
    ~GlobalHotkeyMonitorX11() override;

signals:
    void hotkeyPressed();

private slots:
    void handleX11Event();

private:
    void* m_display = nullptr;
    QSocketNotifier* m_notifier = nullptr;
};

#endif // GLOBALHOTKEYMONITOR_X11_H
