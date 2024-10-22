import QtQuick

Item {
    id: root

    property int bodyCount: 3
    property var bodyColor: []
    property var bodyRadius: [24]
    property int trailLength: 32
    property int timerInterval: 16
    property double velocityMultiplier: 16
    property double maxDistance: Math.min(width, height) - 20
    property double maxVelocity: 2

    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            const ctx = canvas.getContext("2d");
            ctx.clearRect(0, 0, canvas.width, canvas.height); // 清空画布

            // 绘制拖尾
            for (let i = 0; i < root.trailLength; i++) {
                let k = 0;
                for(let j = 0; j < root.bodies.length; j++) {
                    let body = bodies[j];
                    if (i < body.trail.length - 1) {
                        k += 1;
                        ctx.beginPath();
                        let gradient = ctx.createLinearGradient(body.trail[i].x, body.trail[i].y, body.trail[i + 1].x, body.trail[i + 1].y);
                        gradient.addColorStop(0, Qt.rgba(body.color.r, body.color.g, body.color.b, i / body.trail.length));
                        gradient.addColorStop(1, Qt.rgba(body.color.r, body.color.g, body.color.b, (i + 1) / body.trail.length));
                        ctx.strokeStyle = gradient;
                        ctx.lineWidth = 5;
                        ctx.moveTo(body.trail[i].x, body.trail[i].y);
                        ctx.lineTo(body.trail[i + 1].x, body.trail[i + 1].y);
                        ctx.stroke();
                        ctx.closePath();
                    }
                }
                if (k === 0) {
                    break;
                }
            }

            // 绘制三体
            for (let i = 0; i < root.bodies.length; i++) {
                let body = bodies[i];
                ctx.beginPath();
                ctx.arc(body.x, body.y, 10, 0, Math.PI * 2, true);
                ctx.fillStyle = body.color;
                ctx.fill();
                ctx.closePath();
            }

            // 更新三体的位置
            updateBodies();
        }
    }

    Timer {
        interval: root.timerInterval // 每 16 毫秒刷新
        running: true
        repeat: true
        onTriggered: {
            canvas.requestPaint(); // 请求重绘
        }
    }

    // 三体的数组，初始位置在中心
    property var bodies: (function() {
        let bodies = [];
        for (let i = 0; i < root.bodyCount; i++) {
            bodies.push({ x: width / 2 + maxDistance * (Math.random() - 0.5), y: height / 2 + maxDistance * (Math.random() - 0.5), vx: 0, vy: 0, r: i < bodyRadius.length ? bodyRadius[i] : bodyRadius[0], color: i < bodyColor.length ? bodyColor[i] : Qt.rgba(Math.random(), Math.random(), Math.random(), 1), trail: [] });
        }
        return bodies;
    })()

    function updateBodies() {
        for (let i = 0; i < bodies.length; i++) {
            let body = bodies[i];

            // 简单的引力计算
            for (let j = 0; j < bodies.length; j++) {
                if (i !== j) {
                    let other = bodies[j];
                    let dx = other.x - body.x;
                    let dy = other.y - body.y;
                    let distance = Math.sqrt(dx * dx + dy * dy) + body.r + other.r;
                    let force = 100 / (distance * distance); // 简化的引力公式

                    // 更新速度
                    body.vx += (dx / distance) * force;
                    body.vy += (dy / distance) * force;
                }
            }

            // 更新位置
            body.x += body.vx * velocityMultiplier;
            body.y += body.vy * velocityMultiplier;

            // 边界反弹
            if (body.x < 10 || body.x > canvas.width - 10) body.vx *= -1;
            if (body.y < 10 || body.y > canvas.height - 10) body.vy *= -1;

            // 更新拖尾
            body.trail.push({ x: body.x, y: body.y });
            if (body.trail.length > root.trailLength) {
                body.trail.shift(); // 限制拖尾长度
            }
        }
        if (maxVelocity === -1)
            return;
        let reset = false;

        for (let i = 0; i < bodies.length; i++) {
            let body = bodies[i];
            if (Math.abs(body.vx) > maxVelocity || Math.abs(body.vy) > maxVelocity) {
                reset = true;
                break;
            }
        }
        if (reset) {
            for (let i = 0; i < bodies.length; i++) {
                bodies[i].x = width / 2 + maxDistance * (Math.random() - 0.5);
                bodies[i].y = height / 2 + maxDistance * (Math.random() - 0.5);
                bodies[i].vx = 0;
                bodies[i].vy = 0;
                bodies[i].trail = [];
            }
        }
    }
}
