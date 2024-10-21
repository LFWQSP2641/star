import QtQuick

Item {
    id: root

    property bool bounce: true
    property int ballCount: 1000
    property bool enableDelay: true
    property color ballColor: []

    Canvas {
        id: canvas
        anchors.fill: parent
        onPaint: {
            const ctx = canvas.getContext("2d");
            ctx.clearRect(0, 0, canvas.width, canvas.height); // 清空画布

            // 绘制动态原点
            for (let i = 0; i < root.points.length; i++)
            {
                if (root.points[i].delay > 0)
                {
                    root.points[i].delay -= 1;
                    continue;
                }
                let point = points[i];
                ctx.beginPath();
                ctx.arc(point.x, point.y, 5, 0, Math.PI * 2, true);
                ctx.fillStyle = point.color;
                ctx.fill();
                ctx.closePath();

                // 更新原点的位置
                point.x += Math.cos(point.angle) * point.speed;
                point.y += Math.sin(point.angle) * point.speed;

                // 检测边界反弹
                if (bounce)
                {
                    if (point.x <= 5 || point.x >= canvas.width - 5)
                    {
                        point.angle = Math.PI - point.angle; // 水平反弹
                    }
                    if (point.y <= 5 || point.y >= canvas.height - 5)
                    {
                        point.angle = -point.angle; // 垂直反弹
                    }
                }
                else
                {
                    if (point.x <= 5 || point.x >= canvas.width - 5 || point.y <= 5 || point.y >= canvas.height - 5)
                    {
                        point.x = width / 2;
                        point.y = height / 2;
                        point.angle = Math.random() * 2 * Math.PI; // 随机角度
                        point.speed = Math.random() * 2 + 1; // 随机速度
                    }
                }
            }
        }
    }

    Timer {
        interval: 16 // 每 16 毫秒刷新
        running: true
        repeat: true
        onTriggered: {
            canvas.requestPaint(); // 请求重绘
        }
    }

    // 动态点的数组
    property var points: (function() {
        let pts = [];
        for (let i = 0; i < ballCount; i++) {
            let angle = Math.random() * 2 * Math.PI; // 随机角度
            let speed = Math.random() * 2 + 1; // 随机速度
            let delay = 0;
            if (root.enableDelay)
                delay = Math.random() * 500; // 随机延迟
            let color = i < ballColor.length ? ballColor[i] : Qt.rgba(Math.random(), Math.random(), Math.random(), 1);
            pts.push({ x: width / 2, y: height / 2, angle: angle, speed: speed, delay: delay, color: color });
        }
        return pts;
    })()
}
