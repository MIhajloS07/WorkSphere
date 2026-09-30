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
    QString suffix;
    QColor color;
public:
    explicit CircularProgressWidget(QString text, QColor col, QWidget *parent = nullptr);

    void setValue(int value, int maxValue = 100);
    void setSuffix(const QString &s) { suffix = s; update(); }
protected:
    void paintEvent(QPaintEvent *) override;
};

#endif // CIRCULARPROGRESSWIDGET_H
