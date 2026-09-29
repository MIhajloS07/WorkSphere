#ifndef WORKSPHEREWINDOW_H
#define WORKSPHEREWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLineEdit>
#include <QDateEdit>
#include <QTimer>
#include <QComboBox>
#include <QPushButton>
#include <QLayout>
#include <vector>
#include "./models/Project.h"

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

private slots:
    void on_btnProjects_clicked();
    void on_btnSaveProject_clicked();
    void saveSettings();

private:
    void setupNavigation();
    void setupDashboard();
    void setupTimer();
    void createEmployeeForm();
    void createProjectForm();
    void createSettingsForm();
    void applyTheme(const QString &themeName);
    void updateNavigationStyles();
    void clearLayout(QLayout *layout);

    bool m_isDarkTheme = true;
    QPushButton *m_activeNavButton = nullptr;

    QLineEdit *editProjectName = nullptr;
    QDateEdit *editProjectDeadline = nullptr;
    std::vector<Project> projectsList;

    QComboBox *comboCurrency = nullptr;
    QComboBox *comboDateFormat = nullptr;
    QComboBox *comboTheme = nullptr;

    Ui::WorkSphereWindow *ui = nullptr;
};

#endif // WORKSPHEREWINDOW_H