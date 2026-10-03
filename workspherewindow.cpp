#include "workspherewindow.h"
#include "ui_workspherewindow.h"
#include "widgetmodels/CircularProgressWidget.h"
#include "models/Worker.h"
#include "models/Manager.h"
#include <QTimer>
#include <QTime>
#include <QDate>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QFrame>
#include <QStackedWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialog>
#include <QSettings>
#include <QMouseEvent>
#include <QSqlQuery>
#include <QSqlError>
#include <QDialogButtonBox>
#include <QCoreApplication>
#include <QPixmap>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QMap>
#include <QStringList>
#include <algorithm>
#include <memory>
#include <vector>

namespace {

struct Palette
{
    QString bg, sidebar, card, border, input, inputBorder;
    QString text, muted, subtle;
    QString primary, primaryHover, accent;
    QString headerBg, rowAlt, selBg, selText, scroll;
    QString navActiveBg, navActiveText, navText, navHoverBg;
    QString success, danger;
};

Palette makePalette(bool dark)
{
    Palette p;
    if (dark) {
        p.bg = "#0b1120";
        p.sidebar = "#080d1a";
        p.card = "#111a2e";
        p.border = "#1f2b45";
        p.input = "#0b1120";
        p.inputBorder = "#2a3957";
        p.text = "#e6edf8";
        p.muted = "#8b9bb8";
        p.subtle = "#5d6d8c";
        p.primary = "#2563eb";
        p.primaryHover = "#3b82f6";
        p.accent = "#38bdf8";
        p.headerBg = "#0e1729";
        p.rowAlt = "#0f182b";
        p.selBg = "#1a2b4d";
        p.selText = "#93c5fd";
        p.scroll = "#2a3a5c";
        p.navActiveBg = "rgba(56, 189, 248, 14%)";
        p.navActiveText = "#7dd3fc";
        p.navText = "#8b9bb8";
        p.navHoverBg = "#111a2e";
        p.success = "#22c55e";
        p.danger = "#ef4444";
    } else {
        p.bg = "#f1f5f9";
        p.sidebar = "#ffffff";
        p.card = "#ffffff";
        p.border = "#e2e8f0";
        p.input = "#f8fafc";
        p.inputBorder = "#cbd5e1";
        p.text = "#0f172a";
        p.muted = "#64748b";
        p.subtle = "#94a3b8";
        p.primary = "#2563eb";
        p.primaryHover = "#1d4ed8";
        p.accent = "#2563eb";
        p.headerBg = "#f8fafc";
        p.rowAlt = "#f8fafc";
        p.selBg = "#e0ecff";
        p.selText = "#1d4ed8";
        p.scroll = "#cbd5e1";
        p.navActiveBg = "rgba(37, 99, 235, 10%)";
        p.navActiveText = "#1d4ed8";
        p.navText = "#64748b";
        p.navHoverBg = "#f1f5f9";
        p.success = "#15803d";
        p.danger = "#dc2626";
    }
    return p;
}

// One stylesheet for the whole window. Tokens look like %name%.
QString buildStyleSheet(const Palette &p)
{
    QString css = QString::fromLatin1(R"QSS(
QMainWindow, QWidget#centralwidget { background-color: %bg%; }
QStackedWidget, QWidget#dashboardPage, QWidget#employeeFormPage, QWidget#projectFormPage,
QWidget#assignmentsPage, QWidget#settingsPage { background-color: %bg%; }

QLabel { color: %text%; background: transparent; font-family: 'Segoe UI'; font-size: 14px; }
QToolTip { background-color: %card%; color: %text%; border: 1px solid %border%; padding: 4px 8px; }

/* ---------- sidebar ---------- */
QFrame#sidebarWidget { background-color: %sidebar%; border: none; border-right: 1px solid %border%; }
QLabel#labelAppName { font-size: 24px; font-weight: 700; }
QLabel#clockLabel { color: %accent%; font-size: 18px; font-weight: 600; }
QLabel#labelLogoIcon { background-color: %card%; border-radius: 65px; }
QFrame#lineSidebar, QFrame#lineDashboard { background-color: %border%; border: none; min-height: 1px; max-height: 1px; }

/* ---------- dashboard ---------- */
QLabel#labelDashboardTitle { font-size: 26px; font-weight: 700; }
QFrame#card1, QFrame#card2, QFrame#card3 { background-color: %card%; border: 1px solid %border%; border-radius: 14px; }
QLabel#lblCardTitle1, QLabel#lblCardTitle2, QLabel#lblCardTitle3 { color: %muted%; font-size: 13px; font-weight: 600; }

/* ---------- tables ---------- */
QTableWidget {
    background-color: %card%; alternate-background-color: %rowAlt%; color: %text%;
    border: 1px solid %border%; border-radius: 12px; gridline-color: %border%;
    selection-background-color: %selBg%; selection-color: %selText%;
    outline: 0; font-family: 'Segoe UI'; font-size: 13px;
}
QTableWidget::item { padding: 4px 10px; border: none; }
QTableWidget::item:selected { background-color: %selBg%; color: %selText%; }
QTableWidget QLineEdit { padding: 2px 6px; border: 1px solid %accent%; border-radius: 6px; min-height: 0px; }
QHeaderView { background-color: transparent; }
QHeaderView::section {
    background-color: %headerBg%; color: %muted%; padding: 8px 10px; border: none;
    border-bottom: 1px solid %border%; font-family: 'Segoe UI'; font-size: 12px; font-weight: 700;
}
QHeaderView::section:first { border-top-left-radius: 11px; }
QHeaderView::section:last { border-top-right-radius: 11px; }
QTableCornerButton::section { background-color: %headerBg%; border: none; }

/* ---------- scrollbars ---------- */
QScrollBar:vertical { background: transparent; width: 10px; margin: 2px; }
QScrollBar::handle:vertical { background: %scroll%; border-radius: 4px; min-height: 30px; }
QScrollBar:horizontal { background: transparent; height: 10px; margin: 2px; }
QScrollBar::handle:horizontal { background: %scroll%; border-radius: 4px; min-width: 30px; }
QScrollBar::add-line, QScrollBar::sub-line { width: 0px; height: 0px; }
QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }

/* ---------- inputs ---------- */
QLineEdit, QDateEdit, QComboBox {
    background-color: %input%; color: %text%; border: 1px solid %inputBorder%; border-radius: 10px;
    padding: 9px 14px; min-height: 20px; font-family: 'Segoe UI'; font-size: 14px;
    selection-background-color: %primary%; selection-color: #ffffff;
}
QLineEdit:hover, QDateEdit:hover, QComboBox:hover { border-color: %subtle%; }
QLineEdit:focus, QDateEdit:focus, QComboBox:focus, QComboBox:on { border: 1px solid %accent%; }
QComboBox::drop-down, QDateEdit::drop-down { border: none; width: 30px; }
QComboBox QAbstractItemView {
    background-color: %card%; color: %text%; border: 1px solid %border%;
    selection-background-color: %selBg%; selection-color: %selText%; outline: 0;
}
QCalendarWidget QWidget { background-color: %card%; color: %text%; }
QCalendarWidget QToolButton { background: transparent; color: %text%; padding: 6px; border-radius: 6px; }
QCalendarWidget QToolButton:hover { background-color: %selBg%; }
QCalendarWidget QAbstractItemView:enabled { background-color: %card%; color: %text%; selection-background-color: %primary%; selection-color: #ffffff; }
QCalendarWidget QAbstractItemView:disabled { color: %subtle%; }

/* ---------- cards + labels ---------- */
QFrame#formCard { background-color: %card%; border: 1px solid %border%; border-radius: 18px; }
QLabel#cardTitle { font-size: 22px; font-weight: 700; }
QLabel#cardSubtitle { color: %muted%; font-size: 13px; }
QFrame#cardDivider { background-color: %border%; border: none; min-height: 1px; max-height: 1px; }
QLabel#sectionLabel { color: %accent%; font-size: 13px; font-weight: 700; }
QLabel#fieldLabel { color: %muted%; font-size: 12px; font-weight: 600; }
QFrame#teamBox { background-color: %input%; border: 1px solid %border%; border-radius: 12px; }
QLabel#teamTitle { color: %muted%; font-size: 12px; font-weight: 600; }
QLabel#teamText { font-size: 14px; }

/* ---------- buttons ---------- */
QPushButton#createButton {
    background-color: %primary%; color: #ffffff; border: none; border-radius: 10px;
    padding: 12px 26px; font-family: 'Segoe UI'; font-size: 14px; font-weight: 700;
}
QPushButton#createButton:hover { background-color: %primaryHover%; }
QPushButton#createButton:pressed { background-color: %primaryHover%; padding-top: 13px; padding-bottom: 11px; }
QPushButton#secondaryButton {
    background-color: transparent; color: %text%; border: 1px solid %inputBorder%; border-radius: 10px;
    padding: 11px 22px; font-family: 'Segoe UI'; font-size: 14px; font-weight: 600;
}
QPushButton#secondaryButton:hover { background-color: %selBg%; border-color: %accent%; }
QPushButton#btnEdit {
    background-color: %primary%; color: #ffffff; border: none; border-radius: 8px;
    padding: 8px 22px; font-family: 'Segoe UI'; font-size: 13px; font-weight: 700;
}
QPushButton#btnEdit:hover { background-color: %primaryHover%; }
QPushButton#btnDelete {
    background-color: transparent; color: %danger%; border: 1px solid %danger%; border-radius: 8px;
    padding: 7px 22px; font-family: 'Segoe UI'; font-size: 13px; font-weight: 700;
}
QPushButton#btnDelete:hover { background-color: %danger%; color: #ffffff; }

/* ---------- dialogs / message boxes ---------- */
QDialog, QMessageBox { background-color: %card%; }
QDialog QLabel, QMessageBox QLabel { color: %text%; }
QDialogButtonBox QPushButton, QMessageBox QPushButton {
    background-color: %primary%; color: #ffffff; border: none; border-radius: 8px;
    padding: 8px 20px; min-width: 72px; font-family: 'Segoe UI'; font-size: 13px; font-weight: 600;
}
QDialogButtonBox QPushButton:hover, QMessageBox QPushButton:hover { background-color: %primaryHover%; }
)QSS");

    const QMap<QString, QString> tokens = {
        {"bg", p.bg}, {"sidebar", p.sidebar}, {"card", p.card}, {"border", p.border},
        {"input", p.input}, {"inputBorder", p.inputBorder}, {"text", p.text},
        {"muted", p.muted}, {"subtle", p.subtle}, {"primary", p.primary},
        {"primaryHover", p.primaryHover}, {"accent", p.accent}, {"headerBg", p.headerBg},
        {"rowAlt", p.rowAlt}, {"selBg", p.selBg}, {"selText", p.selText},
        {"scroll", p.scroll}, {"danger", p.danger}
    };
    for (auto it = tokens.constBegin(); it != tokens.constEnd(); ++it) {
        css.replace(QString("%") + it.key() + QString("%"), it.value());
    }
    return css;
}

QString navButtonStyle(bool active, const Palette &p)
{
    const QString bg = active ? p.navActiveBg : QString("transparent");
    const QString fg = active ? p.navActiveText : p.navText;
    const QString hoverBg = active ? p.navActiveBg : p.navHoverBg;
    const QString hoverFg = active ? p.navActiveText : p.text;
    const QString weight = active ? QString("700") : QString("500");

    return QString("QPushButton { background-color: %1; color: %2; border: none; border-radius: 10px; "
                   "text-align: left; padding: 13px 20px; font-family: 'Segoe UI'; font-size: 15px; font-weight: %3; } "
                   "QPushButton:hover { background-color: %4; color: %5; }")
        .arg(bg, fg, weight, hoverBg, hoverFg);
}

QString configuredDateFormat()
{
    QSettings settings("WorkSphere", "WorkSphereApp");
    return settings.value("general/dateFormat", "dd.MM.yyyy").toString();
}

// A centered card used by every form page.
QFrame *createCard(QWidget *parent)
{
    QFrame *card = new QFrame(parent);
    card->setObjectName("formCard");
    card->setMinimumWidth(520);
    card->setMaximumWidth(660);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    return card;
}

QVBoxLayout *createCardLayout(QFrame *card)
{
    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(34, 30, 34, 30);
    layout->setSpacing(18);
    return layout;
}

void addCardHeader(QVBoxLayout *layout, QWidget *parent, const QString &title, const QString &subtitle)
{
    QLabel *labelTitle = new QLabel(title, parent);
    labelTitle->setObjectName("cardTitle");

    QLabel *labelSubtitle = new QLabel(subtitle, parent);
    labelSubtitle->setObjectName("cardSubtitle");
    labelSubtitle->setWordWrap(true);

    QFrame *divider = new QFrame(parent);
    divider->setObjectName("cardDivider");

    layout->addWidget(labelTitle);
    layout->addWidget(labelSubtitle);
    layout->addSpacing(2);
    layout->addWidget(divider);
}

QLabel *createSectionLabel(const QString &text, QWidget *parent)
{
    QLabel *label = new QLabel(text, parent);
    label->setObjectName("sectionLabel");
    return label;
}

void placeCentered(QVBoxLayout *pageLayout, QWidget *card)
{
    pageLayout->addStretch(1);
    QHBoxLayout *wrapper = new QHBoxLayout();
    wrapper->addStretch(1);
    wrapper->addWidget(card, 4);
    wrapper->addStretch(1);
    pageLayout->addLayout(wrapper);
    pageLayout->addStretch(1);
}

// All employees of all projects, in the order they were assigned.
QMap<int, QStringList> loadProjectTeams()
{
    QMap<int, QStringList> teams;
    QSqlQuery query;
    query.prepare("SELECT a.project_id, e.Name "
                  "FROM assignments a JOIN Employees e ON e.Id = a.employee_id "
                  "ORDER BY a.assigned_at ASC, e.Id ASC");
    if (query.exec()) {
        while (query.next()) {
            teams[query.value(0).toInt()].append(query.value(1).toString());
        }
    }
    return teams;
}

void refreshTeamPanel(QWidget *page, int projectId)
{
    if (!page) return;
    QLabel *title = page->findChild<QLabel*>("teamTitle");
    QLabel *text = page->findChild<QLabel*>("teamText");
    if (!title || !text) return;

    if (projectId <= 0) {
        title->setText("Current team");
        text->setText("Select a project to see who is already working on it.");
        return;
    }

    const QStringList names = loadProjectTeams().value(projectId);
    title->setText(QString("Current team (%1)").arg(names.size()));
    text->setText(names.isEmpty() ? QString("Nobody is assigned to this project yet.") : names.join(", "));
}

void showFeedback(QWidget *page, const QString &message, const QString &color)
{
    if (!page) return;
    QLabel *label = page->findChild<QLabel*>("assignFeedback");
    if (!label) return;
    label->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: 600;").arg(color));
    label->setText(message);
    label->setVisible(true);
}

} // namespace

// ============================================================================
//  Construction
// ============================================================================
WorkSphereWindow::WorkSphereWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_database("DRIVER={ODBC Driver 17 for SQL Server};SERVER=LOCALHOST\\SQLEXPRESS;DATABASE=WorkSphere;Trusted_Connection=yes;Encrypt=yes;TrustServerCertificate=yes;")
    , ui(new Ui::WorkSphereWindow)
{
    ui->setupUi(this);

    // Forms are built on demand, make sure none of these pointers is ever uninitialised.
    comboAssignProject = nullptr;
    comboAssignEmployee = nullptr;
    txtEmployeeName = nullptr;
    txtEmployeeSalary = nullptr;
    txtEmployeePosition = nullptr;
    txtEmployeeBonus = nullptr;
    comboEmployeeType = nullptr;
    editProjectName = nullptr;
    editProjectDeadline = nullptr;
    comboCurrency = nullptr;
    comboDateFormat = nullptr;
    comboTheme = nullptr;

    // The theme flag must be known before the first table refresh (status colours depend on it).
    QSettings settings("WorkSphere", "WorkSphereApp");
    const QString savedTheme = settings.value("general/theme", "Dark Theme (Default)").toString();
    m_isDarkTheme = !savedTheme.contains("Light", Qt::CaseInsensitive);

    refreshEmployeeTable();
    refreshProjectTable();
    setupDashboard();
    setupNavigation();
    setupTimer();

    // 1. SEARCH INPUTS
    searchEmployeeInput = new QLineEdit(this);
    searchEmployeeInput->setPlaceholderText("Search employees by ID or name...");
    searchEmployeeInput->setClearButtonEnabled(true);

    searchProjectInput = new QLineEdit(this);
    searchProjectInput->setPlaceholderText("Search projects by name or employee...");
    searchProjectInput->setClearButtonEnabled(true);

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

    // 2. APPLY SAVED THEME (builds the forms, styles everything)
    applyTheme(savedTheme);

    // LOGO
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
    auto filterTable = [](QTableWidget *table, const QString &text, const QList<int> &columns) {
        const QString filter = text.trimmed().toLower();
        for (int row = 0; row < table->rowCount(); ++row) {
            bool match = filter.isEmpty();
            for (int col : columns) {
                QTableWidgetItem *item = table->item(row, col);
                if (item && item->text().toLower().contains(filter)) {
                    match = true;
                    break;
                }
            }
            table->setRowHidden(row, !match);
        }
    };

    connect(searchEmployeeInput, &QLineEdit::textChanged, this, [this, filterTable](const QString &text) {
        filterTable(ui->tableRecentEmployees, text, {0, 1});
    });

    connect(searchProjectInput, &QLineEdit::textChanged, this, [this, filterTable](const QString &text) {
        filterTable(ui->tableRecentProjects, text, {0, 1, 4});
    });

    // --- EMPLOYEE TABLE ACTION BAR ---
    ui->tableRecentEmployees->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableRecentEmployees->setSelectionMode(QAbstractItemView::SingleSelection);

    actionWidget = new QWidget(this);
    QHBoxLayout *actionLayout = new QHBoxLayout(actionWidget);
    actionLayout->setContentsMargins(0, 8, 0, 8);
    actionLayout->setSpacing(10);

    QPushButton *btnEditEmp = new QPushButton("Edit employee", this);
    QPushButton *btnDeleteEmp = new QPushButton("Delete employee", this);
    btnEditEmp->setObjectName("btnEdit");
    btnEditEmp->setCursor(Qt::PointingHandCursor);
    btnDeleteEmp->setObjectName("btnDelete");
    btnDeleteEmp->setCursor(Qt::PointingHandCursor);

    actionLayout->addWidget(btnEditEmp);
    actionLayout->addWidget(btnDeleteEmp);
    actionLayout->addStretch();

    actionWidget->setFixedHeight(50);
    ui->verticalLayoutDashboard->addWidget(actionWidget);
    actionWidget->setVisible(false);

    // --- PROJECT TABLE ACTION BAR ---
    ui->tableRecentProjects->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableRecentProjects->setSelectionMode(QAbstractItemView::SingleSelection);

    QWidget *projectActionWidget = new QWidget(this);
    QHBoxLayout *projActionLayout = new QHBoxLayout(projectActionWidget);
    projActionLayout->setContentsMargins(0, 8, 0, 8);
    projActionLayout->setSpacing(10);

    QPushButton *btnEditProj = new QPushButton("Edit project", this);
    QPushButton *btnDeleteProj = new QPushButton("Delete project", this);
    btnEditProj->setObjectName("btnEdit");
    btnEditProj->setCursor(Qt::PointingHandCursor);
    btnDeleteProj->setObjectName("btnDelete");
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

        handleProjectSelectionChanged();
    });

    connect(btnEditEmp, &QPushButton::clicked, this, &WorkSphereWindow::handleEditEmployee);
    connect(btnDeleteEmp, &QPushButton::clicked, this, &WorkSphereWindow::handleDeleteEmployee);
    connect(btnEditProj, &QPushButton::clicked, this, qOverload<>(&WorkSphereWindow::handleEditProject));
    connect(btnDeleteProj, &QPushButton::clicked, this, qOverload<>(&WorkSphereWindow::handleDeleteProject));

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
            // Detach right away: a widget that is only deleteLater()'d would still be found
            // by findChild() until the event loop runs.
            QWidget *w = child->widget();
            w->hide();
            w->setParent(nullptr);
            w->deleteLater();
        }
        delete child;
    }
}

// ============================================================================
//  Assignments: data for the two combo boxes
// ============================================================================
void WorkSphereWindow::loadAssignmentData()
{
    if (!comboAssignProject || !comboAssignEmployee) return;

    comboAssignProject->blockSignals(true);
    comboAssignProject->clear();
    comboAssignProject->addItem("Select a project...", -1);

    QSqlQuery qProjects("SELECT id, name, deadline FROM projects ORDER BY name ASC");
    while (qProjects.next()) {
        const int id = qProjects.value(0).toInt();
        const QString name = qProjects.value(1).toString();
        const QDate deadline = qProjects.value(2).toDate();

        QString label = name;
        if (deadline.isValid()) {
            label += "   (due " + formatDate(deadline) + ")";
        }
        comboAssignProject->addItem(label, id);
        comboAssignProject->setItemData(comboAssignProject->count() - 1, name, Qt::UserRole + 1);
    }
    comboAssignProject->setCurrentIndex(0);
    comboAssignProject->blockSignals(false);

    comboAssignEmployee->clear();
    comboAssignEmployee->addItem("Select an employee...", -1);

    QSqlQuery qEmployees("SELECT id, name FROM employees ORDER BY name ASC");
    while (qEmployees.next()) {
        const int id = qEmployees.value(0).toInt();
        const QString name = qEmployees.value(1).toString();

        comboAssignEmployee->addItem(QString("%1   (#%2)").arg(name).arg(id), id);
        comboAssignEmployee->setItemData(comboAssignEmployee->count() - 1, name, Qt::UserRole + 1);
    }

    refreshTeamPanel(ui->assignmentsPage, -1);
}

void WorkSphereWindow::setActiveButtonStyle(QPushButton* activeButton)
{
    m_activeNavButton = activeButton;
    updateNavigationStyles();
}

// ============================================================================
//  Theme
// ============================================================================
void WorkSphereWindow::applyTheme(const QString &themeName)
{
    m_isDarkTheme = !themeName.contains("Light", Qt::CaseInsensitive);

    this->setStyleSheet(buildStyleSheet(makePalette(m_isDarkTheme)));

    updateNavigationStyles();
    createEmployeeForm();
    createSettingsForm();
    if (ui->stackedWidget->currentWidget() == ui->projectFormPage) {
        createProjectForm();
    }

    // Status colours (green / red) depend on the theme, so rebuild the project rows.
    refreshProjectTable();
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

    const Palette p = makePalette(m_isDarkTheme);
    const QString activeStyle = navButtonStyle(true, p);
    const QString normalStyle = navButtonStyle(false, p);

    ui->btnDashboard->setStyleSheet(ui->btnDashboard == m_activeNavButton ? activeStyle : normalStyle);
    ui->btnEmployees->setStyleSheet(ui->btnEmployees == m_activeNavButton ? activeStyle : normalStyle);
    ui->btnProjects->setStyleSheet(ui->btnProjects == m_activeNavButton ? activeStyle : normalStyle);
    ui->btnAssignments->setStyleSheet(ui->btnAssignments == m_activeNavButton ? activeStyle : normalStyle);
    if (ui->btnSettings) {
        ui->btnSettings->setStyleSheet(ui->btnSettings == m_activeNavButton ? activeStyle : normalStyle);
    }
}

// ============================================================================
//  Dashboard
// ============================================================================
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

    // Tables: read-only look, no grid, no row numbers.
    const QList<QTableWidget*> tables = {ui->tableRecentEmployees, ui->tableRecentProjects};
    for (QTableWidget *table : tables) {
        table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        table->setShowGrid(false);
        table->setAlternatingRowColors(true);
        table->verticalHeader()->setVisible(false);
        table->verticalHeader()->setDefaultSectionSize(34);
        table->horizontalHeader()->setFixedHeight(36);
        table->horizontalHeader()->setHighlightSections(false);
        table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    // Employees are edited through the dialog only (same as projects).
    ui->tableRecentEmployees->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QHeaderView *empHeader = ui->tableRecentEmployees->horizontalHeader();
    empHeader->setSectionResizeMode(QHeaderView::Stretch);
    empHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    empHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    empHeader->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    // Projects are edited through the dialog only.
    ui->tableRecentProjects->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QHeaderView *projHeader = ui->tableRecentProjects->horizontalHeader();
    projHeader->setSectionResizeMode(QHeaderView::Stretch);
    projHeader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    projHeader->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    projHeader->setSectionResizeMode(3, QHeaderView::ResizeToContents);

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

    if (currencySetting.contains("EUR")) {
        exchangeRate = 0.92f;
        currencySymbol = QString::fromUtf8("\xE2\x82\xAC");
    }
    else if (currencySetting.contains("din") || currencySetting.contains("RSD")) {
        exchangeRate = 108.5f;
        currencySymbol = "din";
    }
    else if (currencySetting.contains("KM") || currencySetting.contains("BAM")) {
        exchangeRate = 1.8f;
        currencySymbol = "KM";
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

// ============================================================================
//  Navigation + timer
// ============================================================================
void WorkSphereWindow::setupNavigation()
{
    m_activeNavButton = ui->btnDashboard;

    // uic auto-connects on_btnProjects_clicked(); the lambda below does the same job,
    // so drop the auto connection to avoid running the handler twice.
    ui->btnProjects->disconnect(this);

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
        handleProjectsButtonClicked();
    });

    connect(ui->btnAssignments, &QPushButton::clicked, this, [this]() {
        ui->stackedWidget->setCurrentWidget(ui->assignmentsPage);
        m_activeNavButton = ui->btnAssignments;
        updateNavigationStyles();

        // Rebuild the form each time so the lists always reflect the database.
        QVBoxLayout *pageLayout = qobject_cast<QVBoxLayout*>(ui->assignmentsPage->layout());
        if (pageLayout) {
            clearLayout(pageLayout);
            createAssignWorkerForm(pageLayout);
        }
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
        if (ui->clockLabel) {
            ui->clockLabel->setText(currentTime);
        }
    });
    timer->start(1000);
}

// ============================================================================
//  Employee form
// ============================================================================
void WorkSphereWindow::createEmployeeForm()
{
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->employeeFormPage->layout());
    if (!mainLayout) return;

    clearLayout(mainLayout);

    QFrame *card = createCard(ui->employeeFormPage);
    QVBoxLayout *cardLayout = createCardLayout(card);
    addCardHeader(cardLayout, card, "New employee",
                  "Register a worker or a manager. The last field depends on the employee type.");

    txtEmployeeName = new QLineEdit(card);
    txtEmployeeName->setPlaceholderText("e.g. John Doe");
    cardLayout->addWidget(createInputRow("Full name", txtEmployeeName, card));

    txtEmployeeSalary = new QLineEdit(card);
    txtEmployeeSalary->setPlaceholderText("e.g. 4500");

    comboEmployeeType = new QComboBox(card);
    comboEmployeeType->addItem("Worker");
    comboEmployeeType->addItem("Manager");

    QHBoxLayout *rowTwoColumns = new QHBoxLayout();
    rowTwoColumns->setSpacing(16);
    rowTwoColumns->addWidget(createInputRow("Salary ($)", txtEmployeeSalary, card));
    rowTwoColumns->addWidget(createInputRow("Employee type", comboEmployeeType, card));
    cardLayout->addLayout(rowTwoColumns);

    // Page 0 = worker (position), page 1 = manager (bonus)
    QStackedWidget *stackedFields = new QStackedWidget(card);

    txtEmployeePosition = new QLineEdit();
    txtEmployeePosition->setPlaceholderText("e.g. Senior C++ Developer");
    stackedFields->addWidget(createInputRow("Position", txtEmployeePosition, stackedFields));

    txtEmployeeBonus = new QLineEdit();
    txtEmployeeBonus->setPlaceholderText("e.g. 1200");
    stackedFields->addWidget(createInputRow("Bonus ($)", txtEmployeeBonus, stackedFields));

    cardLayout->addWidget(stackedFields);

    connect(comboEmployeeType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            stackedFields, &QStackedWidget::setCurrentIndex);

    QPushButton *btnClear = new QPushButton("Clear", card);
    btnClear->setObjectName("secondaryButton");
    btnClear->setCursor(Qt::PointingHandCursor);

    QPushButton *btnCreate = new QPushButton("Create employee", card);
    btnCreate->setObjectName("createButton");
    btnCreate->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *footer = new QHBoxLayout();
    footer->setSpacing(12);
    footer->addStretch();
    footer->addWidget(btnClear);
    footer->addWidget(btnCreate);
    cardLayout->addSpacing(6);
    cardLayout->addLayout(footer);

    placeCentered(mainLayout, card);

    connect(btnCreate, &QPushButton::clicked, this, &WorkSphereWindow::handleCreateEmployee);
    connect(txtEmployeeName, &QLineEdit::returnPressed, this, &WorkSphereWindow::handleCreateEmployee);
    connect(txtEmployeeSalary, &QLineEdit::returnPressed, this, &WorkSphereWindow::handleCreateEmployee);
    connect(txtEmployeePosition, &QLineEdit::returnPressed, this, &WorkSphereWindow::handleCreateEmployee);
    connect(txtEmployeeBonus, &QLineEdit::returnPressed, this, &WorkSphereWindow::handleCreateEmployee);

    QLineEdit *name = txtEmployeeName;
    QLineEdit *salary = txtEmployeeSalary;
    QLineEdit *position = txtEmployeePosition;
    QLineEdit *bonus = txtEmployeeBonus;
    connect(btnClear, &QPushButton::clicked, this, [name, salary, position, bonus]() {
        name->clear();
        salary->clear();
        position->clear();
        bonus->clear();
        name->setFocus();
    });
}

// ============================================================================
//  Project form
// ============================================================================
void WorkSphereWindow::handleProjectsButtonClicked()
{
    ui->stackedWidget->setCurrentWidget(ui->projectFormPage);
    createProjectForm();
}

void WorkSphereWindow::createProjectForm()
{
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->projectFormPage->layout());
    if (!mainLayout) return;

    clearLayout(mainLayout);

    QFrame *card = createCard(ui->projectFormPage);
    QVBoxLayout *cardLayout = createCardLayout(card);
    addCardHeader(cardLayout, card, "New project",
                  "Give the project a name and a deadline. Add employees on the Assignments page.");

    editProjectName = new QLineEdit(card);
    editProjectName->setPlaceholderText("e.g. Website redesign");
    cardLayout->addWidget(createInputRow("Project name", editProjectName, card));

    editProjectDeadline = new QDateEdit(QDate::currentDate().addMonths(1), card);
    editProjectDeadline->setCalendarPopup(true);
    editProjectDeadline->setDisplayFormat(configuredDateFormat());
    cardLayout->addWidget(createInputRow("Deadline", editProjectDeadline, card));

    // Live preview of the status the project will get.
    QLabel *statusHint = new QLabel(card);
    statusHint->setWordWrap(true);
    cardLayout->addWidget(statusHint);

    QDateEdit *deadlineEdit = editProjectDeadline;
    const bool dark = m_isDarkTheme;
    auto updateHint = [statusHint, deadlineEdit, dark]() {
        const Palette p = makePalette(dark);
        const bool active = QDate::currentDate() < deadlineEdit->date();
        statusHint->setText(active
                                ? QString("Active: the project stays active until its deadline.")
                                : QString("Inactive: the deadline is not in the future."));
        statusHint->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: 600;")
                                      .arg(active ? p.success : p.danger));
    };
    connect(deadlineEdit, &QDateEdit::dateChanged, this, [updateHint](const QDate &) { updateHint(); });
    updateHint();

    QPushButton *btnReset = new QPushButton("Reset", card);
    btnReset->setObjectName("secondaryButton");
    btnReset->setCursor(Qt::PointingHandCursor);

    QPushButton *btnSave = new QPushButton("Add project", card);
    btnSave->setObjectName("createButton");
    btnSave->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *footer = new QHBoxLayout();
    footer->setSpacing(12);
    footer->addStretch();
    footer->addWidget(btnReset);
    footer->addWidget(btnSave);
    cardLayout->addSpacing(6);
    cardLayout->addLayout(footer);

    placeCentered(mainLayout, card);

    connect(btnSave, &QPushButton::clicked, this, &WorkSphereWindow::handleSaveProject);
    connect(editProjectName, &QLineEdit::returnPressed, this, &WorkSphereWindow::handleSaveProject);

    QLineEdit *nameEdit = editProjectName;
    connect(btnReset, &QPushButton::clicked, this, [nameEdit, deadlineEdit]() {
        nameEdit->clear();
        deadlineEdit->setDate(QDate::currentDate().addMonths(1));
        nameEdit->setFocus();
    });
}

void WorkSphereWindow::handleSaveProject()
{
    if (!editProjectName || !editProjectDeadline) return;

    std::string name = editProjectName->text().trimmed().toStdString();
    QDate deadline = editProjectDeadline->date();

    if (name.empty()) {
        QMessageBox::warning(this, "Project name missing", "Enter a name for the project before adding it.");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO Projects (Name, Deadline) VALUES (:name, :deadline)");
    query.bindValue(":name", QString::fromStdString(name));
    query.bindValue(":deadline", deadline);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database error",
                              "The project could not be added.\n\n" + query.lastError().text());
        return;
    }

    QMessageBox::information(this, "Project added", "The project was added successfully.");

    editProjectName->clear();
    editProjectDeadline->setDate(QDate::currentDate().addMonths(1));

    refreshProjectTable();
    updateDashboardStats();
}

// ============================================================================
//  Settings form
// ============================================================================
void WorkSphereWindow::createSettingsForm()
{
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->settingsPage->layout());
    if (!mainLayout) return;

    clearLayout(mainLayout);

    QSettings settings("WorkSphere", "WorkSphereApp");
    QString savedCurrency = settings.value("general/currency", "USD ($)").toString();
    QString savedDateFormat = settings.value("general/dateFormat", "dd.MM.yyyy").toString();
    QString savedTheme = settings.value("general/theme", m_isDarkTheme ? "Dark Theme (Default)" : "Light Theme").toString();

    QFrame *card = createCard(ui->settingsPage);
    QVBoxLayout *cardLayout = createCardLayout(card);
    addCardHeader(cardLayout, card, "Settings",
                  "Choose how WorkSphere shows money, dates and colors.");

    // --- Regional ---
    cardLayout->addWidget(createSectionLabel("Regional", card));

    comboCurrency = new QComboBox(card);
    QStringList currencies;
    currencies << QString::fromUtf8("EUR (\xE2\x82\xAC)") << "USD ($)" << "RSD (din)" << "BAM (KM)";
    comboCurrency->addItems(currencies);
    const int currencyIndex = comboCurrency->findText(savedCurrency);
    if (currencyIndex >= 0) comboCurrency->setCurrentIndex(currencyIndex);

    comboDateFormat = new QComboBox(card);
    comboDateFormat->addItems({"dd.MM.yyyy", "yyyy-MM-dd", "MM/dd/yyyy"});
    const int dateIndex = comboDateFormat->findText(savedDateFormat);
    if (dateIndex >= 0) comboDateFormat->setCurrentIndex(dateIndex);

    QHBoxLayout *regionalRow = new QHBoxLayout();
    regionalRow->setSpacing(16);
    regionalRow->addWidget(createInputRow("Currency", comboCurrency, card));
    regionalRow->addWidget(createInputRow("Date format", comboDateFormat, card));
    cardLayout->addLayout(regionalRow);

    QLabel *currencyNote = new QLabel("Salaries are stored in USD and converted for display.", card);
    currencyNote->setObjectName("cardSubtitle");
    currencyNote->setWordWrap(true);
    cardLayout->addWidget(currencyNote);

    // --- Appearance ---
    cardLayout->addSpacing(4);
    cardLayout->addWidget(createSectionLabel("Appearance", card));

    comboTheme = new QComboBox(card);
    comboTheme->addItems({"Dark Theme (Default)", "Light Theme"});
    const int themeIndex = comboTheme->findText(savedTheme);
    if (themeIndex >= 0) comboTheme->setCurrentIndex(themeIndex);
    cardLayout->addWidget(createInputRow("Theme", comboTheme, card));

    QPushButton *btnSaveSettings = new QPushButton("Save settings", card);
    btnSaveSettings->setObjectName("createButton");
    btnSaveSettings->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *footer = new QHBoxLayout();
    footer->addStretch();
    footer->addWidget(btnSaveSettings);
    cardLayout->addSpacing(6);
    cardLayout->addLayout(footer);

    placeCentered(mainLayout, card);

    connect(btnSaveSettings, &QPushButton::clicked, this, &WorkSphereWindow::saveSettings);
}

void WorkSphereWindow::saveSettings()
{
    if (!comboCurrency || !comboDateFormat || !comboTheme) return;

    const QString selectedTheme = comboTheme->currentText();

    QSettings settings("WorkSphere", "WorkSphereApp");
    settings.setValue("general/currency", comboCurrency->currentText());
    settings.setValue("general/dateFormat", comboDateFormat->currentText());
    settings.setValue("general/theme", selectedTheme);

    // Rebuilds the forms (this button is deleted later, which is safe) and re-colors the tables.
    applyTheme(selectedTheme);

    updateDashboardStats();

    QMessageBox::information(this, "Settings saved", "Your settings were saved.");
}

// ============================================================================
//  Assignments
// ============================================================================
void WorkSphereWindow::createAssignWorkerForm(QVBoxLayout *parentLayout)
{
    if (!parentLayout) return;

    QWidget *page = parentLayout->parentWidget();

    QFrame *card = createCard(page);
    QVBoxLayout *cardLayout = createCardLayout(card);
    addCardHeader(cardLayout, card, "Assign employee",
                  "Add an employee to a project. You can assign several people one after another.");

    comboAssignProject = new QComboBox(card);
    comboAssignEmployee = new QComboBox(card);
    for (QComboBox *combo : {comboAssignProject, comboAssignEmployee}) {
        combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
        combo->setMinimumContentsLength(18);
    }

    cardLayout->addWidget(createInputRow("Project", comboAssignProject, card));
    cardLayout->addWidget(createInputRow("Employee (worker or manager)", comboAssignEmployee, card));

    // Who is already on the selected project
    QFrame *teamBox = new QFrame(card);
    teamBox->setObjectName("teamBox");
    QVBoxLayout *teamLayout = new QVBoxLayout(teamBox);
    teamLayout->setContentsMargins(16, 14, 16, 14);
    teamLayout->setSpacing(6);

    QLabel *teamTitle = new QLabel("Current team", teamBox);
    teamTitle->setObjectName("teamTitle");
    QLabel *teamText = new QLabel(teamBox);
    teamText->setObjectName("teamText");
    teamText->setWordWrap(true);
    teamLayout->addWidget(teamTitle);
    teamLayout->addWidget(teamText);
    cardLayout->addWidget(teamBox);

    QLabel *feedback = new QLabel(card);
    feedback->setObjectName("assignFeedback");
    feedback->setWordWrap(true);
    feedback->setVisible(false);
    cardLayout->addWidget(feedback);

    QPushButton *btnAssign = new QPushButton("Assign employee", card);
    btnAssign->setObjectName("createButton");
    btnAssign->setCursor(Qt::PointingHandCursor);

    QHBoxLayout *footer = new QHBoxLayout();
    footer->addStretch();
    footer->addWidget(btnAssign);
    cardLayout->addSpacing(6);
    cardLayout->addLayout(footer);

    placeCentered(parentLayout, card);

    connect(btnAssign, &QPushButton::clicked, this, &WorkSphereWindow::handleSaveAssignment);

    QComboBox *projectCombo = comboAssignProject;
    connect(projectCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [page, projectCombo](int) {
        refreshTeamPanel(page, projectCombo->currentData().toInt());
        QLabel *label = page->findChild<QLabel*>("assignFeedback");
        if (label) label->setVisible(false);
    });

    loadAssignmentData();
}

void WorkSphereWindow::handleSaveAssignment()
{
    if (!comboAssignProject || !comboAssignEmployee) return;

    const int projectId = comboAssignProject->currentData().toInt();
    const int employeeId = comboAssignEmployee->currentData().toInt();

    if (projectId <= 0 || employeeId <= 0) {
        QMessageBox::warning(this, "Selection missing",
                             "Choose both a project and an employee before assigning.");
        return;
    }

    const Palette p = makePalette(m_isDarkTheme);
    const QString projectName = comboAssignProject->currentData(Qt::UserRole + 1).toString();
    const QString employeeName = comboAssignEmployee->currentData(Qt::UserRole + 1).toString();

    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM assignments WHERE project_id = :pId AND employee_id = :eId");
    checkQuery.bindValue(":pId", projectId);
    checkQuery.bindValue(":eId", employeeId);

    if (!checkQuery.exec()) {
        QMessageBox::critical(this, "Database error",
                              "The assignments could not be checked.\n\n" + checkQuery.lastError().text());
        return;
    }
    if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        showFeedback(ui->assignmentsPage,
                     QString("%1 is already on %2.").arg(employeeName, projectName),
                     p.danger);
        return;
    }
    checkQuery.finish();

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO assignments (project_id, employee_id, assigned_at) "
                        "VALUES (:pId, :eId, CURRENT_TIMESTAMP)");
    insertQuery.bindValue(":pId", projectId);
    insertQuery.bindValue(":eId", employeeId);

    if (!insertQuery.exec()) {
        QMessageBox::critical(this, "Database error",
                              "The employee could not be assigned.\n\n" + insertQuery.lastError().text());
        return;
    }

    // Keep the project selected so several employees can be added in a row.
    comboAssignEmployee->setCurrentIndex(0);
    refreshTeamPanel(ui->assignmentsPage, projectId);
    showFeedback(ui->assignmentsPage,
                 QString("%1 was added to %2.").arg(employeeName, projectName),
                 p.success);

    refreshProjectTable();
}

void WorkSphereWindow::handleAssignWorker()
{
    handleSaveAssignment();
}

// ============================================================================
//  Employees: create / edit / delete
// ============================================================================
void WorkSphereWindow::handleCreateEmployee()
{
    if (!txtEmployeeName || !txtEmployeeSalary || !comboEmployeeType) return;

    QString nameStr = txtEmployeeName->text().trimmed();
    QString salaryStr = txtEmployeeSalary->text().trimmed();
    int typeIndex = comboEmployeeType->currentIndex();

    if (nameStr.isEmpty() || salaryStr.isEmpty()) {
        QMessageBox::warning(this, "Missing details", "Enter the employee's name and salary.");
        return;
    }

    bool salaryOk = false;
    float salary = salaryStr.toFloat(&salaryOk);
    if (!salaryOk || salary < 0) {
        QMessageBox::warning(this, "Invalid salary", "Enter the salary as a number that is not negative, for example 4500.");
        return;
    }

    int id = 0;

    try {
        if (typeIndex == 0) { // Worker
            QString positionStr = txtEmployeePosition ? txtEmployeePosition->text().trimmed() : "";

            if (positionStr.isEmpty()) {
                QMessageBox::warning(this, "Missing position", "Enter the worker's position.");
                return;
            }

            Worker worker(positionStr.toStdString(), id, nameStr.toStdString(), salary);
            m_database.addWorker(worker);
        }
        else { // Manager
            QString bonusStr = txtEmployeeBonus ? txtEmployeeBonus->text().trimmed() : "";

            if (bonusStr.isEmpty()) {
                QMessageBox::warning(this, "Missing bonus", "Enter the manager's bonus.");
                return;
            }

            bool bonusOk = false;
            float bonus = bonusStr.toFloat(&bonusOk);
            if (!bonusOk || bonus < 0) {
                QMessageBox::warning(this, "Invalid bonus", "Enter the bonus as a number that is not negative, for example 1200.");
                return;
            }

            Manager manager(id, nameStr.toStdString(), salary, bonus);
            m_database.addManager(manager);
        }
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Database error", QString("The employee could not be added.\n\n%1").arg(e.what()));
        return;
    }

    QMessageBox::information(this, "Employee added", "The employee was added successfully.");

    txtEmployeeName->clear();
    txtEmployeeSalary->clear();
    if (txtEmployeePosition) txtEmployeePosition->clear();
    if (txtEmployeeBonus) txtEmployeeBonus->clear();

    updateDashboardStats();
    refreshEmployeeTable();
}

void WorkSphereWindow::handleEditEmployee()
{
    QTableWidget *table = ui->tableRecentEmployees;
    const int currentRow = table->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "No employee selected", "Select an employee in the table first.");
        return;
    }

    QTableWidgetItem *idItem = table->item(currentRow, 0);
    QTableWidgetItem *nameItem = table->item(currentRow, 1);
    QTableWidgetItem *salaryItem = table->item(currentRow, 2);
    QTableWidgetItem *typeItem = table->item(currentRow, 3);
    QTableWidgetItem *extraItem = table->item(currentRow, 4);

    if (!idItem || !nameItem || !salaryItem || !typeItem) {
        QMessageBox::critical(this, "Error", "The selected row has invalid employee data.");
        return;
    }

    const int empId = idItem->text().toInt();
    const QString currentName = nameItem->text().trimmed();
    const QString currentSalary = salaryItem->text().trimmed();
    const QString empType = typeItem->text().trimmed();
    const QString currentExtra = extraItem ? extraItem->text().trimmed() : QString();
    const bool isWorker = empType.contains("Worker", Qt::CaseInsensitive);

    // ---- Dialog ----
    QDialog dialog(this);
    dialog.setWindowTitle("Edit employee");
    dialog.setMinimumWidth(440);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(26, 24, 26, 20);
    layout->setSpacing(16);

    QLabel *title = new QLabel(QString("Edit employee #%1").arg(empId), &dialog);
    title->setObjectName("cardTitle");
    layout->addWidget(title);

    QLabel *subtitle = new QLabel("Change the details below and press Save. The employee type cannot be changed.", &dialog);
    subtitle->setObjectName("cardSubtitle");
    subtitle->setWordWrap(true);
    layout->addWidget(subtitle);

    QLineEdit *editName = new QLineEdit(currentName, &dialog);
    layout->addWidget(createInputRow("Full name", editName, &dialog));

    QLineEdit *editSalary = new QLineEdit(currentSalary, &dialog);
    QLineEdit *editType = new QLineEdit(isWorker ? "Worker" : "Manager", &dialog);
    editType->setReadOnly(true);
    editType->setEnabled(false);

    QHBoxLayout *twoColumns = new QHBoxLayout();
    twoColumns->setSpacing(16);
    twoColumns->addWidget(createInputRow("Salary ($)", editSalary, &dialog));
    twoColumns->addWidget(createInputRow("Employee type", editType, &dialog));
    layout->addLayout(twoColumns);

    QLineEdit *editExtra = new QLineEdit(currentExtra, &dialog);
    editExtra->setPlaceholderText(isWorker ? "e.g. Senior C++ Developer" : "e.g. 1200");
    layout->addWidget(createInputRow(isWorker ? "Position" : "Bonus ($)", editExtra, &dialog));
    layout->addSpacing(4);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(buttonBox);

    // Keep the dialog open until every field is valid.
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog,
            [&dialog, editName, editSalary, editExtra, isWorker]() {
                if (editName->text().trimmed().isEmpty()) {
                    QMessageBox::warning(&dialog, "Missing name", "The employee's name cannot be empty.");
                    return;
                }

                bool salaryOk = false;
                const float salary = editSalary->text().trimmed().toFloat(&salaryOk);
                if (!salaryOk || salary < 0) {
                    QMessageBox::warning(&dialog, "Invalid salary",
                                         "Enter the salary as a number that is not negative, for example 4500.");
                    return;
                }

                const QString extra = editExtra->text().trimmed();
                if (isWorker) {
                    if (extra.isEmpty()) {
                        QMessageBox::warning(&dialog, "Missing position", "The position cannot be empty.");
                        return;
                    }
                } else {
                    bool bonusOk = false;
                    const float bonus = extra.toFloat(&bonusOk);
                    if (!bonusOk || bonus < 0) {
                        QMessageBox::warning(&dialog, "Invalid bonus",
                                             "Enter the bonus as a number that is not negative, for example 1200.");
                        return;
                    }
                }
                dialog.accept();
            });
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    editName->setFocus();
    if (dialog.exec() != QDialog::Accepted) return;

    // ---- Save ----
    const QString name = editName->text().trimmed();
    const float salary = editSalary->text().trimmed().toFloat();
    const QString extra = editExtra->text().trimmed();

    try {
        if (isWorker) {
            Worker worker(extra.toStdString(), empId, name.toStdString(), salary);
            m_database.updateEmployee(worker);
        } else {
            Manager manager(empId, name.toStdString(), salary, extra.toFloat());
            m_database.updateEmployee(manager);
        }

        QMessageBox::information(this, "Employee updated", "The employee's details were updated.");
        refreshEmployeeTable();
        refreshProjectTable();   // employee names are shown in the project rows
        updateDashboardStats();
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Database error",
                              QString("The employee could not be updated.\n\n%1").arg(e.what()));
    }
}

void WorkSphereWindow::handleDeleteEmployee()
{
    QModelIndexList selectedIndexes = ui->tableRecentEmployees->selectionModel()->selectedRows();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "No employee selected", "Select an employee in the table first.");
        return;
    }

    int row = selectedIndexes.first().row();
    QTableWidgetItem *idItem = ui->tableRecentEmployees->item(row, 0);
    if (!idItem) return;

    int employeeId = idItem->text().toInt();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Delete employee",
        "Delete the selected employee? They will also be removed from all projects.",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        try {
            // Remove the employee's project assignments first, so no row is left pointing at them.
            QSqlQuery cleanup;
            cleanup.prepare("DELETE FROM assignments WHERE employee_id = :id");
            cleanup.bindValue(":id", employeeId);
            cleanup.exec();

            m_database.removeEmployee(employeeId);
            QMessageBox::information(this, "Employee deleted", "The employee was deleted.");
            refreshEmployeeTable();
            refreshProjectTable();
            updateDashboardStats();
        }
        catch (const std::exception& e) {
            QMessageBox::critical(this, "Database error", QString("The employee could not be deleted.\n\n%1").arg(e.what()));
        }
    }
}

void WorkSphereWindow::onEmployeeCellChanged(int row, int column)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
}

// ============================================================================
//  Tables
// ============================================================================
void WorkSphereWindow::refreshEmployeeTable()
{
    if (!ui->tableRecentEmployees) return;

    std::vector<std::unique_ptr<Employee>> employees = m_database.loadEmployees();
    ui->tableRecentEmployees->setRowCount(0);

    for (const auto& emp : employees) {
        int row = ui->tableRecentEmployees->rowCount();
        ui->tableRecentEmployees->insertRow(row);

        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(emp->getId()));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        ui->tableRecentEmployees->setItem(row, 0, idItem);

        ui->tableRecentEmployees->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(emp->getName())));
        ui->tableRecentEmployees->setItem(row, 2, new QTableWidgetItem(QString::number(emp->getSalary())));

        if (auto worker = dynamic_cast<const Worker*>(emp.get())) {
            QTableWidgetItem *typeItem = new QTableWidgetItem("Worker");
            typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
            ui->tableRecentEmployees->setItem(row, 3, typeItem);

            ui->tableRecentEmployees->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(worker->getPosition())));
        }
        else if (auto manager = dynamic_cast<const Manager*>(emp.get())) {
            QTableWidgetItem *typeItem = new QTableWidgetItem("Manager");
            typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
            ui->tableRecentEmployees->setItem(row, 3, typeItem);

            ui->tableRecentEmployees->setItem(row, 4, new QTableWidgetItem(QString::number(manager->getBonus())));
        }
    }
}

// Columns: 0 Id | 1 Project name | 2 Deadline | 3 Status (green/red) | 4 Employees (comma separated)
void WorkSphereWindow::refreshProjectTable()
{
    projectsList.clear();

    QTableWidget *table = ui->tableRecentProjects;
    if (!table) return;

    // 1) Read everything first. Running a second query while the first one is still
    //    being read can fail on ODBC / SQL Server ("connection is busy").
    struct ProjectRow
    {
        int id;
        QString name;
        QDate deadline;
    };
    std::vector<ProjectRow> rows;
    {
        QSqlQuery query("SELECT Id, Name, Deadline FROM Projects ORDER BY Id");
        while (query.next()) {
            ProjectRow r;
            r.id = query.value(0).toInt();
            r.name = query.value(1).toString();
            r.deadline = query.value(2).toDate();
            rows.push_back(r);
        }
    }
    const QMap<int, QStringList> teams = loadProjectTeams();

    // 2) Fill the table
    const Palette p = makePalette(m_isDarkTheme);
    const QDate today = QDate::currentDate();

    table->setRowCount(0);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto makeItem = [](const QString &text) {
        QTableWidgetItem *item = new QTableWidgetItem(text);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        return item;
    };

    for (const ProjectRow &r : rows) {
        projectsList.emplace_back(r.name.toStdString(), r.deadline);

        // A project is active while today is before its deadline.
        const bool active = r.deadline.isValid() && today < r.deadline;
        const QStringList names = teams.value(r.id);

        const int row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0, makeItem(QString::number(r.id)));
        table->setItem(row, 1, makeItem(r.name));
        table->setItem(row, 2, makeItem(formatDate(r.deadline)));

        QTableWidgetItem *statusItem = makeItem(active ? "Active" : "Inactive");
        statusItem->setForeground(QBrush(QColor(active ? p.success : p.danger)));
        QFont statusFont = statusItem->font();
        statusFont.setBold(true);
        statusItem->setFont(statusFont);
        table->setItem(row, 3, statusItem);

        QTableWidgetItem *teamItem = makeItem(names.isEmpty() ? QString("No employees") : names.join(", "));
        if (names.isEmpty()) {
            teamItem->setForeground(QBrush(QColor(p.subtle)));
        } else {
            teamItem->setToolTip(names.join("\n"));
        }
        table->setItem(row, 4, teamItem);
    }

    handleProjectSelectionChanged();
}

// Project actions live in the dashboard action bar now, so there are no per-row buttons.
void WorkSphereWindow::addProjectActionButtons(int row, int projectId)
{
    Q_UNUSED(row);
    Q_UNUSED(projectId);
}

// ============================================================================
//  Projects: delete / edit
// ============================================================================
void WorkSphereWindow::handleDeleteProject(int projectId)
{
    QMessageBox::StandardButton confirm = QMessageBox::question(
        this, "Delete project",
        "Delete this project? Its employee assignments will be removed too.",
        QMessageBox::Yes | QMessageBox::No
        );

    if (confirm != QMessageBox::Yes) return;

    QSqlQuery cleanup;
    cleanup.prepare("DELETE FROM assignments WHERE project_id = :id");
    cleanup.bindValue(":id", projectId);
    cleanup.exec();

    QSqlQuery query;
    query.prepare("DELETE FROM Projects WHERE Id = :id");
    query.bindValue(":id", projectId);

    if (query.exec()) {
        QMessageBox::information(this, "Project deleted", "The project was deleted.");
        refreshProjectTable();
        updateDashboardStats();
    } else {
        QMessageBox::critical(this, "Database error",
                              "The project could not be deleted.\n\n" + query.lastError().text());
    }
}

void WorkSphereWindow::handleDeleteProject()
{
    int currentRow = ui->tableRecentProjects->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "No project selected", "Select a project in the table first.");
        return;
    }

    QTableWidgetItem *idItem = ui->tableRecentProjects->item(currentRow, 0);
    if (!idItem) return;

    handleDeleteProject(idItem->text().toInt());
}

void WorkSphereWindow::handleEditProject(int projectId)
{
    QString currentName;
    QDate currentDeadline;
    {
        QSqlQuery query;
        query.prepare("SELECT Name, Deadline FROM Projects WHERE Id = :id");
        query.bindValue(":id", projectId);

        if (!query.exec() || !query.next()) {
            QMessageBox::critical(this, "Database error", "The project could not be loaded.");
            return;
        }
        currentName = query.value(0).toString();
        currentDeadline = query.value(1).toDate();
    }
    if (!currentDeadline.isValid()) currentDeadline = QDate::currentDate();

    QDialog dialog(this);
    dialog.setWindowTitle("Edit project");
    dialog.setMinimumWidth(420);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(26, 24, 26, 20);
    layout->setSpacing(16);

    QLabel *title = new QLabel("Edit project", &dialog);
    title->setObjectName("cardTitle");
    layout->addWidget(title);

    QLineEdit *editName = new QLineEdit(currentName, &dialog);
    QDateEdit *editDeadline = new QDateEdit(currentDeadline, &dialog);
    editDeadline->setCalendarPopup(true);
    editDeadline->setDisplayFormat(configuredDateFormat());

    layout->addWidget(createInputRow("Project name", editName, &dialog));
    layout->addWidget(createInputRow("Deadline", editDeadline, &dialog));
    layout->addSpacing(4);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialog
        );
    layout->addWidget(buttonBox);

    // Keep the dialog open until the name is valid.
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, [&dialog, editName]() {
        if (editName->text().trimmed().isEmpty()) {
            QMessageBox::warning(&dialog, "Project name missing", "Enter a name for the project.");
            return;
        }
        dialog.accept();
    });
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted) return;

    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE Projects SET Name = :name, Deadline = :deadline WHERE Id = :id");
    updateQuery.bindValue(":name", editName->text().trimmed());
    updateQuery.bindValue(":deadline", editDeadline->date());
    updateQuery.bindValue(":id", projectId);

    if (updateQuery.exec()) {
        QMessageBox::information(this, "Project updated", "The project was updated.");
        refreshProjectTable();
    } else {
        QMessageBox::critical(this, "Database error",
                              "The project could not be updated.\n\n" + updateQuery.lastError().text());
    }
}

void WorkSphereWindow::handleEditProject()
{
    int currentRow = ui->tableRecentProjects->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "No project selected", "Select a project in the table first.");
        return;
    }

    QTableWidgetItem *idItem = ui->tableRecentProjects->item(currentRow, 0);
    if (!idItem) return;

    handleEditProject(idItem->text().toInt());
}

// ============================================================================
//  Misc helpers
// ============================================================================

// Label above the input (works for both full-width and two-column rows).
QWidget* WorkSphereWindow::createInputRow(const QString &labelText, QWidget *inputField, QWidget *parent)
{
    QWidget *rowWidget = new QWidget(parent ? parent : this);
    QVBoxLayout *layout = new QVBoxLayout(rowWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);

    QLabel *label = new QLabel(labelText, rowWidget);
    label->setObjectName("fieldLabel");

    layout->addWidget(label);
    layout->addWidget(inputField);

    return rowWidget;
}

void WorkSphereWindow::handleProjectSelectionChanged()
{
    // The Edit / Delete project buttons are shown or hidden by the selection lambda in the
    // constructor (dashboard action bar), so nothing else has to happen here.
}

QString WorkSphereWindow::formatDate(const QDate &date) const {
    if (!date.isValid()) return QString("-");
    QSettings settings("WorkSphere", "WorkSphereApp");
    QString dateFormat = settings.value("general/dateFormat", "dd.MM.yyyy").toString();
    return date.toString(dateFormat);
}

// ============================================================================
//  Auto-connected slot wrappers (kept because the header declares them)
// ============================================================================
void WorkSphereWindow::on_btnProjects_clicked() { handleProjectsButtonClicked(); }
void WorkSphereWindow::on_btnSaveProject_clicked() { handleSaveProject(); }
void WorkSphereWindow::on_btnDeleteProject_clicked() { handleDeleteProject(); }
void WorkSphereWindow::on_btnEditProject_clicked() { handleEditProject(); }
void WorkSphereWindow::onProjectSelectionChanged() { handleProjectSelectionChanged(); }
void WorkSphereWindow::on_btnEditEmployee_clicked() { handleEditEmployee(); }
void WorkSphereWindow::on_btnSaveAssignment_clicked() { handleSaveAssignment(); }
void WorkSphereWindow::on_btnDeleteEmployee_clicked() { handleDeleteEmployee(); }