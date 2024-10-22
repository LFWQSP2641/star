#ifndef LAUNCHINGBALL_H
#define LAUNCHINGBALL_H

#include <QFuture>
#include <QPainter>
#include <QQmlEngine>
#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>

class LaunchingBall : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit LaunchingBall(QQuickItem *parent = nullptr);

public slots:
    void initialize();

protected:
    void paint(QPainter *painter) override;
    void timerEvent(QTimerEvent *event) override;

protected:
    bool m_initialized = false;
    int m_timerId = 0;
    int m_timerIntervalMs = 20;

    struct Ball
    {
        QPointF position;
        QPointF velocity;
        QColor color;
        int daleyMs;
        QList<QPointF> trail;
    };

    QList<Ball> m_balls;
    QList<Ball> m_ballsNext;

    int m_ballCount = 1000;
    bool m_bounce = true;
    bool m_enableDelay = true;
    int m_maxDelayMs = 8000;
    QList<QColor> m_colors;
    QList<QPointF> m_launchVelocities;
    QList<int> m_launchDelayMs;
    int m_trailLength = 0;

    void handleBalls(int begin, int end);
    QList<QFuture<void>> m_futures;

    void updateBalls();
    void runHandleBodies(QList<QFuture<void>> &futures);
};

#endif // LAUNCHINGBALL_H
