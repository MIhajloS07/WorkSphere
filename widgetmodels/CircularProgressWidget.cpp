#include "CircularProgressWidget.h"

CircularProgressWidget::CircularProgressWidget(QString text, QColor col, QWidget *parent)
    : QWidget(parent), value(0), maxValue(100), labelText(text), color(col) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFixedSize(140, 140);
}

void CircularProgressWidget::setValue(int val, int max) {
    value = val;
    maxValue = max > 0 ? max : 1;
    update();
}

void CircularProgressWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int margin = 10;
    int side = qMin(width(), height()) - (margin * 2);
    if (side <= 0) return;

    QRectF rect((width() - side) / 2.0, (height() - side) / 2.0, side, side);

    int penWidth = qMax(10, side / 10);

    // 1. Crtanje pozadinskog luka
    QPen bgPen(QColor(30, 41, 59), penWidth, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(bgPen);
    painter.drawArc(rect, 0, 360 * 16);

    // 2. Crtanje popunjenog luka
    QPen progressPen(color, penWidth, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(progressPen);

    double maxV = (maxValue > 0) ? maxValue : 100;
    int spanAngle = static_cast<int>(-(value / static_cast<double>(maxV)) * 360 * 16);
    painter.drawArc(rect, 90 * 16, spanAngle);

    QFont font = painter.font();
    font.setPixelSize(qMax(16, side / 4));
    font.setBold(true);
    painter.setFont(font);

    painter.setPen(color);
    painter.drawText(rect, Qt::AlignCenter, QString::number(value));
}