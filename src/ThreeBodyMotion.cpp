#include "ThreeBodyMotion.h"

#include <QPainter>
#include <QRandomGenerator>
#include <QtConcurrent>

ThreeBodyMotion::ThreeBodyMotion(QQuickItem *parent)
    : QQuickPaintedItem{ parent }
{
}

void ThreeBodyMotion::initialize()
{
    if (m_initialized)
    {
        m_bodies.clear();
        m_bodiesNext.clear();
        killTimer(m_timerId);
    }
    m_initialized = false;
    m_bodies.reserve(m_bodyCount);
    m_bodiesNext.reserve(m_bodyCount);
    m_futures.clear();
    m_futures.reserve(QThreadPool::globalInstance()->maxThreadCount());
    m_maxDistance = qMin(width(), height()) - 20;
    for (int i(0); i < m_bodyCount; ++i)
    {
        QPointF launchPoint;
        QPointF launchVelocity;
        double radius;
        double mass;
        QColor color;
        if (i < m_launchPoints.size())
        {
            launchPoint = m_launchPoints[i];
        }
        else
        {
            launchPoint = QPointF(QRandomGenerator::global()->bounded(m_maxDistance),
                                  QRandomGenerator::global()->bounded(m_maxDistance));
        }
        if (i < m_launchVelocities.size())
        {
            launchVelocity = m_launchVelocities[i];
        }
        else
        {
            launchVelocity = QPointF(QRandomGenerator::global()->bounded(m_maxVelocity),
                                     QRandomGenerator::global()->bounded(m_maxVelocity));
        }
        if (i < m_bodyRadius.size())
        {
            radius = m_bodyRadius[i];
        }
        else
        {
            radius = m_bodyRadius.last();
        }
        if (i < m_bodyMass.size())
        {
            mass = m_bodyMass[i];
        }
        else
        {
            mass = m_bodyMass.last();
        }
        if (i < m_colors.size())
        {
            color = m_colors[i];
        }
        else
        {
            color = QColor::fromRgb(QRandomGenerator::global()->bounded(256),
                                    QRandomGenerator::global()->bounded(256),
                                    QRandomGenerator::global()->bounded(256));
        }

        m_bodies.push_back(
            { launchPoint,
              launchVelocity,
              radius,
              mass,
              color,
              {} });
    }
    m_bodiesNext = QList<Body>(m_bodies.size());
    m_timerId = startTimer(m_timerIntervalMs);
    m_initialized = true;
}

void ThreeBodyMotion::paint(QPainter *painter)
{
    if (!m_initialized)
    {
        return;
    }
    painter->setRenderHint(QPainter::Antialiasing, true);
    // 绘制三体的拖尾
    for (int i = 0; i < m_trailLength; ++i)
    {
        int k = 0;
        for (int j = 0; j < m_bodies.size(); j++)
        {
            const auto &body = m_bodies[j];
            if (body.trail.size() <= i + 1)
                continue;
            k += 1;
            QLinearGradient gradient(body.trail[i], body.trail[i + 1]);
            QColor beginColor = body.color;
            beginColor.setAlphaF(1.0 - (double)i / m_trailLength);
            QColor endColor = body.color;
            endColor.setAlphaF(1.0 - (double)(i + 1) / m_trailLength);
            gradient.setColorAt(0, beginColor);
            gradient.setColorAt(1, endColor);
            painter->setPen(QPen(gradient, 5));

            painter->drawLine(body.trail[i], body.trail[i + 1]);
        }
        if (k == 0)
            break;
    }

    painter->setPen(Qt::NoPen);

    // 绘制三体
    for (const Body &body : m_bodies)
    {
        painter->setBrush(body.color);
        painter->drawEllipse(body.position, 10, 10);
    }
}

void ThreeBodyMotion::timerEvent([[maybe_unused]] QTimerEvent *event)
{
    updateBodies(); // 更新物体的位置、速度
    update();       // 请求重新绘制
}

void ThreeBodyMotion::handleBodies(int begin, int end)
{
    for (int i(begin); i < end; ++i)
    {
        auto &body = m_bodies[i];
        auto &bodyNext = m_bodiesNext[i];
        bodyNext = body;
        bodyNext.trail.prepend(body.position);
        if (bodyNext.trail.size() > m_trailLength)
        {
            bodyNext.trail.removeLast();
        }

        QPointF acceleration;
        for (int j(0); j < m_bodies.size(); ++j)
        {
            if (j == i)
            {
                continue;
            }
            const auto &other = m_bodies[j];
            const QPointF delta = other.position - body.position;
            const double distance = std::sqrt(delta.x() * delta.x() + delta.y() * delta.y()) + body.radius + other.radius;
            const double force = m_G * body.mass * other.mass / (distance * distance);
            acceleration += force / body.mass * delta / distance;
        }
        bodyNext.velocity += acceleration;

        const double speed = std::sqrt(bodyNext.velocity.x() * bodyNext.velocity.x() + bodyNext.velocity.y() * bodyNext.velocity.y());
        if (speed > m_maxReserVelocity && m_maxReserVelocity > 0)
        {
            bodyNext.velocity *= m_maxReserVelocity / speed;
        }
        bodyNext.position += bodyNext.velocity * m_velocityMultiplier;
        if (bodyNext.position.x() < 10 || bodyNext.position.x() > width() - 10)
        {
            bodyNext.velocity.setX(-bodyNext.velocity.x());
        }
        if (bodyNext.position.y() < 10 || bodyNext.position.y() > height() - 10)
        {
            bodyNext.velocity.setY(-bodyNext.velocity.y());
        }
    }
}

void ThreeBodyMotion::updateBodies()
{
    if (m_futures.isEmpty())
    {
        QList<QFuture<void>> futures;
        futures.reserve(QThreadPool::globalInstance()->maxThreadCount());
        runHandleBodies(futures);
        m_futures = std::move(futures);
    }
    for (auto &future : m_futures)
    {
        future.waitForFinished();
    }
    m_bodies = std::move(m_bodiesNext);
    m_bodiesNext = QList<Body>(m_bodies.size());
    runHandleBodies(m_futures);
}

void ThreeBodyMotion::runHandleBodies(QList<QFuture<void>> &futures)
{
    const int threadCount = QThreadPool::globalInstance()->maxThreadCount();
    const int bodiesPerThread = qCeil(double(m_bodies.size()) / double(threadCount));
    for (int i(0); i < threadCount; ++i)
    {
        const int begin = i * bodiesPerThread;
        const int end = (i + 1) * bodiesPerThread;
        if (begin >= m_bodies.size())
        {
            break;
        }
        if (end > m_bodies.size())
        {
            futures.append(QtConcurrent::run(&ThreeBodyMotion::handleBodies, this, begin, m_bodies.size()));
            break;
        }
        futures.append(QtConcurrent::run(&ThreeBodyMotion::handleBodies, this, begin, end));
    }
}
