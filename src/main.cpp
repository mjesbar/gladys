#include <QApplication>
#include <QScreen>
#include <QRect>
#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>
#include <QWidget>

#include "mainwindow.h"
#include "globalhotkeymonitor_x11.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Gladys");
    app.setQuitOnLastWindowClosed(false);

    QString serverName = "gladys-ipc-server";

    // Try to connect to existing instance
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (socket.waitForConnected(200)) {
        socket.write("toggle");
        socket.waitForBytesWritten(1000);
        socket.disconnectFromServer();
        return 0;
    }

    // No existing instance, start server
    QLocalServer::removeServer(serverName);
    QLocalServer server;
    if (!server.listen(serverName)) {
        qWarning() << "Could not start local server:" << server.errorString();
    }

    QWidget dummy;
    MainWindow window(&dummy);

    QObject::connect(&server, &QLocalServer::newConnection, [&]() {
        QLocalSocket *clientConnection = server.nextPendingConnection();
        QObject::connect(clientConnection, &QLocalSocket::readyRead, [clientConnection, &window]() {
            QByteArray data = clientConnection->readAll();
            if (data == "toggle") {
                window.toggleVisibility();
            }
            clientConnection->disconnectFromServer();
        });
        QObject::connect(clientConnection, &QLocalSocket::disconnected, clientConnection, &QLocalSocket::deleteLater);
    });

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
#else
    qWarning() << "Global hotkeys are not implemented for this platform.";
#endif

    return app.exec();
}
