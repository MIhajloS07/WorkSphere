#include "workspherewindow.h"
#include "ui_workspherewindow.h"
#include "widgetmodels/CircularProgressWidget.h"
#include "models/Worker.h"
#include "models/Manager.h"
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
#include <QMenu>
#include <QRandomGenerator>
#include <QAction>
#include <QMouseEvent>
#include <QSqlQuery>
#include <QSqlError>
#include <QDialogButtonBox>

WorkSphereWindow::WorkSphereWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_database("DRIVER={ODBC Driver 17 for SQL Server};SERVER=LOCALHOST\\SQLEXPRESS;DATABASE=WorkSphere;Trusted_Connection=yes;Encrypt=yes;TrustServerCertificate=yes;")
    , ui(new Ui::WorkSphereWindow)
{
    ui->setupUi(this);

    refreshEmployeeTable();
    refreshProjectTable();
    setupDashboard();
    setupNavigation();
    setupTimer();

    // 1. CREATE SEARCH INPUTS
    searchEmployeeInput = new QLineEdit(this);
    searchEmployeeInput->setPlaceholderText("Search employees by ID or name...");

    searchProjectInput = new QLineEdit(this);
    searchProjectInput->setPlaceholderText("Search projects by name...");

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(15);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchLayout->addWidget(searchEmployeeInput);
    searchLayout->addWidget(searchProjectInput);

    QVBoxLayout *dashLayout = qobject_cast<QVBoxLayout*>(ui->dashboardPage->layout());
    if (dashLayout) {
        int index = dashLayout->indexOf(ui->tableRecentEmployees);
        if (index != -1) {
            dashLayout->insertLayout(index, searchLayout);
        }
    }

    // 2. APPLY SAVED THEME
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

    // SEARCH CONNECTIONS
    connect(searchEmployeeInput, &QLineEdit::textChanged, this, [this](const QString &text) {
        QString filter = text.trimmed().toLower();
        QTableWidget *table = ui->tableRecentEmployees;

        for (int row = 0; row < table->rowCount(); ++row) {
            bool match = false;
            QTableWidgetItem *idItem = table->item(row, 0);
            QTableWidgetItem *nameItem = table->item(row, 1);

            if (idItem && idItem->text().toLower().contains(filter)) {
                match = true;
            }
            if (nameItem && nameItem->text().toLower().contains(filter)) {
                match = true;
            }

            table->setRowHidden(row, !match);
        }
    });

    connect(searchProjectInput, &QLineEdit::textChanged, this, [this](const QString &text) {
        QString filter = text.trimmed().toLower();
        QTableWidget *table = ui->tableRecentProjects;

        for (int row = 0; row < table->rowCount(); ++row) {
            bool match = false;
            QTableWidgetItem *idItem = table->item(row, 0);
            QTableWidgetItem *nameItem = table->item(row, 1);
            QTableWidgetItem *clientItem = table->item(row, 2);
            if (idItem && idItem->text().toLower().contains(filter)) {
                match = true;
            }
            if (nameItem && nameItem->text().toLower().contains(filter)) {
                match = true;
            }
            if (clientItem && clientItem->text().toLower().contains(filter)) {
                match = true;
            }

            table->setRowHidden(row, !match);
        }
    });

    // COMMON BUTTON STYLES
    QString btnEditStyle = "QPushButton { background-color: #3b82f6; color: white; border-radius: 6px; padding: 8px 20px; font-weight: bold; font-family: 'Segoe UI'; }"
                           "QPushButton:hover { background-color: #2563eb; }";
    QString btnDeleteStyle = "QPushButton { background-color: #ef4444; color: white; border-radius: 6px; padding: 8px 20px; font-weight: bold; font-family: 'Segoe UI'; }"
                             "QPushButton:hover { background-color: #dc2626; }";

    // --- EMPLOYEE TABLE SETUP ---
    ui->tableRecentEmployees->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableRecentEmployees->setSelectionMode(QAbstractItemView::SingleSelection);

    actionWidget = new QWidget(this);
    QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
    actionLayout->setContentsMargins(0, 10, 0, 10);

    QPushButton *btnEditEmp = new QPushButton("Edit", this);
    QPushButton *btnDeleteEmp = new QPushButton("Delete", this);

    btnEditEmp->setStyleSheet(btnEditStyle);
    btnEditEmp->setCursor(Qt::PointingHandCursor);

    btnDeleteEmp->setStyleSheet(btnDeleteStyle);
    btnDeleteEmp->setCursor(Qt::PointingHandCursor);

    actionLayout->addWidget(btnEditEmp);
    actionLayout->addWidget(btnDeleteEmp);
    actionLayout->addStretch();

    actionWidget->setFixedHeight(50);
    ui->verticalLayoutDashboard->addWidget(actionWidget);
    actionWidget->setVisible(false);

    // --- PROJECT TABLE SETUP ---
    ui->tableRecentProjects->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableRecentProjects->setSelectionMode(QAbstractItemView::SingleSelection);

    QWidget *projectActionWidget = new QWidget(this);
    QHBoxLayout *projActionLayout = new QHBoxLayout(projectActionWidget);
    projActionLayout->setContentsMargins(0, 10, 0, 10);

    QPushButton *btnEditProj = new QPushButton("Edit Project", this);
    QPushButton *btnDeleteProj = new QPushButton("Delete Project", this);

    btnEditProj->setStyleSheet(btnEditStyle);
    btnEditProj->setCursor(Qt::PointingHandCursor);

    btnDeleteProj->setStyleSheet(btnDeleteStyle);
    btnDeleteProj->setCursor(Qt::PointingHandCursor);

    projActionLayout->addWidget(btnEditProj);
    projActionLayout->addWidget(btnDeleteProj);
    projActionLayout->addStretch();

    projectActionWidget->setFixedHeight(50);
    ui->verticalLayoutDashboard->addWidget(projectActionWidget);
    projectActionWidget->setVisible(false);

    // MUTUAL EXCLUSION SELECTION LOGIC
    connect(ui->tableRecentEmployees, &QTableWidget::itemSelectionChanged, this, [this, projectActionWidget]() {
        bool hasEmpSelection = !ui->tableRecentEmployees->selectedItems().isEmpty();

        if (hasEmpSelection) {
            ui->tableRecentProjects->blockSignals(true);
            ui->tableRecentProjects->clearSelection();
            ui->tableRecentProjects->blockSignals(false);
            if (projectActionWidget) projectActionWidget->setVisible(false);
        }

        if (actionWidget) {
            actionWidget->setVisible(hasEmpSelection);
        }
    });

    connect(ui->tableRecentProjects, &QTableWidget::itemSelectionChanged, this, [this, projectActionWidget]() {
        bool hasProjSelection = !ui->tableRecentProjects->selectedItems().isEmpty();

        if (hasProjSelection) {
            ui->tableRecentEmployees->blockSignals(true);
            ui->tableRecentEmployees->clearSelection();
            ui->tableRecentEmployees->blockSignals(false);
            if (actionWidget) actionWidget->setVisible(false);
        }

        if (projectActionWidget) {
            projectActionWidget->setVisible(hasProjSelection);
        }

        onProjectSelectionChanged();
    });

    // CONNECT BUTTON ACTIONS
    connect(btnEditEmp, &QPushButton::clicked, this, &WorkSphereWindow::on_btnEditEmployee_clicked);
    connect(btnDeleteEmp, &QPushButton::clicked, this, &WorkSphereWindow::on_btnDeleteEmployee_clicked);
    connect(btnEditProj, &QPushButton::clicked, this, &WorkSphereWindow::on_btnEditProject_clicked);
    connect(btnDeleteProj, &QPushButton::clicked, this, &WorkSphereWindow::on_btnDeleteProject_clicked);

    // MOUNT EVENT FILTERS FOR DESELECTION ON CLICK OUTSIDE
    ui->tableRecentEmployees->viewport()->installEventFilter(this);
    ui->tableRecentProjects->viewport()->installEventFilter(this);
    ui->dashboardPage->installEventFilter(this);
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

    QString mainBgColor, textColor, cardBgColor, cardBorderColor;
    QString tableBg, tableHeaderBg, tableHeaderTextColor, tableBorder, tableSelectedBg, tableSelectedText;
    QString inputBg, inputBorder, inputFocusBorder;

    if (m_isDarkTheme) {
        mainBgColor = "#090d16";
        textColor = "#ffffff";
        cardBgColor = "#0f172a";
        cardBorderColor = "#1e293b";

        tableBg = "#0f172a";
        tableHeaderBg = "#1e293b";
        tableHeaderTextColor = "#94a3b8";
        tableBorder = "#1e293b";
        tableSelectedBg = "#1e293b";
        tableSelectedText = "#00d2ff";

        inputBg = "#0f172a";
        inputBorder = "#1e293b";
        inputFocusBorder = "#38bdf8";
    } else {
        mainBgColor = "#f8fafc";
        textColor = "#0f172a";
        cardBgColor = "#ffffff";
        cardBorderColor = "#cbd5e1";

        tableBg = "#ffffff";
        tableHeaderBg = "#f1f5f9";
        tableHeaderTextColor = "#475569";
        tableBorder = "#cbd5e1";
        tableSelectedBg = "#eff6ff";
        tableSelectedText = "#2563eb";

        inputBg = "#ffffff";
        inputBorder = "#cbd5e1";
        inputFocusBorder = "#2563eb";
    }

    this->setStyleSheet(QString(R"(
        QMainWindow, QWidget#centralwidget, QWidget#dashboardPage,
        QWidget#employeeFormPage, QWidget#projectFormPage, QWidget#settingsPage {
            background-color: %1;
            color: %2;
        }
    )").arg(mainBgColor, textColor));

    QString searchInputStyle = QString(R"(
        QLineEdit {
            background-color: %1;
            color: %2;
            padding: 8px 10px;
            border-radius: 6px;
            border: 1px solid %3;
            font-family: 'Segoe UI';
        }
        QLineEdit:focus {
            border: 1px solid %4;
        }
    )").arg(inputBg, textColor, inputBorder, inputFocusBorder);

    if (searchEmployeeInput) searchEmployeeInput->setStyleSheet(searchInputStyle);
    if (searchProjectInput) searchProjectInput->setStyleSheet(searchInputStyle);

    QString tableStyle = QString(R"(
        QTableWidget {
            background-color: %1;
            color: %2;
            gridline-color: %3;
            border: 1px solid %3;
            border-radius: 8px;
            font-family: 'Segoe UI';
        }
        QTableWidget::item {
            padding: 6px;
        }
        QTableWidget::item:selected {
            background-color: %4;
            color: %5;
        }
        QHeaderView::section {
            background-color: %6;
            color: %7;
            padding: 8px;
            border: none;
            border-bottom: 1px solid %3;
            font-weight: bold;
        }
    )").arg(tableBg, textColor, tableBorder, tableSelectedBg, tableSelectedText, tableHeaderBg, tableHeaderTextColor);

    if (ui->tableRecentEmployees) ui->tableRecentEmployees->setStyleSheet(tableStyle);
    if (ui->tableRecentProjects) ui->tableRecentProjects->setStyleSheet(tableStyle);
    if (ui->tableProjects) ui->tableProjects->setStyleSheet(tableStyle);

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

bool WorkSphereWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (watched == ui->tableRecentEmployees->viewport()) {
            if (!ui->tableRecentEmployees->itemAt(mouseEvent->pos())) {
                ui->tableRecentEmployees->clearSelection();
            }
        }
        else if (watched == ui->tableRecentProjects->viewport()) {
            if (!ui->tableRecentProjects->itemAt(mouseEvent->pos())) {
                ui->tableRecentProjects->clearSelection();
            }
        }
        else if (watched == ui->dashboardPage) {
            ui->tableRecentEmployees->clearSelection();
            ui->tableRecentProjects->clearSelection();
        }
    }

    return QMainWindow::eventFilter(watched, event);
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
        return w;
    };

    widgetEmployees = createExpandedWidget("Employees", QColor(76, 201, 240), 0, 100, ui->lblValueEmp, ui->verticalLayoutCard1);
    widgetProjects  = createExpandedWidget("Projects", QColor(13, 148, 136), 0, 20, ui->lblValueProj, ui->verticalLayoutCard2);
    widgetPayroll   = createExpandedWidget("Payroll", QColor(99, 102, 241), 0, 5000, ui->lblValuePayroll, ui->verticalLayoutCard3);

    ui->tableRecentEmployees->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->tableRecentEmployees->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableRecentProjects->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    updateDashboardStats();
}

void WorkSphereWindow::updateDashboardStats()
{
    std::vector<std::unique_ptr<Employee>> employees = m_database.loadEmployees();
    int empCount = employees.size();

    float totalPayrollUSD = 0.0f;
    for (const auto& emp : employees) {
        totalPayrollUSD += emp->getSalary();
        if (auto manager = dynamic_cast<const Manager*>(emp.get())) {
            totalPayrollUSD += manager->getBonus();
        }
    }

    int projCount = projectsList.size();

    QSettings settings("WorkSphere", "WorkSphereApp");
    QString currencySetting = settings.value("general/currency", "USD ($)").toString();

    float exchangeRate = 1.0f;
    QString currencySymbol = "$";

    if (currencySetting.contains("€") || currencySetting.contains("EUR")) {
        exchangeRate = 0.92f;
        currencySymbol = "€";
    }
    else if (currencySetting.contains("din") || currencySetting.contains("RSD")) {
        exchangeRate = 108.5f;
        currencySymbol = "din";
    }
    else if (currencySetting.contains("KM")) {
        exchangeRate = 1.8f;
        currencySymbol = "KM";
    }
    else {
        exchangeRate = 1.0f;
        currencySymbol = "$";
    }

    float finalPayroll = totalPayrollUSD * exchangeRate;

    if (widgetEmployees) {
        widgetEmployees->setValue(empCount, std::max(100, empCount + 25));
    }

    if (widgetProjects) {
        widgetProjects->setValue(projCount, std::max(20, projCount + 5));
    }

    if (widgetPayroll) {
        int payrollInt = static_cast<int>(finalPayroll);
        int maxPayroll = std::max(5000, static_cast<int>(finalPayroll * 1.3f));

        widgetPayroll->setValue(payrollInt, maxPayroll);
        widgetPayroll->setSuffix(" " + currencySymbol);
    }
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
        createEmployeeForm();
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
    QString subtitleColor = m_isDarkTheme ? "#94a3b8" : "#64748b";

    QWidget *headerContainer = new QWidget(ui->employeeFormPage);
    QVBoxLayout *headerLayout = new QVBoxLayout(headerContainer);
    headerLayout->setContentsMargins(0, 0, 0, 15);
    headerLayout->setSpacing(4);

    QLabel *labelFormTitle = new QLabel("Create Employee Profile", headerContainer);
    labelFormTitle->setAlignment(Qt::AlignCenter);
    labelFormTitle->setStyleSheet(QString("color: %1; font-size: 24px; font-weight: bold; font-family: 'Segoe UI';").arg(titleColor));

    QLabel *labelFormSubTitle = new QLabel("Enter details below to register a new worker or manager", headerContainer);
    labelFormSubTitle->setAlignment(Qt::AlignCenter);
    labelFormSubTitle->setStyleSheet(QString("color: %1; font-size: 13px; font-family: 'Segoe UI';").arg(subtitleColor));

    headerLayout->addWidget(labelFormTitle);
    headerLayout->addWidget(labelFormSubTitle);
    mainLayout->addWidget(headerContainer);

    auto createInputRow = [](QString labelText, QWidget *inputWidget, QWidget *parent) {
        QWidget *rowWidget = new QWidget(parent);
        QVBoxLayout *vBox = new QVBoxLayout(rowWidget);
        vBox->setContentsMargins(0, 0, 0, 0);
        vBox->setSpacing(6);

        QLabel *lbl = new QLabel(labelText, rowWidget);
        lbl->setObjectName("formLabel");

        inputWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        vBox->addWidget(lbl);
        vBox->addWidget(inputWidget);
        return rowWidget;
    };

    QWidget *formContainer = new QWidget(ui->employeeFormPage);
    formContainer->setMaximumWidth(750);
    formContainer->setMinimumWidth(580);

    QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
    formLayout->setSpacing(16);
    formLayout->setContentsMargins(20, 0, 20, 0);

    txtEmployeeName = new QLineEdit(formContainer);
    txtEmployeeName->setPlaceholderText("e.g. John Doe");
    QWidget *rowName = createInputRow("Full Name", txtEmployeeName, formContainer);

    txtEmployeeSalary = new QLineEdit(formContainer);
    txtEmployeeSalary->setPlaceholderText("e.g. 4500");
    QWidget *rowSalary = createInputRow("Salary ($)", txtEmployeeSalary, formContainer);

    comboEmployeeType = new QComboBox(formContainer);
    comboEmployeeType->addItem("Worker");
    comboEmployeeType->addItem("Manager");
    comboEmployeeType->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QWidget *rowType = createInputRow("Employee Type", comboEmployeeType, formContainer);

    QStackedWidget *stackedFields = new QStackedWidget(formContainer);
    stackedFields->setContentsMargins(0, 0, 0, 0);

    txtEmployeePosition = new QLineEdit();
    txtEmployeePosition->setPlaceholderText("e.g. Senior C++ Developer");
    QWidget *workerPage = createInputRow("Position", txtEmployeePosition, stackedFields);
    stackedFields->addWidget(workerPage);

    txtEmployeeBonus = new QLineEdit();
    txtEmployeeBonus->setPlaceholderText("e.g. 1200");
    QWidget *managerPage = createInputRow("Bonus ($)", txtEmployeeBonus, stackedFields);
    stackedFields->addWidget(managerPage);

    connect(comboEmployeeType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            stackedFields, &QStackedWidget::setCurrentIndex);

    QPushButton *btnCreate = new QPushButton("Create Employee", formContainer);
    btnCreate->setObjectName("createButton");
    btnCreate->setCursor(Qt::PointingHandCursor);

    QString formStyle = m_isDarkTheme ? R"(
        QLabel#formLabel {
            color: #ffffff;
            font-size: 13px;
            font-weight: bold;
            font-family: 'Segoe UI';
        }
        QLineEdit, QComboBox {
            background-color: #040711;
            color: #ffffff;
            border: 1px solid #1e293b;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            font-family: 'Segoe UI';
        }
        QLineEdit:focus, QComboBox:focus {
            border: 1px solid #00d2ff;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 30px;
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
        QLabel#formLabel {
            color: #0f172a;
            font-size: 13px;
            font-weight: bold;
            font-family: 'Segoe UI';
        }
        QLineEdit, QComboBox {
            background-color: #ffffff;
            color: #0f172a;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            font-family: 'Segoe UI';
        }
        QLineEdit:focus, QComboBox:focus {
            border: 1px solid #2563eb;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 30px;
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

    connect(btnCreate, &QPushButton::clicked, this, &WorkSphereWindow::on_btnCreateEmployee_clicked);
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
    QSettings settings("WorkSphere", "WorkSphereApp");
    QString dateFormat = settings.value("general/dateFormat", "dd.MM.yyyy").toString();

    editProjectDeadline = new QDateEdit(QDate::currentDate().addMonths(1), formContainer);
    editProjectDeadline->setCalendarPopup(true);
    editProjectDeadline->setDisplayFormat(dateFormat);
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

    QSqlQuery query;
    query.prepare("INSERT INTO Projects (Name, Deadline) VALUES (:name, :deadline)");
    query.bindValue(":name", QString::fromStdString(name));
    query.bindValue(":deadline", deadline);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", "Failed to add project to database.");
        return;
    }

    QMessageBox::information(this, "Success", "Project successfully added!");

    editProjectName->clear();
    editProjectDeadline->setDate(QDate::currentDate().addMonths(1));

    refreshProjectTable();
    updateDashboardStats();
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

    comboCurrency = new QComboBox(formContainer);
    comboCurrency->addItems({"EUR (€)", "USD ($)", "RSD (din)", "BAM (KM)"});
    comboCurrency->setCurrentText(savedCurrency);
    QWidget *rowCurrency = createInputRow("Application Currency", comboCurrency, formContainer);

    comboDateFormat = new QComboBox(formContainer);
    comboDateFormat->addItems({"dd.MM.yyyy", "yyyy-MM-dd", "MM/dd/yyyy"});
    comboDateFormat->setCurrentText(savedDateFormat);
    QWidget *rowDateFormat = createInputRow("Date Display Format", comboDateFormat, formContainer);

    comboTheme = new QComboBox(formContainer);
    comboTheme->addItems({"Dark Theme (Default)", "Light Theme"});
    comboTheme->setCurrentText(savedTheme);
    QWidget *rowTheme = createInputRow("Appearance Theme", comboTheme, formContainer);

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

    updateDashboardStats();

    refreshProjectTable();

    QMessageBox::information(this, "Settings Saved", "Your preferences have been successfully updated!");
}

void WorkSphereWindow::on_btnCreateEmployee_clicked()
{
    QString nameStr = txtEmployeeName->text().trimmed();
    QString salaryStr = txtEmployeeSalary->text().trimmed();
    int typeIndex = comboEmployeeType->currentIndex();

    if (nameStr.isEmpty() || salaryStr.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please fill in all required fields (Name and Salary)!");
        return;
    }

    bool salaryOk = false;
    float salary = salaryStr.toFloat(&salaryOk);
    if (!salaryOk || salary < 0) {
        QMessageBox::warning(this, "Validation Error", "Salary must be a valid non-negative number!");
        return;
    }

    int id = 0;

    if (typeIndex == 0) { // Worker
        QString positionStr = txtEmployeePosition->text().trimmed();

        if (positionStr.isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Please enter a position for the worker!");
            return;
        }

        Worker worker(positionStr.toStdString(), id, nameStr.toStdString(), salary);
        m_database.addWorker(worker);
    }
    else { // Manager
        QString bonusStr = txtEmployeeBonus->text().trimmed();

        if (bonusStr.isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Please enter a bonus for the manager!");
            return;
        }

        bool bonusOk = false;
        float bonus = bonusStr.toFloat(&bonusOk);
        if (!bonusOk || bonus < 0) {
            QMessageBox::warning(this, "Validation Error", "Bonus must be a valid non-negative number!");
            return;
        }

        Manager manager(id, nameStr.toStdString(), salary, bonus);
        m_database.addManager(manager);
    }

    QMessageBox::information(this, "Success", "Employee added successfully!");

    updateDashboardStats();
    refreshEmployeeTable();
}

void WorkSphereWindow::on_btnEditEmployee_clicked()
{
    int currentRow = ui->tableRecentEmployees->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Selection Error", "Please select an employee from the table to edit!");
        return;
    }

    QTableWidgetItem *idItem = ui->tableRecentEmployees->item(currentRow, 0);
    QTableWidgetItem *nameItem = ui->tableRecentEmployees->item(currentRow, 1);
    QTableWidgetItem *salaryItem = ui->tableRecentEmployees->item(currentRow, 2);
    QTableWidgetItem *typeItem = ui->tableRecentEmployees->item(currentRow, 3);
    QTableWidgetItem *positionBonusItem = ui->tableRecentEmployees->item(currentRow, 4);

    if (!idItem || !nameItem || !salaryItem) {
        QMessageBox::critical(this, "Error", "Invalid employee data in selected row.");
        return;
    }

    int empId = idItem->text().toInt();
    QString name = nameItem->text().trimmed();

    bool salaryOk = false;
    float salary = salaryItem->text().toFloat(&salaryOk);

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Employee name cannot be empty!");
        return;
    }

    if (!salaryOk || salary < 0) {
        QMessageBox::warning(this, "Validation Error", "Salary must be a valid non-negative number!");
        return;
    }

    QString empType = typeItem ? typeItem->text().trimmed() : "Worker";
    QString extraField = positionBonusItem ? positionBonusItem->text().trimmed() : "";

    try {
        if (empType.contains("Worker", Qt::CaseInsensitive)) {
            if (extraField.isEmpty()) {
                QMessageBox::warning(this, "Validation Error", "Position cannot be empty!");
                return;
            }
            Worker worker(extraField.toStdString(), empId, name.toStdString(), salary);
            m_database.updateEmployee(worker);
        } else { // Manager
            bool bonusOk = false;
            float bonus = extraField.toFloat(&bonusOk);
            if (!bonusOk || bonus < 0) {
                QMessageBox::warning(this, "Validation Error", "Bonus must be a valid non-negative number!");
                return;
            }
            Manager manager(empId, name.toStdString(), salary, bonus);
            m_database.updateEmployee(manager);
        }

        QMessageBox::information(this, "Success", "Employee details successfully updated!");
        refreshEmployeeTable();
        updateDashboardStats();
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Database Error", QString("Failed to update employee: %1").arg(e.what()));
    }
    catch (...) {
        QMessageBox::critical(this, "Database Error", "An error occurred while updating employee data.");
    }
}

void WorkSphereWindow::on_btnDeleteEmployee_clicked()
{
    QModelIndexList selectedIndexes = ui->tableRecentEmployees->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please select an employee from the table to delete!");
        return;
    }

    int row = selectedIndexes.first().row();

    QTableWidgetItem *idItem = ui->tableRecentEmployees->item(row, 0);
    if (!idItem) {
        QMessageBox::warning(this, "Error", "Could not retrieve the employee ID.");
        return;
    }

    int employeeId = idItem->text().toInt();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Deletion",
                                  "Are you sure you want to delete the selected employee?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        try {
            m_database.removeEmployee(employeeId);

            QMessageBox::information(this, "Success", "Employee deleted from database.");
            refreshEmployeeTable();
            updateDashboardStats();
        }
        catch (const std::exception& e) {
            QMessageBox::critical(this, "Database Error", QString("Failed to delete employee: %1").arg(e.what()));
        }
        catch (...) {
            QMessageBox::critical(this, "Database Error", "An unknown error occurred while deleting the employee.");
        }
    }
}

void WorkSphereWindow::onEmployeeCellChanged(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
}

void WorkSphereWindow::refreshEmployeeTable()
{
    std::vector<std::unique_ptr<Employee>> employees = m_database.loadEmployees();

    ui->tableRecentEmployees->setRowCount(0);

    for (const auto& emp : employees) {
        int row = ui->tableRecentEmployees->rowCount();
        ui->tableRecentEmployees->insertRow(row);

        // 1. ID - READ ONLY
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(emp->getId()));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        ui->tableRecentEmployees->setItem(row, 0, idItem);

        // 2. Name - Editable
        ui->tableRecentEmployees->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(emp->getName())));

        // 3. Salary - Editable
        ui->tableRecentEmployees->setItem(row, 2, new QTableWidgetItem(QString::number(emp->getSalary())));

        if (auto worker = dynamic_cast<const Worker*>(emp.get())) {
            // 4. Type ("Worker") - READ ONLY
            QTableWidgetItem *typeItem = new QTableWidgetItem("Worker");
            typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
            ui->tableRecentEmployees->setItem(row, 3, typeItem);

            // 5. Position - Editable
            ui->tableRecentEmployees->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(worker->getPosition())));
        }
        else if (auto manager = dynamic_cast<const Manager*>(emp.get())) {
            // 4. Type ("Manager") - READ ONLY
            QTableWidgetItem *typeItem = new QTableWidgetItem("Manager");
            typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
            ui->tableRecentEmployees->setItem(row, 3, typeItem);

            // 5. Bonus - Editable
            ui->tableRecentEmployees->setItem(row, 4, new QTableWidgetItem(QString::number(manager->getBonus())));
        }
    }
}

void WorkSphereWindow::refreshProjectTable()
{
    projectsList.clear();

    QSettings settings("WorkSphere", "WorkSphereApp");
    QString dateFormat = settings.value("general/dateFormat", "dd.MM.yyyy").toString();

    auto setupTable = [](QTableWidget *table) {
        if (!table) return;
        table->setRowCount(0);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    };

    setupTable(ui->tableRecentProjects);
    setupTable(ui->tableProjects);

    QSqlQuery query("SELECT Id, Name, Deadline FROM Projects");

    int row = 0;
    while (query.next()) {
        int id = query.value(0).toInt();
        std::string name = query.value(1).toString().toStdString();
        QDate deadline = query.value(2).toDate();

        projectsList.emplace_back(name, deadline);

        QString nameStr = QString::fromStdString(name);
        QString deadlineStr = formatDate(deadline);

        QDate currentDate = QDate::currentDate();
        QString statusText;
        QColor statusColor;

        if (currentDate > deadline) {
            statusText = "Overdue";
            statusColor = QColor("#ef4444");
        } else {
            statusText = "Active";
            statusColor = QColor("#10b981");
        }

        if (ui->tableRecentProjects) {
            ui->tableRecentProjects->insertRow(row);

            QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(id));
            idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);

            QTableWidgetItem *statusItemRecent = new QTableWidgetItem(statusText);
            statusItemRecent->setFlags(statusItemRecent->flags() & ~Qt::ItemIsEditable);
            statusItemRecent->setTextAlignment(Qt::AlignCenter);
            statusItemRecent->setForeground(statusColor);

            ui->tableRecentProjects->setItem(row, 0, idItem);
            ui->tableRecentProjects->setItem(row, 1, new QTableWidgetItem(nameStr));
            ui->tableRecentProjects->setItem(row, 2, new QTableWidgetItem(deadlineStr));
            ui->tableRecentProjects->setItem(row, 3, statusItemRecent);
        }

        if (ui->tableProjects) {
            ui->tableProjects->insertRow(row);

            // ID - READ ONLY
            QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(id));
            idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);

            // Name - EDITABLE
            QTableWidgetItem *nameItem = new QTableWidgetItem(nameStr);

            // Deadline - EDITABLE
            QTableWidgetItem *deadlineItem = new QTableWidgetItem(deadlineStr);

            // Status - READ ONLY
            QTableWidgetItem *statusItemMain = new QTableWidgetItem(statusText);
            statusItemMain->setFlags(statusItemMain->flags() & ~Qt::ItemIsEditable);
            statusItemMain->setTextAlignment(Qt::AlignCenter);
            statusItemMain->setForeground(statusColor);

            ui->tableProjects->setItem(row, 0, idItem);
            ui->tableProjects->setItem(row, 1, nameItem);
            ui->tableProjects->setItem(row, 2, deadlineItem);
            ui->tableProjects->setItem(row, 3, statusItemMain);

            if (ui->tableProjects->columnCount() >= 5) {
                addProjectActionButtons(row, id);
            }
        }

        row++;
    }
    onProjectSelectionChanged();
}

void WorkSphereWindow::addProjectActionButtons(int row, int projectId)
{
    QWidget *btnWidget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(btnWidget);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(6);

    QPushButton *btnEdit = new QPushButton("Edit", btnWidget);
    QPushButton *btnDelete = new QPushButton("Delete", btnWidget);

    btnEdit->setStyleSheet(
        "QPushButton {"
        "   background-color: #6C5CE7;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 4px 8px;"
        "}"
        "QPushButton:hover { background-color: #5A4AD1; }"
        );

    btnDelete->setStyleSheet(
        "QPushButton {"
        "   background-color: #E17055;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 4px 8px;"
        "}"
        "QPushButton:hover { background-color: #D15B40; }"
        );

    layout->addWidget(btnEdit);
    layout->addWidget(btnDelete);
    btnWidget->setLayout(layout);

    ui->tableProjects->setCellWidget(row, 3, btnWidget);

    connect(btnEdit, &QPushButton::clicked, this, [this, projectId]() {
        on_editProject_clicked(projectId);
    });

    connect(btnDelete, &QPushButton::clicked, this, [this, projectId]() {
        on_deleteProject_clicked(projectId);
    });
}

void WorkSphereWindow::on_deleteProject_clicked(int projectId)
{
    QMessageBox::StandardButton confirm = QMessageBox::question(
        this,
        "Confirm Deletion",
        "Are you sure you want to delete this project?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (confirm == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM Projects WHERE Id = :id");
        query.bindValue(":id", projectId);

        if (query.exec()) {
            QMessageBox::information(this, "Success", "Project deleted successfully!");
            refreshProjectTable();
            updateDashboardStats();
        } else {
            QMessageBox::critical(this, "Database Error", "Failed to delete project from database.");
        }
    }
}

void WorkSphereWindow::on_editProject_clicked(int projectId)
{
    QSqlQuery query;
    query.prepare("SELECT Name, Deadline FROM Projects WHERE Id = :id");
    query.bindValue(":id", projectId);

    if (!query.exec() || !query.next()) return;

    QString currentName = query.value(0).toString();
    QDate currentDeadline = query.value(1).toDate();

    QDialog dialog(this);
    dialog.setWindowTitle("Edit Project");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLineEdit *editName = new QLineEdit(currentName, &dialog);
    QDateEdit *editDeadline = new QDateEdit(currentDeadline, &dialog);
    editDeadline->setCalendarPopup(true);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog
        );

    layout->addWidget(new QLabel("Project Name:"));
    layout->addWidget(editName);
    layout->addWidget(new QLabel("Deadline:"));
    layout->addWidget(editDeadline);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString newName = editName->text().trimmed();
        QDate newDeadline = editDeadline->date();

        if (newName.isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Project name cannot be empty!");
            return;
        }

        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE Projects SET Name = :name, Deadline = :deadline WHERE Id = :id");
        updateQuery.bindValue(":name", newName);
        updateQuery.bindValue(":deadline", newDeadline);
        updateQuery.bindValue(":id", projectId);

        if (updateQuery.exec()) {
            QMessageBox::information(this, "Success", "Project updated successfully!");
            refreshProjectTable();
        } else {
            QMessageBox::critical(this, "Database Error", "Failed to update project in database.");
        }
    }
}

QWidget* WorkSphereWindow::createInputRow(const QString &labelText, QWidget *inputField, QWidget *parent)
{
    QWidget *rowWidget = new QWidget(parent ? parent : this);
    QHBoxLayout *layout = new QHBoxLayout(rowWidget);
    layout->setContentsMargins(0, 0, 0, 0);

    QLabel *label = new QLabel(labelText, rowWidget);
    label->setMinimumWidth(120);

    layout->addWidget(label);
    layout->addWidget(inputField);

    return rowWidget;
}

void WorkSphereWindow::on_btnDeleteProject_clicked()
{
    int currentRow = ui->tableRecentProjects->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Selection Error", "Please select a project from the table to delete.");
        return;
    }

    QTableWidgetItem *idItem = ui->tableRecentProjects->item(currentRow, 0);
    QTableWidgetItem *nameItem = ui->tableRecentProjects->item(currentRow, 1);

    if (!idItem) return;

    int projId = idItem->text().toInt();
    QString projName = nameItem ? nameItem->text() : "selected project";

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Deletion",
        QString("Are you sure you want to delete the project: %1 (ID: %2)?").arg(projName).arg(projId),
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM Projects WHERE Id = :id");
        query.bindValue(":id", projId);

        if (query.exec()) {
            QMessageBox::information(this, "Success", "Project successfully deleted!");
            refreshProjectTable();
            updateDashboardStats();
        } else {
            QMessageBox::critical(this, "Database Error", "Failed to delete project: " + query.lastError().text());
        }
    }
}

void WorkSphereWindow::on_btnEditProject_clicked()
{
    int currentRow = ui->tableRecentProjects->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Selection Error", "Please select a project from the table to edit.");
        return;
    }

    QTableWidgetItem *idItem = ui->tableRecentProjects->item(currentRow, 0);
    QTableWidgetItem *nameItem = ui->tableRecentProjects->item(currentRow, 1);
    QTableWidgetItem *deadlineItem = ui->tableRecentProjects->item(currentRow, 2);

    if (!idItem || !nameItem || !deadlineItem) {
        QMessageBox::critical(this, "Error", "Project data could not be retrieved from the table.");
        return;
    }

    int projId = idItem->text().toInt();
    QString projName = nameItem->text().trimmed();
    QString deadlineStr = deadlineItem->text().trimmed();

    if (projName.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Project name cannot be empty!");
        return;
    }

    QSettings settings("WorkSphere", "WorkSphereApp");
    QString dateFormat = settings.value("general/dateFormat", "dd.MM.yyyy").toString();
    QDate deadline = QDate::fromString(deadlineStr, dateFormat);

    if (!deadline.isValid()) {
        deadline = QDate::fromString(deadlineStr, "yyyy-MM-dd");
    }

    if (!deadline.isValid()) {
        QMessageBox::warning(this, "Validation Error", QString("Invalid date format! Expected format is %1").arg(dateFormat));
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE Projects SET Name = :name, Deadline = :deadline WHERE Id = :id");
    query.bindValue(":name", projName);
    query.bindValue(":deadline", deadline);
    query.bindValue(":id", projId);

    if (query.exec()) {
        QMessageBox::information(this, "Success", "Project details successfully updated!");
        refreshProjectTable();
    } else {
        QMessageBox::critical(this, "Database Error", "Failed to update project: " + query.lastError().text());
    }
}

void WorkSphereWindow::onProjectSelectionChanged()
{
    bool hasMainSel = (ui->tableProjects && !ui->tableProjects->selectedItems().isEmpty());
    bool hasRecentSel = (ui->tableRecentProjects && !ui->tableRecentProjects->selectedItems().isEmpty());

    bool hasSelection = hasMainSel || hasRecentSel;

    if (ui->btnEditProject) {
        ui->btnEditProject->setVisible(hasSelection);
    }
    if (ui->btnDeleteProject) {
        ui->btnDeleteProject->setVisible(hasSelection);
    }
}

QString WorkSphereWindow::formatDate(const QDate &date) const {
    QSettings settings("WorkSphere", "WorkSphereApp");
    QString dateFormat = settings.value("general/dateFormat", "dd.MM.yyyy").toString();
    return date.toString(dateFormat);
}