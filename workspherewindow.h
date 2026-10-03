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
#include <QEvent>
#include <QMouseEvent>
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

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_btnProjects_clicked();
    void on_btnSaveProject_clicked();
    void saveSettings();
    void on_btnDeleteEmployee_clicked();
    void onEmployeeCellChanged(int row, int column);
    void on_btnDeleteProject_clicked();
    void on_btnEditProject_clicked();
    void onProjectSelectionChanged();
    void on_btnEditEmployee_clicked();
    void on_btnSaveAssignment_clicked();

private:
    void setupNavigation();
    void setupDashboard();
    void setupProjectTable();
    void setupTimer();

    void createEmployeeForm();
    void createProjectForm();
    void createSettingsForm();
    void setActiveButtonStyle(QPushButton* activeButton);
    void loadAssignmentData();
    void applyTheme(const QString &themeName);
    void updateNavigationStyles();
    void clearLayout(QLayout *layout);

    void loadEmployeesToTable();
    void refreshEmployeeTable();
    void refreshProjectTable();
    void updateDashboardStats();
    void createAssignWorkerForm(QVBoxLayout *parentLayout);
    void addProjectActionButtons(int row, int projectId);

    void handleProjectSelectionChanged();
    void handleProjectsButtonClicked();
    void handleCreateEmployee();
    void handleEditEmployee();
    void handleDeleteEmployee();
    void handleEditProject();
    void handleEditProject(int projectId);
    void handleDeleteProject();
    void handleDeleteProject(int projectId);
    void handleSaveProject();
    void handleSaveAssignment();
    void handleAssignWorker();

    QWidget* createInputRow(const QString &labelText, QWidget *inputField, QWidget *parent = nullptr);
    QString formatDate(const QDate &date) const;
    QWidget *actionWidget = nullptr;

    bool m_isDarkTheme = true;
    bool m_isLoadingData = false;

    QPushButton *m_activeNavButton = nullptr;

    QDateEdit *editProjectDeadline = nullptr;

    QComboBox *comboCurrency = nullptr;
    QComboBox *comboDateFormat = nullptr;
    QComboBox *comboTheme = nullptr;
    QComboBox *comboAssignProject = nullptr;
    QComboBox *comboAssignEmployee = nullptr;
    QComboBox *comboEmployeeType = nullptr;

    QLineEdit *txtEmployeeName = nullptr;
    QLineEdit *txtEmployeeSalary = nullptr;
    QLineEdit *txtEmployeePosition = nullptr;
    QLineEdit *txtEmployeeBonus = nullptr;
    QLineEdit *searchEmployeeInput = nullptr;
    QLineEdit *searchProjectInput = nullptr;
    QLineEdit *editProjectName = nullptr;

    CircularProgressWidget *widgetEmployees = nullptr;
    CircularProgressWidget *widgetProjects = nullptr;
    CircularProgressWidget *widgetPayroll = nullptr;

    std::vector<Project> projectsList;
    Database m_database;

    Ui::WorkSphereWindow *ui = nullptr;
};

#endif // WORKSPHEREWINDOW_H