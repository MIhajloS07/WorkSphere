#ifndef WORKSPHEREWINDOW_H
#define WORKSPHEREWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

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
    void setupNavigation();
    void setupPages();
    void setupDashboard();
    void setupTimer();
    void createEmployeeForm();

    QStackedWidget *stackedWidget;
    QWidget *dashboardPage;
    QWidget *employeeFormPage;
    QTimer *timer;
    Ui::WorkSphereWindow *ui;
};
#endif // WORKSPHEREWINDOW_H
