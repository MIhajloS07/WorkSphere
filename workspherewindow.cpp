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
#include <QHeaderView>
#include <QSqlQuery>
#include <QSqlError>

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

    searchEmployeeInput = new QLineEdit(this);
    searchEmployeeInput->setPlaceholderText("Search employees by ID or name...");
    searchEmployeeInput->setStyleSheet(
        "QLineEdit {"
        "   background-color: #0f172a;"
        "   color: #ffffff;"
        "   padding: 8px 10px;"
        "   border-radius: 6px;"
        "   border: 1px solid #1e293b;"
        "   font-family: 'Segoe UI';"
        "}"
        "QLineEdit:focus {"
        "   border: 1px solid #38bdf8;"
        "}"
        );

    QString searchStyle =
        "QLineEdit {"
        "   background-color: #0f172a;"
        "   color: #ffffff;"
        "   padding: 8px 10px;"
        "   border-radius: 6px;"
        "   border: 1px solid #1e293b;"
        "   font-family: 'Segoe UI';"
        "}"
        "QLineEdit:focus {"
        "   border: 1px solid #38bdf8;"
        "}";

    searchProjectInput = new QLineEdit(this);
    searchProjectInput->setPlaceholderText("Search projects by name...");
    searchProjectInput->setStyleSheet(searchStyle);

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

    ui->tableRecentEmployees->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableRecentEmployees->setSelectionMode(QAbstractItemView::SingleSelection);

    actionWidget = new QWidget(this);
    QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
    actionLayout->setContentsMargins(0, 10, 0, 0);

    QPushButton *btnEdit = new QPushButton("Edit", this);
    QPushButton *btnDelete = new QPushButton("Delete", this);

    QString btnEditStyle = "QPushButton { background-color: #3b82f6; color: white; border-radius: 6px; padding: 8px 20px; font-weight: bold; font-family: 'Segoe UI'; }"
                           "QPushButton:hover { background-color: #2563eb; }";
    btnEdit->setStyleSheet(btnEditStyle);
    btnEdit->setCursor(Qt::PointingHandCursor);

    QString btnDeleteStyle = "QPushButton { background-color: #ef4444; color: white; border-radius: 6px; padding: 8px 20px; font-weight: bold; font-family: 'Segoe UI'; }"
                             "QPushButton:hover { background-color: #dc2626; }";
    btnDelete->setStyleSheet(btnDeleteStyle);
    btnDelete->setCursor(Qt::PointingHandCursor);

    actionLayout->addWidget(btnEdit);
    actionLayout->addWidget(btnDelete);
    actionLayout->addStretch();

    ui->verticalLayoutDashboard->addWidget(actionWidget);

    actionWidget->setVisible(false);

    connect(ui->tableRecentEmployees, &QTableWidget::itemSelectionChanged, this, [this]() {
        bool isSelected = !ui->tableRecentEmployees->selectedItems().isEmpty();
        if (actionWidget) {
            actionWidget->setVisible(isSelected);
        }
    });

    connect(btnDelete, &QPushButton::clicked, this, &WorkSphereWindow::on_btnDeleteEmployee_clicked);

    ui->tableRecentEmployees->viewport()->installEventFilter(this);
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

bool WorkSphereWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (watched == ui->tableRecentEmployees->viewport()) {
            QModelIndex index = ui->tableRecentEmployees->indexAt(mouseEvent->pos());

            if (!index.isValid()) {
                ui->tableRecentEmployees->clearSelection();
                return true;
            }

            if (ui->tableRecentEmployees->selectionModel()->isSelected(index)) {
                ui->tableRecentEmployees->clearSelection();
                return true;
            }
        }
        if (watched == ui->dashboardPage) {
            QWidget *child = ui->dashboardPage->childAt(mouseEvent->pos());

            bool clickedOnTable = (child == ui->tableRecentEmployees || ui->tableRecentEmployees->isAncestorOf(child));
            bool clickedOnActions = (actionWidget && (child == actionWidget || actionWidget->isAncestorOf(child)));

            if (!clickedOnTable && !clickedOnActions) {
                ui->tableRecentEmployees->clearSelection();
            }
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
    ui->tableRecentEmployees->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

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
        widgetPayroll->setSuffix(" " + currencySymbol); // Postavlja tačnu valutu i pokreće osvežavanje
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
    QString labelColor = m_isDarkTheme ? "#94a3b8" : "#475569";

    QLabel *labelFormTitle = new QLabel("Add New Employee", ui->employeeFormPage);
    labelFormTitle->setAlignment(Qt::AlignCenter);
    labelFormTitle->setStyleSheet(QString("color: %1; font-size: 22px; font-weight: bold; font-family: 'Segoe UI'; margin-bottom: 25px;").arg(titleColor));
    mainLayout->addWidget(labelFormTitle);

    QWidget *formContainer = new QWidget(ui->employeeFormPage);
    formContainer->setMaximumWidth(750);
    formContainer->setMinimumWidth(580);
    QVBoxLayout *formLayout = new QVBoxLayout(formContainer);
    formLayout->setSpacing(16);
    formLayout->setContentsMargins(20, 0, 20, 0);

    txtEmployeeName = new QLineEdit(formContainer);
    txtEmployeeName->setPlaceholderText("Enter employee name...");
    QWidget *rowName = createInputRow("Full Name", txtEmployeeName, formContainer);

    txtEmployeeSalary = new QLineEdit(formContainer);
    txtEmployeeSalary->setPlaceholderText("Enter salary amount...");
    QWidget *rowSalary = createInputRow("Salary ($)", txtEmployeeSalary, formContainer);

    comboEmployeeType = new QComboBox(formContainer);
    comboEmployeeType->addItem("Worker");
    comboEmployeeType->addItem("Manager");
    QWidget *rowType = createInputRow("Employee Type", comboEmployeeType, formContainer);

    QStackedWidget *stackedFields = new QStackedWidget(formContainer);

    txtEmployeePosition = new QLineEdit();
    txtEmployeePosition->setPlaceholderText("Enter worker position...");
    QWidget *workerPage = createInputRow("Position", txtEmployeePosition, stackedFields);
    stackedFields->addWidget(workerPage);

    txtEmployeeBonus = new QLineEdit();
    txtEmployeeBonus->setPlaceholderText("Enter manager bonus...");
    QWidget *managerPage = createInputRow("Bonus ($)", txtEmployeeBonus, stackedFields);
    stackedFields->addWidget(managerPage);

    connect(comboEmployeeType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            stackedFields, &QStackedWidget::setCurrentIndex);

    QPushButton *btnCreate = new QPushButton("Create Employee", formContainer);
    btnCreate->setObjectName("createButton");
    btnCreate->setCursor(Qt::PointingHandCursor);

    QString formStyle = m_isDarkTheme ? R"(
        QLineEdit, QComboBox, QStackedWidget {
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
        QLineEdit, QComboBox, QStackedWidget {
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

    updateDashboardStats();

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
        m_database.addWorker(worker); // <--- Writes to SQL Server
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
        m_database.addManager(manager); // <--- Writes to SQL Server
    }

    // Success message
    QMessageBox::information(this, "Success", "Employee added successfully!");

    updateDashboardStats();
    refreshEmployeeTable();
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
            updateDashboardStats(); // Da se osveže i statistike na dashboard-u
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

    // ui->btnSaveEmp->setEnabled(true);
}

void WorkSphereWindow::refreshEmployeeTable()
{
    std::vector<std::unique_ptr<Employee>> employees = m_database.loadEmployees();

    ui->tableRecentEmployees->setRowCount(0);

    for (const auto& emp : employees) {
        int row = ui->tableRecentEmployees->rowCount();
        ui->tableRecentEmployees->insertRow(row);

        ui->tableRecentEmployees->setItem(row, 0, new QTableWidgetItem(QString::number(emp->getId())));
        ui->tableRecentEmployees->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(emp->getName())));
        ui->tableRecentEmployees->setItem(row, 2, new QTableWidgetItem(QString::number(emp->getSalary())));

        if (auto worker = dynamic_cast<const Worker*>(emp.get())) {
            ui->tableRecentEmployees->setItem(row, 3, new QTableWidgetItem("Worker"));
            ui->tableRecentEmployees->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(worker->getPosition())));
        }
        else if (auto manager = dynamic_cast<const Manager*>(emp.get())) {
            ui->tableRecentEmployees->setItem(row, 3, new QTableWidgetItem("Manager"));
            ui->tableRecentEmployees->setItem(row, 4, new QTableWidgetItem(QString::number(manager->getBonus())));
        }
    }
}

void WorkSphereWindow::refreshProjectTable()
{
    projectsList.clear();
    ui->tableRecentProjects->setRowCount(0);
    if (ui->tableProjects) {
        ui->tableProjects->setRowCount(0);
    }

    // Load configured date format
    QSettings settings("WorkSphere", "WorkSphereApp");
    QString dateFormat = settings.value("general/dateFormat", "dd.MM.yyyy").toString();

    QSqlQuery query("SELECT Name, Deadline FROM Projects");
    while (query.next()) {
        std::string name = query.value(0).toString().toStdString();
        QDate deadline = query.value(1).toDate();

        projectsList.emplace_back(name, deadline);
    }

    int row = 0;
    for (const auto &proj : projectsList) {
        QString nameStr = QString::fromStdString(proj.getName());
        QString deadlineStr = proj.getDeadline().toString(dateFormat);

        ui->tableRecentProjects->insertRow(row);
        ui->tableRecentProjects->setItem(row, 0, new QTableWidgetItem(nameStr));
        ui->tableRecentProjects->setItem(row, 1, new QTableWidgetItem(deadlineStr));

        if (ui->tableProjects) {
            ui->tableProjects->insertRow(row);
            ui->tableProjects->setItem(row, 0, new QTableWidgetItem(nameStr));
            ui->tableProjects->setItem(row, 1, new QTableWidgetItem(deadlineStr));
        }

        row++;
    }
}

void WorkSphereWindow::addProjectActionButtons(int row, int projectId)
{
    QWidget *actionWidget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(actionWidget);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(6);

    QPushButton *btnEdit = new QPushButton("Edit", actionWidget);
    QPushButton *btnDelete = new QPushButton("Delete", actionWidget);

    // Stilovi sa novim bojama specifičnim za projekte
    btnEdit->setStyleSheet(
        "QPushButton {"
        "   background-color: #6C5CE7;" // Ljubičasta / Indigo za projekte
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 4px 8px;"
        "}"
        "QPushButton:hover { background-color: #5A4AD1; }"
        );

    btnDelete->setStyleSheet(
        "QPushButton {"
        "   background-color: #E17055;" // Narandžasto-crvena za brisanje projekta
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 4px 8px;"
        "}"
        "QPushButton:hover { background-color: #D15B40; }"
        );

    layout->addWidget(btnEdit);
    layout->addWidget(btnDelete);
    actionWidget->setLayout(layout);

    ui->tableProjects->setCellWidget(row, 2, actionWidget);

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
        "Potvrda brisanja",
        "Da li ste sigurni da želite da obrišete ovaj projekat?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (confirm == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM Projects WHERE Id = :id");
        query.bindValue(":id", projectId);

        if (query.exec()) {
            QMessageBox::information(this, "Uspeh", "Projekat je uspešno obrisan!");
            refreshProjectTable();
        } else {
            QMessageBox::critical(this, "Greška", "Greška pri brisanju projekta.");
        }
    }
}

void WorkSphereWindow::on_editProject_clicked(int projectId)
{
    // Dobavljanje trenutnih podataka iz baze
    QSqlQuery query;
    query.prepare("SELECT Name, Deadline FROM Projects WHERE Id = :id");
    query.bindValue(":id", projectId);

    if (!query.exec() || !query.next()) return;

    QString currentName = query.value(0).toString();
    QDate currentDeadline = query.value(1).toDate();

    // Dijalog za izmenu
    QDialog dialog(this);
    dialog.setWindowTitle("Izmena Projekta");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLineEdit *editName = new QLineEdit(currentName, &dialog);
    QDateEdit *editDeadline = new QDateEdit(currentDeadline, &dialog);
    editDeadline->setCalendarPopup(true);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog
        );

    layout->addWidget(new QLabel("Naziv projekta:"));
    layout->addWidget(editName);
    layout->addWidget(new QLabel("Rok (Deadline):"));
    layout->addWidget(editDeadline);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString newName = editName->text().trimmed();
        QDate newDeadline = editDeadline->date();

        if (newName.isEmpty()) {
            QMessageBox::warning(this, "Greška", "Naziv ne može biti prazan!");
            return;
        }

        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE Projects SET Name = :name, Deadline = :deadline WHERE Id = :id");
        updateQuery.bindValue(":name", newName);
        updateQuery.bindValue(":deadline", newDeadline);
        updateQuery.bindValue(":id", projectId);

        if (updateQuery.exec()) {
            QMessageBox::information(this, "Uspeh", "Projekat uspešno izmenjen!");
            refreshProjectTable();
        } else {
            QMessageBox::critical(this, "Greška", "Greška pri ažuriranju baze.");
        }
    }
}

void WorkSphereWindow::on_btnAddProject_clicked()
{
    // Logika za prelazak na formu ili dodavanje projekta
}

void WorkSphereWindow::on_btnCancelProject_clicked()
{
    // Logika za otkazivanje / povratak
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