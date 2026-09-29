#ifndef CIRCULARPROGRESSWIDGET_H
#define CIRCULARPROGRESSWIDGET_H

#include <QWidget>
#include <QPainter>

class CircularProgressWidget : public QWidget {
    Q_OBJECT
private:
    int value;
    int maxValue;
    QString labelText;
    QColor color;
public:
    explicit CircularProgressWidget(QString text, QColor col, QWidget *parent = nullptr);

    void setValue(int value, int maxValue = 100);
protected:
    void paintEvent(QPaintEvent *) override;
};

#endif // CIRCULARPROGRESSWIDGET_H
