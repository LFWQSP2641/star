#ifndef LAUNCHINGBALL_H
#define LAUNCHINGBALL_H

#include <QFuture>
#include <QPainter>
#include <QQmlEngine>
#include <QQuickPaintedItem>
#include <QtQml/qqmlregistration.h>

class ThreeBodyMotion : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit ThreeBodyMotion(QQuickItem *parent = nullptr);

public slots:
    void initialize();

protected:
    void paint(QPainter *painter) override;
    void timerEvent(QTimerEvent *event) override;

protected:
    bool m_initialized = false;
    int m_timerId = 0;

    struct Body
    {
        QPointF position;
        QPointF velocity;
        double radius, mass;
        QColor color;
        QList<QPointF> trail;
    };
    QList<Body> m_bodies;
    QList<Body> m_bodiesNext;

    int m_bodyCount = 3;
    QList<QColor> m_colors;
    QList<double> m_bodyRadius = { 64 };
    QList<double> m_bodyMass = { 64 };
    QList<QPointF> m_launchPoints;
    QList<QPointF> m_launchVelocities;
    int m_trailLength = 32;
    double m_G = 100;
    int m_timerIntervalMs = 20;
    double m_maxVelocity = 0.01;
    double m_maxReserVelocity = 2;
    double m_maxDistance = 200;
    double m_velocityMultiplier = 4;

    void handleBodies(int begin, int end);
    QList<QFuture<void>> m_futures;

    void updateBodies();
    void runHandleBodies(QList<QFuture<void>> &futures);
};

#endif // LAUNCHINGBALL_H
