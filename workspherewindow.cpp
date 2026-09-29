#include "workspherewindow.h"
#include "ui_workspherewindow.h"
#include "widgetmodels/CircularProgressWidget.h"
#include <QTimer>
#include <QTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QHeaderView>

WorkSphereWindow::WorkSphereWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::WorkSphereWindow)
{
    ui->setupUi(this);

    // Poziv modularnih metoda
    setupDashboard();
    setupNavigation();
    setupTimer();
    createEmployeeForm();
}

WorkSphereWindow::~WorkSphereWindow()
{
    delete ui;
}

void WorkSphereWindow::setupDashboard()
{
    // 1. Tabela
    ui->tableRecentEmployees->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->tableRecentEmployees->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 2. Kružni widgeti (korišćenje QColor sa RGB vrednostima umesto string literala)
    CircularProgressWidget *widgetEmp = new CircularProgressWidget("Employees", QColor(76, 201, 240), this);
    widgetEmp->setValue(84, 100);
    ui->verticalLayoutCard1->replaceWidget(ui->lblValueEmp, widgetEmp);
    ui->verticalLayoutCard1->setAlignment(widgetEmp, Qt::AlignCenter);
    delete ui->lblValueEmp;

    CircularProgressWidget *widgetProj = new CircularProgressWidget("Projects", QColor(13, 71, 85), this);
    widgetProj->setValue(12, 20);
    ui->verticalLayoutCard2->replaceWidget(ui->lblValueProj, widgetProj);
    ui->verticalLayoutCard2->setAlignment(widgetProj, Qt::AlignCenter);
    delete ui->lblValueProj;

    CircularProgressWidget *widgetPayroll = new CircularProgressWidget("Payroll", QColor(71, 152, 170), this);
    widgetPayroll->setValue(234, 5000);
    ui->verticalLayoutCard3->replaceWidget(ui->lblValuePayroll, widgetPayroll);
    ui->verticalLayoutCard3->setAlignment(widgetPayroll, Qt::AlignCenter);
    delete ui->lblValuePayroll;
}

void WorkSphereWindow::setupNavigation()
{
    // Korišćenje eksplicitnog [this] umesto [=] zbog C++20 standarda
    connect(ui->btnDashboard, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->dashboardPage);
    });

    connect(ui->btnEmployees, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->employeeFormPage);
    });
}

void WorkSphereWindow::setupTimer()
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        QString currentTime = QTime::currentTime().toString("hh:mm:ss");
        ui->clockLabel->setText(currentTime);
    });
    timer->start(1000);
}

void WorkSphereWindow::createEmployeeForm()
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(ui->employeeFormPage->layout());
    if (!layout) return;

    // Polje: Ime i prezime
    QLabel *lblName = new QLabel("Full Name", ui->employeeFormPage);
    lblName->setStyleSheet("color: #94a3b8; font-size: 13px;");
    QLineEdit *txtName = new QLineEdit(ui->employeeFormPage);
    txtName->setPlaceholderText("Enter employee name...");

    // Polje: Tip (Worker / Manager)
    QLabel *lblType = new QLabel("Employee Type", ui->employeeFormPage);
    lblType->setStyleSheet("color: #94a3b8; font-size: 13px;");
    QComboBox *comboType = new QComboBox(ui->employeeFormPage);
    comboType->addItem("Worker");
    comboType->addItem("Manager");

    // Polje: Plata
    QLabel *lblSalary = new QLabel("Salary ($)", ui->employeeFormPage);
    lblSalary->setStyleSheet("color: #94a3b8; font-size: 13px;");
    QLineEdit *txtSalary = new QLineEdit(ui->employeeFormPage);
    txtSalary->setPlaceholderText("Enter salary amount...");

    // Dugme za kreiranje sa zelenim hover efektom
    QPushButton *btnCreate = new QPushButton("Create Employee", ui->employeeFormPage);
    btnCreate->setObjectName("createButton");
    btnCreate->setCursor(Qt::PointingHandCursor);

    // QSS stil za formu i dugme
    QString formStyle = R"(
        QLineEdit, QComboBox {
            background-color: #040711;
            color: #ffffff;
            border: 1px solid #1e293b;
            border-radius: 8px;
            padding: 10px;
            font-size: 14px;
        }
        QLineEdit:focus, QComboBox:focus {
            border: 1px solid #00d2ff;
        }
        QPushButton#createButton {
            background-color: #0f172a;
            color: #00d2ff;
            border: 1px solid #00d2ff;
            border-radius: 8px;
            font-weight: bold;
            font-size: 14px;
            padding: 12px;
            margin-top: 15px;
        }
        QPushButton#createButton:hover {
            background-color: #10b981; /* Prelazi u zelenu boju na hover */
            color: #ffffff;
            border: 1px solid #10b981;
        }
        QPushButton#createButton:pressed {
            background-color: #059669;
        }
    )";
    ui->employeeFormPage->setStyleSheet(formStyle);
    layout->addWidget(lblName);
    layout->addWidget(txtName);
    layout->addWidget(lblType);
    layout->addWidget(comboType);
    layout->addWidget(lblSalary);
    layout->addWidget(txtSalary);
    layout->addWidget(btnCreate);

    layout->addStretch();
}