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
#include <QMessageBox>
#include <QSettings>

WorkSphereWindow::WorkSphereWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::WorkSphereWindow)
{
    ui->setupUi(this);

    setupDashboard();
    setupNavigation();
    setupTimer();

    QSettings settings("WorkSphere", "WorkSphereApp");
    QString savedTheme = settings.value("general/theme", "Dark Theme (Default)").toString();
    applyTheme(savedTheme);

    // LOGO SETTING
    QString logoPath = QCoreApplication::applicationDirPath() + "/logo.svg";
    QPixmap logoPixmap(logoPath);

    if (!logoPixmap.isNull() && ui->labelLogoIcon) {
        ui->labelLogoIcon->setStyleSheet("background: transparent; border: none;");
        ui->labelLogoIcon->setAlignment(Qt::AlignCenter);
        ui->labelLogoIcon->setFixedSize(224, 224);

        QPixmap scaledLogo = logoPixmap.scaled(ui->labelLogoIcon->size(),
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);

        ui->labelLogoIcon->setPixmap(scaledLogo);
    }
}

WorkSphereWindow::~WorkSphereWindow()
{
    delete ui;
}

void WorkSphereWindow::clearLayout(QLayout *layout)
{
    if (!layout) return;
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        if (child->layout()) {
            clearLayout(child->layout());
        }
        if (child->widget()) {
            child->widget()->deleteLater();
        }
        delete child;
    }
}

void WorkSphereWindow::applyTheme(const QString &themeName)
{
    m_isDarkTheme = !themeName.contains("Light", Qt::CaseInsensitive);

    QString mainBgColor, textColor, cardBgColor, cardBorderColor, tableHeaderBg;

    if (m_isDarkTheme) {
        mainBgColor = "#090d16";
        textColor = "#ffffff";
        cardBgColor = "#0f172a";
        cardBorderColor = "#1e293b";
        tableHeaderBg = "#1e293b";
    } else {
        mainBgColor = "#f8fafc";
        textColor = "#0f172a";
        cardBgColor = "#ffffff";
        cardBorderColor = "#cbd5e1";
        tableHeaderBg = "#f1f5f9";
    }

    this->setStyleSheet(QString(R"(
        QMainWindow, QWidget#centralwidget, QWidget#dashboardPage,
        QWidget#employeeFormPage, QWidget#projectFormPage, QWidget#settingsPage {
            background-color: %1;
            color: %2;
        }
        QTableWidget {
            background-color: %3;
            color: %2;
            gridline-color: %4;
            border: 1px solid %4;
            border-radius: 8px;
        }
        QHeaderView::section {
            background-color: %5;
            color: %2;
            padding: 8px;
            border: none;
            font-weight: bold;
        }
    )").arg(mainBgColor, textColor, cardBgColor, cardBorderColor, tableHeaderBg));

    if (ui->labelDashboardTitle) {
        ui->labelDashboardTitle->setStyleSheet(QString("color: %1; font-size: 26px; font-weight: bold; background: transparent;").arg(textColor));
    }


    QString cardStyle = QString(R"(
        QWidget {
            background-color: %1;
            border: 1px solid %2;
            border-radius: 12px;
        }
        QLabel {
            color: %3;
            border: none;
            background: transparent;
        }
    )").arg(cardBgColor, cardBorderColor, textColor);

    if (ui->verticalLayoutCard1 && ui->verticalLayoutCard1->parentWidget()) {
        ui->verticalLayoutCard1->parentWidget()->setStyleSheet(cardStyle);
    }
    if (ui->verticalLayoutCard2 && ui->verticalLayoutCard2->parentWidget()) {
        ui->verticalLayoutCard2->parentWidget()->setStyleSheet(cardStyle);
    }
    if (ui->verticalLayoutCard3 && ui->verticalLayoutCard3->parentWidget()) {
        ui->verticalLayoutCard3->parentWidget()->setStyleSheet(cardStyle);
    }

    updateNavigationStyles();
    createEmployeeForm();
    createSettingsForm();
    if (ui->stackedWidget->currentWidget() == ui->projectFormPage) {
        createProjectForm();
    }
}

void WorkSphereWindow::updateNavigationStyles()
{
    if (!m_activeNavButton) {
        m_activeNavButton = ui->btnDashboard;
    }

    QString activeStyle;
    QString normalStyle;

    if (m_isDarkTheme) {
        activeStyle = "QPushButton { background-color: #0f172a; color: #ffffff; border: none; border-radius: 8px; text-align: center; padding: 14px; font-family: 'Segoe UI'; font-size: 16px; font-weight: bold; }";
        normalStyle = "QPushButton { background-color: transparent; color: #94a3b8; border: none; border-radius: 8px; text-align: center; padding: 14px; font-family: 'Segoe UI'; font-size: 16px; font-weight: normal; } QPushButton:hover { background-color: #0f172a; color: #ffffff; }";
    } else {
        activeStyle = "QPushButton { background-color: #e2e8f0; color: #0f172a; border: none; border-radius: 8px; text-align: center; padding: 14px; font-family: 'Segoe UI'; font-size: 16px; font-weight: bold; }";
        normalStyle = "QPushButton { background-color: transparent; color: #64748b; border: none; border-radius: 8px; text-align: center; padding: 14px; font-family: 'Segoe UI'; font-size: 16px; font-weight: normal; } QPushButton:hover { background-color: #e2e8f0; color: #0f172a; }";
    }

    ui->btnDashboard->setStyleSheet(ui->btnDashboard == m_activeNavButton ? activeStyle : normalStyle);
    ui->btnEmployees->setStyleSheet(ui->btnEmployees == m_activeNavButton ? activeStyle : normalStyle);
    ui->btnProjects->setStyleSheet(ui->btnProjects == m_activeNavButton ? activeStyle : normalStyle);
    if (ui->btnSettings) {
        ui->btnSettings->setStyleSheet(ui->btnSettings == m_activeNavButton ? activeStyle : normalStyle);
    }
}

void WorkSphereWindow::setupDashboard()
{
    if (ui->dashboardPage->layout()) {
        ui->dashboardPage->layout()->setContentsMargins(20, 10, 20, 15);
        ui->dashboardPage->layout()->setSpacing(10);
    }

    QList<QVBoxLayout*> cardLayouts = {ui->verticalLayoutCard1, ui->verticalLayoutCard2, ui->verticalLayoutCard3};
    for (QVBoxLayout* layout : cardLayouts) {
        if (layout) {
            layout->setContentsMargins(10, 10, 10, 10);
            layout->setSpacing(0);
            layout->setAlignment(Qt::AlignCenter);
        }
    }

    auto createExpandedWidget = [this](const QString &title, const QColor &color, int val, int maxVal, QWidget *oldWidget, QVBoxLayout *cardLayout) {
        CircularProgressWidget *w = new CircularProgressWidget(title, color, this);
        w->setValue(val, maxVal);

        w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        w->setMinimumSize(150, 150);

        cardLayout->replaceWidget(oldWidget, w);
        cardLayout->setAlignment(w, Qt::AlignCenter);
        delete oldWidget;
    };

    createExpandedWidget("Employees", QColor(76, 201, 240), 84, 100, ui->lblValueEmp, ui->verticalLayoutCard1);
    createExpandedWidget("Projects", QColor(13, 148, 136), 12, 20, ui->lblValueProj, ui->verticalLayoutCard2);
    createExpandedWidget("Payroll", QColor(99, 102, 241), 234, 5000, ui->lblValuePayroll, ui->verticalLayoutCard3);

    ui->tableRecentEmployees->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->tableRecentEmployees->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void WorkSphereWindow::setupNavigation()
{
    m_activeNavButton = ui->btnDashboard;

    connect(ui->btnDashboard, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->dashboardPage);
        m_activeNavButton = ui->btnDashboard;
        updateNavigationStyles();
    });

    connect(ui->btnEmployees, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->employeeFormPage);
        m_activeNavButton = ui->btnEmployees;
        updateNavigationStyles();
    });

    connect(ui->btnProjects, &QPushButton::clicked, this, [this]() {
        m_activeNavButton = ui->btnProjects;
        updateNavigationStyles();
        on_btnProjects_clicked();
    });

    if (ui->btnSettings) {
        connect(ui->btnSettings, &QPushButton::clicked, this, [this]() {
            ui->stackedWidget->setCurrentWidget(ui->settingsPage);
            m_activeNavButton = ui->btnSettings;
            updateNavigationStyles();
        });
    }
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
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->employeeFormPage->layout());
    if (!mainLayout) return;

    clearLayout(mainLayout);

    QString titleColor = m_isDarkTheme ? "#ffffff" : "#0f172a";
    QString labelColor = m_isDarkTheme ? "#94a3b8" : "#475569";

    QLabel *labelFormTitle = new QLabel("Create New Employee / Manager", ui->employeeFormPage);
    labelFormTitle->setAlignment(Qt::AlignCenter);
    labelFormTitle->setStyleSheet(QString("color: %1; font-size: 22px; font-weight: bold; font-family: 'Segoe UI'; margin-bottom: 25px;").arg(titleColor));
    mainLayout->addWidget(labelFormTitle);

    QWidget *formContainer = new QWidget(ui->employeeFormPage);
    formContainer->setMaximumWidth(750);
    formContainer->setMinimumWidth(580);
    QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
    formLayout->setSpacing(16);
    formLayout->setContentsMargins(20, 0, 20, 0);

    auto createInputRow = [labelColor](QString labelText, QWidget *inputWidget, QWidget *parent) {
        QWidget *rowWidget = new QWidget(parent);
        QVBoxLayout *vBox = new QVBoxLayout(rowWidget);
        vBox->setContentsMargins(0, 0, 0, 0);
        vBox->setSpacing(6);

        QLabel *lbl = new QLabel(labelText, rowWidget);
        lbl->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: bold; font-family: 'Segoe UI';").arg(labelColor));
        vBox->addWidget(lbl);

        vBox->addWidget(inputWidget);
        return rowWidget;
    };

    QLineEdit *txtName = new QLineEdit(formContainer);
    txtName->setPlaceholderText("Enter employee name...");
    QWidget *rowName = createInputRow("Full Name", txtName, formContainer);

    QLineEdit *txtSalary = new QLineEdit(formContainer);
    txtSalary->setPlaceholderText("Enter salary amount...");
    QWidget *rowSalary = createInputRow("Salary ($)", txtSalary, formContainer);

    QComboBox *comboType = new QComboBox(formContainer);
    comboType->addItem("Worker");
    comboType->addItem("Manager");
    QWidget *rowType = createInputRow("Employee Type", comboType, formContainer);

    QStackedWidget *stackedFields = new QStackedWidget(formContainer);

    QLineEdit *txtPosition = new QLineEdit();
    txtPosition->setPlaceholderText("Enter worker position...");
    QWidget *workerPage = createInputRow("Position", txtPosition, stackedFields);
    stackedFields->addWidget(workerPage);

    QLineEdit *txtBonus = new QLineEdit();
    txtBonus->setPlaceholderText("Enter manager bonus...");
    QWidget *managerPage = createInputRow("Bonus ($)", txtBonus, stackedFields);
    stackedFields->addWidget(managerPage);

    connect(comboType, QOverload<int>::of(&QComboBox::currentIndexChanged), stackedFields, &QStackedWidget::setCurrentIndex);

    QPushButton *btnCreate = new QPushButton("Create Employee", formContainer);
    btnCreate->setObjectName("createButton");
    btnCreate->setCursor(Qt::PointingHandCursor);

    QString formStyle = m_isDarkTheme ? R"(
        QLineEdit, QComboBox {
            background-color: #040711;
            color: #ffffff;
            border: 1px solid #1e293b;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
        }
        QLineEdit:focus, QComboBox:focus {
            border: 1px solid #00d2ff;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 25px;
            border-left-width: 0px;
        }
        QComboBox QAbstractItemView {
            background-color: #0f172a;
            color: #ffffff;
            selection-background-color: #00d2ff;
            selection-color: #000000;
        }
        QPushButton#createButton {
            background-color: #0f172a;
            color: #00d2ff;
            border: 1px solid #00d2ff;
            border-radius: 8px;
            font-weight: bold;
            font-size: 15px;
            padding: 13px;
            margin-top: 15px;
        }
        QPushButton#createButton:hover {
            background-color: #10b981;
            color: #ffffff;
            border: 1px solid #10b981;
        }
        QPushButton#createButton:pressed {
            background-color: #059669;
        }
    )" : R"(
        QLineEdit, QComboBox {
            background-color: #ffffff;
            color: #0f172a;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
        }
        QLineEdit:focus, QComboBox:focus {
            border: 1px solid #2563eb;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 25px;
            border-left-width: 0px;
        }
        QComboBox QAbstractItemView {
            background-color: #ffffff;
            color: #0f172a;
            selection-background-color: #2563eb;
            selection-color: #ffffff;
        }
        QPushButton#createButton {
            background-color: #2563eb;
            color: #ffffff;
            border: 1px solid #2563eb;
            border-radius: 8px;
            font-weight: bold;
            font-size: 15px;
            padding: 13px;
            margin-top: 15px;
        }
        QPushButton#createButton:hover {
            background-color: #1d4ed8;
            color: #ffffff;
            border: 1px solid #1d4ed8;
        }
        QPushButton#createButton:pressed {
            background-color: #1e40af;
        }
    )";
    formContainer->setStyleSheet(formStyle);

    formLayout->addWidget(rowName);
    formLayout->addWidget(rowSalary);
    formLayout->addWidget(rowType);
    formLayout->addWidget(stackedFields);
    formLayout->addWidget(btnCreate);

    mainLayout->addStretch();

    QHBoxLayout *centerWrapper = new QHBoxLayout();
    centerWrapper->addStretch();
    centerWrapper->addWidget(formContainer);
    centerWrapper->addStretch();

    mainLayout->addLayout(centerWrapper);
    mainLayout->addStretch();
}

void WorkSphereWindow::on_btnProjects_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->projectFormPage);
    createProjectForm();
}

void WorkSphereWindow::createProjectForm()
{
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->projectFormPage->layout());
    if (!mainLayout) return;

    clearLayout(mainLayout);

    QString titleColor = m_isDarkTheme ? "#ffffff" : "#0f172a";
    QString labelColor = m_isDarkTheme ? "#94a3b8" : "#475569";

    QLabel *labelFormTitle = new QLabel("Create New Project", ui->projectFormPage);
    labelFormTitle->setAlignment(Qt::AlignCenter);
    labelFormTitle->setStyleSheet(QString("color: %1; font-size: 22px; font-weight: bold; font-family: 'Segoe UI'; margin-bottom: 25px;").arg(titleColor));
    mainLayout->addWidget(labelFormTitle);

    // MAIN CONTAINER
    QWidget *formContainer = new QWidget(ui->projectFormPage);
    formContainer->setMaximumWidth(750);
    formContainer->setMinimumWidth(580);
    QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
    formLayout->setSpacing(16);
    formLayout->setContentsMargins(20, 0, 20, 0);

    auto createInputRow = [labelColor](QString labelText, QWidget *inputWidget, QWidget *parent) {
        QWidget *rowWidget = new QWidget(parent);
        QVBoxLayout *vBox = new QVBoxLayout(rowWidget);
        vBox->setContentsMargins(0, 0, 0, 0);
        vBox->setSpacing(6);

        QLabel *lbl = new QLabel(labelText, rowWidget);
        lbl->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: bold; font-family: 'Segoe UI';").arg(labelColor));
        vBox->addWidget(lbl);

        vBox->addWidget(inputWidget);
        return rowWidget;
    };

    // 1. PROJECT NAME
    editProjectName = new QLineEdit(formContainer);
    editProjectName->setPlaceholderText("Enter project name...");
    QWidget *rowName = createInputRow("Project Name", editProjectName, formContainer);

    // 2. DEADLINE
    editProjectDeadline = new QDateEdit(QDate::currentDate().addMonths(1), formContainer);
    editProjectDeadline->setCalendarPopup(true);
    editProjectDeadline->setDisplayFormat("dd.MM.yyyy");
    QWidget *rowDeadline = createInputRow("Deadline", editProjectDeadline, formContainer);

    // 3. SAVE BUTTON
    QPushButton *btnSave = new QPushButton("Add Project", formContainer);
    btnSave->setObjectName("createButton");
    btnSave->setCursor(Qt::PointingHandCursor);

    QString formStyle = m_isDarkTheme ? R"(
        QLineEdit, QDateEdit {
            background-color: #040711;
            color: #ffffff;
            border: 1px solid #1e293b;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            font-family: 'Segoe UI';
        }
        QLineEdit:focus, QDateEdit:focus {
            border: 1px solid #00d2ff;
        }
        QDateEdit::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 25px;
            border-left-width: 0px;
        }
        QPushButton#createButton {
            background-color: #0f172a;
            color: #00d2ff;
            border: 1px solid #00d2ff;
            border-radius: 8px;
            font-weight: bold;
            font-size: 15px;
            padding: 13px;
            margin-top: 15px;
        }
        QPushButton#createButton:hover {
            background-color: #10b981;
            color: #ffffff;
            border: 1px solid #10b981;
        }
        QPushButton#createButton:pressed {
            background-color: #059669;
        }
    )" : R"(
        QLineEdit, QDateEdit {
            background-color: #ffffff;
            color: #0f172a;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            font-family: 'Segoe UI';
        }
        QLineEdit:focus, QDateEdit:focus {
            border: 1px solid #2563eb;
        }
        QDateEdit::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 25px;
            border-left-width: 0px;
        }
        QPushButton#createButton {
            background-color: #2563eb;
            color: #ffffff;
            border: 1px solid #2563eb;
            border-radius: 8px;
            font-weight: bold;
            font-size: 15px;
            padding: 13px;
            margin-top: 15px;
        }
        QPushButton#createButton:hover {
            background-color: #1d4ed8;
            color: #ffffff;
            border: 1px solid #1d4ed8;
        }
        QPushButton#createButton:pressed {
            background-color: #1e40af;
        }
    )";
    formContainer->setStyleSheet(formStyle);

    formLayout->addWidget(rowName);
    formLayout->addWidget(rowDeadline);
    formLayout->addWidget(btnSave);

    mainLayout->addStretch();

    QHBoxLayout *centerWrapper = new QHBoxLayout();
    centerWrapper->addStretch();
    centerWrapper->addWidget(formContainer);
    centerWrapper->addStretch();

    mainLayout->addLayout(centerWrapper);
    mainLayout->addStretch();

    connect(btnSave, &QPushButton::clicked, this, &WorkSphereWindow::on_btnSaveProject_clicked);
}

void WorkSphereWindow::on_btnSaveProject_clicked()
{
    if (!editProjectName || !editProjectDeadline) return;

    std::string name = editProjectName->text().trimmed().toStdString();
    QDate deadline = editProjectDeadline->date();

    if (name.empty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter a project name!");
        return;
    }

    projectsList.emplace_back(name, deadline);

    QMessageBox::information(this, "Success", "Project successfully added!");

    editProjectName->clear();
    editProjectDeadline->setDate(QDate::currentDate().addMonths(1));
}

void WorkSphereWindow::createSettingsForm()
{
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->settingsPage->layout());
    if (!mainLayout) return;

    clearLayout(mainLayout);

    QString titleColor = m_isDarkTheme ? "#ffffff" : "#0f172a";
    QString labelColor = m_isDarkTheme ? "#94a3b8" : "#475569";

    QLabel *labelFormTitle = new QLabel("General Settings", ui->settingsPage);
    labelFormTitle->setAlignment(Qt::AlignCenter);
    labelFormTitle->setStyleSheet(QString("color: %1; font-size: 22px; font-weight: bold; font-family: 'Segoe UI'; margin-bottom: 25px;").arg(titleColor));
    mainLayout->addWidget(labelFormTitle);

    QWidget *formContainer = new QWidget(ui->settingsPage);
    formContainer->setMaximumWidth(750);
    formContainer->setMinimumWidth(580);
    QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
    formLayout->setSpacing(18);
    formLayout->setContentsMargins(20, 0, 20, 0);

    auto createInputRow = [labelColor](QString labelText, QWidget *inputWidget, QWidget *parent) {
        QWidget *rowWidget = new QWidget(parent);
        QVBoxLayout *vBox = new QVBoxLayout(rowWidget);
        vBox->setContentsMargins(0, 0, 0, 0);
        vBox->setSpacing(6);

        QLabel *lbl = new QLabel(labelText, rowWidget);
        lbl->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: bold; font-family: 'Segoe UI';").arg(labelColor));
        vBox->addWidget(lbl);

        vBox->addWidget(inputWidget);
        return rowWidget;
    };

    QSettings settings("WorkSphere", "WorkSphereApp");
    QString savedCurrency = settings.value("general/currency", "USD ($)").toString();
    QString savedDateFormat = settings.value("general/dateFormat", "dd.MM.yyyy").toString();
    QString savedTheme = settings.value("general/theme", m_isDarkTheme ? "Dark Theme (Default)" : "Light Theme").toString();

    // 1. SELECTOR VALUE
    comboCurrency = new QComboBox(formContainer);
    comboCurrency->addItems({"EUR (€)", "USD ($)", "RSD (din)", "BAM (KM)"});
    comboCurrency->setCurrentText(savedCurrency);
    QWidget *rowCurrency = createInputRow("Application Currency", comboCurrency, formContainer);

    // 2. DATE FORMAT
    comboDateFormat = new QComboBox(formContainer);
    comboDateFormat->addItems({"dd.MM.yyyy", "yyyy-MM-dd", "MM/dd/yyyy"});
    comboDateFormat->setCurrentText(savedDateFormat);
    QWidget *rowDateFormat = createInputRow("Date Display Format", comboDateFormat, formContainer);

    // 3. APPLICATION THEME
    comboTheme = new QComboBox(formContainer);
    comboTheme->addItems({"Dark Theme (Default)", "Light Theme"});
    comboTheme->setCurrentText(savedTheme);
    QWidget *rowTheme = createInputRow("Appearance Theme", comboTheme, formContainer);

    // 4. SAVE BUTTON
    QPushButton *btnSaveSettings = new QPushButton("Save Preferences", formContainer);
    btnSaveSettings->setObjectName("createButton");
    btnSaveSettings->setCursor(Qt::PointingHandCursor);

    QString formStyle = m_isDarkTheme ? R"(
        QComboBox {
            background-color: #040711;
            color: #ffffff;
            border: 1px solid #1e293b;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            font-family: 'Segoe UI';
        }
        QComboBox:focus {
            border: 1px solid #00d2ff;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 25px;
            border-left-width: 0px;
        }
        QComboBox QAbstractItemView {
            background-color: #0f172a;
            color: #ffffff;
            selection-background-color: #00d2ff;
            selection-color: #000000;
        }
        QPushButton#createButton {
            background-color: #0f172a;
            color: #00d2ff;
            border: 1px solid #00d2ff;
            border-radius: 8px;
            font-weight: bold;
            font-size: 15px;
            padding: 13px;
            margin-top: 15px;
        }
        QPushButton#createButton:hover {
            background-color: #10b981;
            color: #ffffff;
            border: 1px solid #10b981;
        }
        QPushButton#createButton:pressed {
            background-color: #059669;
        }
    )" : R"(
        QComboBox {
            background-color: #ffffff;
            color: #0f172a;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            font-family: 'Segoe UI';
        }
        QComboBox:focus {
            border: 1px solid #2563eb;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 25px;
            border-left-width: 0px;
        }
        QComboBox QAbstractItemView {
            background-color: #ffffff;
            color: #0f172a;
            selection-background-color: #2563eb;
            selection-color: #ffffff;
        }
        QPushButton#createButton {
            background-color: #2563eb;
            color: #ffffff;
            border: 1px solid #2563eb;
            border-radius: 8px;
            font-weight: bold;
            font-size: 15px;
            padding: 13px;
            margin-top: 15px;
        }
        QPushButton#createButton:hover {
            background-color: #1d4ed8;
            color: #ffffff;
            border: 1px solid #1d4ed8;
        }
        QPushButton#createButton:pressed {
            background-color: #1e40af;
        }
    )";
    formContainer->setStyleSheet(formStyle);

    formLayout->addWidget(rowCurrency);
    formLayout->addWidget(rowDateFormat);
    formLayout->addWidget(rowTheme);
    formLayout->addWidget(btnSaveSettings);

    mainLayout->addStretch();

    QHBoxLayout *centerWrapper = new QHBoxLayout();
    centerWrapper->addStretch();
    centerWrapper->addWidget(formContainer);
    centerWrapper->addStretch();

    mainLayout->addLayout(centerWrapper);
    mainLayout->addStretch();

    connect(btnSaveSettings, &QPushButton::clicked, this, &WorkSphereWindow::saveSettings);
}

void WorkSphereWindow::saveSettings()
{
    if (!comboCurrency || !comboDateFormat || !comboTheme) return;

    QString selectedTheme = comboTheme->currentText();

    QSettings settings("WorkSphere", "WorkSphereApp");
    settings.setValue("general/currency", comboCurrency->currentText());
    settings.setValue("general/dateFormat", comboDateFormat->currentText());
    settings.setValue("general/theme", selectedTheme);
    applyTheme(selectedTheme);

    QMessageBox::information(this, "Settings Saved", "Your preferences have been successfully updated!");
}