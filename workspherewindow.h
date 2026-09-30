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
#include "./database/Database.h"
#include "./widgetmodels/CircularProgressWidget.h"

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
    void on_btnCreateEmployee_clicked();
    void on_btnDeleteEmployee_clicked();
    void onEmployeeCellChanged(int row, int column);
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
    void loadEmployeesToTable();
    void refreshEmployeeTable();
    void updateDashboardStats();
    QWidget* createInputRow(QString labelText, QWidget *inputField, QWidget *parent = nullptr);
    bool m_isDarkTheme = true;
    bool m_isLoadingData = false;

    QPushButton *m_activeNavButton = nullptr;
    QLineEdit *editProjectName = nullptr;
    QLineEdit *searchEmployeeInput;
    QDateEdit *editProjectDeadline = nullptr;
    std::vector<Project> projectsList;
    QComboBox *comboCurrency = nullptr;
    QComboBox *comboDateFormat = nullptr;
    QComboBox *comboTheme = nullptr;
    QLineEdit *txtEmployeeName = nullptr;
    QLineEdit *txtEmployeeSalary = nullptr;
    QComboBox *comboEmployeeType = nullptr;
    QLineEdit *txtEmployeePosition = nullptr;
    QLineEdit *txtEmployeeBonus = nullptr;
    CircularProgressWidget *widgetEmployees = nullptr;
    CircularProgressWidget *widgetProjects = nullptr;
    CircularProgressWidget *widgetPayroll = nullptr;
    Database m_database;

    Ui::WorkSphereWindow *ui = nullptr;
};

#endif // WORKSPHEREWINDOW_H