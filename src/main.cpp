#include <QApplication>
#include <QScreen>
#include <QRect>
#include <QDebug>
#include <QX11Info>
#include "mainwindow.h"

#ifdef Q_OS_LINUX
#include "globalhotkeymonitor_x11.h"
#endif

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Gladys");

    MainWindow window;

    // Positioning logic here before window.show()
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect rect = screen->availableGeometry();
        int x = rect.left() + (rect.width() - window.width()) / 2;
        int y = 30; // Initial Y position at 30px from top
        window.setGeometry(x, y, window.width(), window.height());
    }

    window.show();

#ifdef Q_OS_LINUX
    GlobalHotkeyMonitorX11 *hotkeyMonitor = new GlobalHotkeyMonitorX11(&app);
    QObject::connect(hotkeyMonitor, &GlobalHotkeyMonitorX11::hotkeyPressed, &window, &MainWindow::toggleVisibility);
    app.installNativeEventFilter(hotkeyMonitor);
#else
    qWarning() << "Global hotkeys are not implemented for this platform.";
#endif

    return app.exec();
}
