#include "CircularProgressWidget.h"

CircularProgressWidget::CircularProgressWidget(QString text, QColor col, QWidget *parent)
    : QWidget(parent), value(0), maxValue(100), labelText(text), color(col) {
    setFixedSize(140, 140);
}

void CircularProgressWidget::setValue(int val, int max) {
    value = val;
    maxValue = max > 0 ? max : 1;
    update();
}

void CircularProgressWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int side = qMin(width(), height());
    QRectF rect(10, 10, side - 20, side - 20);

    // Pozadinski deblji prsten (debljina 18px)
    QPen bgPen(QColor("#212d52"), 18, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(bgPen);
    painter.drawEllipse(rect);

    // Prsten napretka (debljina 18px)
    QPen progressPen(color, 18, Qt::SolidLine, Qt::RoundCap);
    painter.setPen(progressPen);

    double percentage = static_cast<double>(value) / maxValue;
    if (percentage > 1.0) percentage = 1.0;
    int angle = static_cast<int>(360.0 * percentage * 16);

    painter.drawArc(rect, 90 * 16, -angle);

    // Veličina fonta i ispis broja u centru kruga
    painter.setPen(color);
    painter.setFont(QFont("Arial", 24, QFont::Bold));
    painter.drawText(rect, Qt::AlignCenter, QString::number(value));
}