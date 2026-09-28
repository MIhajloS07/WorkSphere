#ifndef WORKSPHEREWINDOW_H
#define WORKSPHEREWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class WorkSphereWindow;
}
QT_END_NAMESPACE

class WorkSphereWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit WorkSphereWindow(QWidget *parent = nullptr);
    ~WorkSphereWindow() override;

private:
    Ui::WorkSphereWindow *ui;
};
#endif // WORKSPHEREWINDOW_H
