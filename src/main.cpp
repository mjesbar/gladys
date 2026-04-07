#include <QApplication>
#include <QScreen>
#include <QRect>
#include <QDebug>

#include "mainwindow.h"



int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Gladys");
    app.setQuitOnLastWindowClosed(false);

    QWidget dummy;
    MainWindow window(&dummy);

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
    qWarning() << "Global hotkeys are not implemented for this platform.";
#else
    qWarning() << "Global hotkeys are not implemented for this platform.";
#endif

    return app.exec();
}
