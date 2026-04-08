#include <QApplication>
#include <QDebug>
#include <QLocalServer>
#include <QLocalSocket>
#include <QRect>
#include <QScreen>
#include <QWidget>

#include "gladyswindow.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName("Gladys");
  app.setQuitOnLastWindowClosed(false);

  QString serverName = "gladys-ipc-server";

  QLocalServer::removeServer(serverName);
  QLocalServer server;
  if (!server.listen(serverName)) {
    qWarning() << "Gladys: Could not start local server:"
               << server.errorString();
    return 1;
  }

  qDebug() << "Gladys: Server listening on" << serverName;

  QWidget dummy;
  GladysWindow window(&dummy);

  QObject::connect(&server, &QLocalServer::newConnection, [&]() {
    qDebug() << "Gladys: New connection";
    QLocalSocket *clientConnection = server.nextPendingConnection();
    QObject::connect(clientConnection, &QLocalSocket::readyRead,
                     [clientConnection, &window]() {
                       QByteArray data = clientConnection->readAll();
                       qDebug() << "Gladys: Received:" << data;
                       if (data == "toggle") {
                         qDebug() << "Gladys: Toggling visibility";
                         window.toggleVisibility();
                       }
                       clientConnection->disconnectFromServer();
                     });
    QObject::connect(clientConnection, &QLocalSocket::disconnected,
                     clientConnection, &QLocalSocket::deleteLater);
  });

  QScreen *screen = QApplication::primaryScreen();
  if (screen) {
    QRect rect = screen->availableGeometry();
    int x = rect.left() + (rect.width() - window.width()) / 2;
    int y = 10;
    window.setGeometry(x, y, window.width(), window.height());
    window.setWindowOpacity(0.0);
  }

  window.show();
  qDebug() << "Gladys: Window shown";

  return app.exec();
}
