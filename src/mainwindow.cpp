#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QRegion>
#include <QPainter>
#include <QImage>
#include <QPalette>
#include <QPoint>
#include <QKeySequence>
#include <QDebug>
#include <QShortcut>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , m_positionAnimation(new QPropertyAnimation(this, "pos"))
    , m_opacityAnimation(new QPropertyAnimation(this, "windowOpacity"))
    , m_targetYVisible(30)
    , m_targetYSubtle(10)
    , m_isProminent(true)
    , m_originalSize(size())
    , m_shortcut(new QShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_I), this))
{
    setFixedSize(60, 60);
    setWindowTitle("Gladys");
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool); // Added Qt::Tool
    setAttribute(Qt::WA_TranslucentBackground);

    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::transparent);
    setPalette(palette);

    // Setup position animation
    m_positionAnimation->setDuration(500); // 0.5 seconds
    m_positionAnimation->setEasingCurve(QEasingCurve::OutCubic);

    // Setup opacity animation
    m_opacityAnimation->setDuration(500); // 0.5 seconds
    m_opacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_opacityAnimation, &QPropertyAnimation::finished, this, &MainWindow::handleOpacityAnimationFinished);

    // Connect QShortcut signal
    connect(m_shortcut, &QShortcut::activated, this, &MainWindow::toggleVisibility);
}

MainWindow::~MainWindow() {
    delete m_positionAnimation;
    delete m_opacityAnimation;
    delete m_shortcut;
}

void MainWindow::toggleVisibility() {
    if (m_positionAnimation->state() == QPropertyAnimation::Running
        || m_opacityAnimation->state() == QPropertyAnimation::Running) {
        return; // Avoid re-triggering while animating
    }

    QPoint startPos = pos();
    QPoint endPos;

    qreal startOpacity;
    qreal endOpacity;

    if (m_isProminent) {
        // Transition to subtle (fading out and moving up slightly)
        endPos = QPoint(startPos.x(), m_targetYSubtle);
        startOpacity = 1.0;
        endOpacity = 0.0; // Fully transparent
    } else {
        // Transition to prominent (fading in and moving down)
        setFixedSize(m_originalSize); // Restore size before animating
        endPos = QPoint(startPos.x(), m_targetYVisible);
        startOpacity = 0.0;
        endOpacity = 1.0; // Fully opaque
    }

    m_positionAnimation->setStartValue(startPos);
    m_positionAnimation->setEndValue(endPos);
    m_positionAnimation->start();

    m_opacityAnimation->setStartValue(startOpacity);
    m_opacityAnimation->setEndValue(endOpacity);
    m_opacityAnimation->start();

    m_isProminent = !m_isProminent;
}

void MainWindow::handleOpacityAnimationFinished() {
    if (!m_isProminent) {
        // If we just finished fading out to the subtle state, resize to 1x1
        setFixedSize(1, 1);
    }
}

void MainWindow::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Explicitly fill with transparent color to ensure no background
    p.fillRect(rect(), Qt::transparent);

    p.setBrush(Qt::white);
    p.setPen(Qt::NoPen);
    p.drawEllipse(0, 0, 60, 60);

    QImage image("icons/mic-light.png");
    if (!image.isNull()) {
        QPixmap pix = QPixmap::fromImage(image.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        int x = 14;
        int y = 14;
        p.drawPixmap(x, y, pix);
    }
}
