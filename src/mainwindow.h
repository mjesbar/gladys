#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QApplication>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QSize>
#include <QShortcut>

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    void toggleVisibility();
    void handleOpacityAnimationFinished();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPropertyAnimation *m_positionAnimation;
    QPropertyAnimation *m_opacityAnimation;
    int m_targetYVisible;
    int m_targetYSubtle;
    bool m_isProminent;
    QSize m_originalSize;

    QShortcut *m_shortcut; // Reverted to QShortcut instance
};

#endif // MAINWINDOW_H