#include "LaunchingBall.h"

#include <QPainter>
#include <QRandomGenerator>
#include <QtConcurrent>

LaunchingBall::LaunchingBall(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
}

void LaunchingBall::initialize()
{
    if (m_initialized)
    {
        m_balls.clear();
        m_ballsNext.clear();
        killTimer(m_timerId);
    }
    m_initialized = false;
    m_balls.reserve(m_ballCount);
    m_ballsNext.reserve(m_ballCount);
    m_futures.clear();
    m_futures.reserve(QThreadPool::globalInstance()->maxThreadCount());
    for (int i(0); i < m_ballCount; ++i)
    {
        QPointF launchVelocity;
        QColor color;
        int delayMs(0);
        if (i < m_launchVelocities.size())
        {
            launchVelocity = m_launchVelocities[i];
        }
        else
        {
            launchVelocity = QPointF((QRandomGenerator::global()->generateDouble() - 0.5) * 3,
                                     (QRandomGenerator::global()->generateDouble() - 0.5) * 3);
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
        if (i < m_launchDelayMs.size())
        {
            delayMs = m_launchDelayMs[i];
        }
        else if (m_enableDelay)
        {
            delayMs = QRandomGenerator::global()->bounded(m_maxDelayMs);
        }
        m_balls.append({ QPointF(width() / 2, height() / 2), launchVelocity, color, delayMs, {} });
    }
    m_ballsNext = QList<Ball>(m_balls.size());
    m_timerId = startTimer(m_timerIntervalMs);
    m_initialized = true;
}

void LaunchingBall::paint(QPainter *painter)
{
    if (!m_initialized)
    {
        return;
    }
    painter->setRenderHint(QPainter::Antialiasing, true);
    if (m_trailLength > 0)
    {
        for (int i = 0; i < m_trailLength; ++i)
        {
            int k = 0;
            for (int j = 0; j < m_balls.size(); j++)
            {
                const auto &ball = m_balls[j];
                if (ball.trail.size() <= i + 1)
                    continue;
                k += 1;
                QLinearGradient gradient(ball.trail[i], ball.trail[i + 1]);
                QColor beginColor = ball.color;
                beginColor.setAlphaF(1.0 - (double)i / m_trailLength);
                QColor endColor = ball.color;
                endColor.setAlphaF(1.0 - (double)(i + 1) / m_trailLength);
                gradient.setColorAt(0, beginColor);
                gradient.setColorAt(1, endColor);
                painter->setPen(QPen(gradient, 1));

                painter->drawLine(ball.trail[i], ball.trail[i + 1]);
            }
            if (k == 0)
                break;
        }
    }

    painter->setPen(Qt::NoPen);

    for (const Ball &ball : m_balls)
    {
        if (ball.daleyMs > 0)
        {
            continue;
        }
        painter->setBrush(ball.color);
        painter->drawEllipse(ball.position, 5, 5);
    }
}

void LaunchingBall::timerEvent([[maybe_unused]] QTimerEvent *event)
{
    updateBalls(); // 更新物体的位置、速度
    update();      // 请求重新绘制
}

void LaunchingBall::handleBalls(int begin, int end)
{
    for (int i(begin); i < end; ++i)
    {
        auto &ball = m_balls[i];
        if (ball.daleyMs > 0)
        {
            ball.daleyMs -= m_timerIntervalMs;
            m_ballsNext[i] = ball;
            continue;
        }
        auto &ballNext = m_ballsNext[i];
        ballNext = ball;
        if (m_trailLength > 0)
        {
            ballNext.trail.prepend(ballNext.position);
            if (ballNext.trail.size() > m_trailLength)
            {
                ballNext.trail.removeLast();
            }
        }

        ballNext.position += ballNext.velocity;
        if (m_bounce)
        {
            if (ballNext.position.x() < 10 || ballNext.position.x() > width() - 10)
            {
                ballNext.velocity.rx() *= -1;
            }
            if (ballNext.position.y() < 10 || ballNext.position.y() > height() - 10)
            {
                ballNext.velocity.ry() *= -1;
            }
        }
    }
}

void LaunchingBall::updateBalls()
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
    m_balls = std::move(m_ballsNext);
    m_ballsNext = QList<Ball>(m_balls.size());
    runHandleBodies(m_futures);
}

void LaunchingBall::runHandleBodies(QList<QFuture<void>> &futures)
{
    const int threadCount = QThreadPool::globalInstance()->maxThreadCount();
    const int bodiesPerThread = qCeil(double(m_balls.size()) / double(threadCount));
    for (int i(0); i < threadCount; ++i)
    {
        const int begin = i * bodiesPerThread;
        const int end = (i + 1) * bodiesPerThread;
        if (begin >= m_balls.size())
        {
            break;
        }
        if (end > m_balls.size())
        {
            futures.append(QtConcurrent::run(&LaunchingBall::handleBalls, this, begin, m_balls.size()));
            break;
        }
        futures.append(QtConcurrent::run(&LaunchingBall::handleBalls, this, begin, end));
    }
}
