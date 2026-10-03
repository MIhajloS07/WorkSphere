<div align="center">

<img width="239" height="231" alt="image" src="https://github.com/user-attachments/assets/1d753591-e03e-4ece-8a4a-50817cd4621c" />


# WorkSphere

**A modern desktop platform for workforce administration, project tracking, and resource allocation.**
Built with C++ and Qt, backed by Microsoft SQL Server.

![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)
![Qt](https://img.shields.io/badge/Qt-Widgets-41CD52?logo=qt&logoColor=white)
![SQL Server](https://img.shields.io/badge/SQL%20Server-Express-CC2927?logo=microsoftsqlserver&logoColor=white)
![Platform](https://img.shields.io/badge/platform-Windows-0078D6?logo=windows&logoColor=white)
![License](https://img.shields.io/badge/license-MIT-blue)

</div>

---

## Table of contents

- [Overview](#overview)
- [Features](#features)
- [Screenshots](#screenshots)
- [Tech stack](#tech-stack)
- [Architecture](#architecture)
- [Getting started](#getting-started)
- [Database setup](#database-setup)
- [Configuration](#configuration)
- [Usage](#usage)
- [Project structure](#project-structure)
- [Roadmap](#roadmap)
- [Contributing](#contributing)
- [License](#license)

## Overview

WorkSphere is a lightweight management tool for small teams. It keeps employees, projects and the
assignments between them in one place, with a live dashboard that summarizes headcount, active
projects and total payroll.

The interface is a modern, fully themed Qt Widgets UI with a dark and a light mode, a card-based
form layout and in-table search.

## Features

**Dashboard**
- Animated circular widgets for employee count, project count and total payroll
- Live search over employees (ID or name) and projects (name or assigned employee)
- Context-aware action bar: select an employee or project to reveal Edit / Delete

**Employees**
- Two employee types: **Worker** (with a position) and **Manager** (with a bonus)
- Create employees through a validated form
- Edit employees in a dedicated dialog with full input validation (the employee type is locked)
- Delete employees, with their project assignments cleaned up automatically

**Projects**
- Create projects with a name and a deadline
- Automatic **Active / Inactive** status derived from the deadline, color-coded in the table
- Edit and delete projects through dialogs
- Each project row lists the employees currently assigned to it

**Assignments**
- Assign any employee to any project, several people in a row
- Live "current team" panel for the selected project
- Duplicate assignments are detected and rejected with clear feedback

**Settings**
- Currency: EUR, USD, RSD, BAM (salaries are stored in USD and converted for display)
- Date format: `dd.MM.yyyy`, `yyyy-MM-dd`, `MM/dd/yyyy`
- Dark / Light theme, applied instantly and remembered between sessions

## Screenshots

| Dashboard (dark) | Dashboard (light) |
|---|---|
| <img width="1166" height="766" alt="image" src="https://github.com/user-attachments/assets/75115e14-bc1d-4c4e-ba1e-8c668fc101da" /> | <img width="1176" height="777" alt="image" src="https://github.com/user-attachments/assets/190590f6-daf3-4a7a-b2d8-858555ac86b0" /> |

| Edit employee | Assignments |
|---|---|
| <img width="1166" height="754" alt="image" src="https://github.com/user-attachments/assets/64e1f737-7f65-423c-abb4-9c92c821f4d9" />  | <img width="1159" height="756" alt="image" src="https://github.com/user-attachments/assets/798899f0-6e84-48f1-aa00-97447c467ba6" /> |

| Add employee | Add project |
|---|---|
| <img width="1155" height="761" alt="image" src="https://github.com/user-attachments/assets/d39931a0-daab-4b69-a4e8-4dd9ee12a80c" /> | <img width="1158" height="763" alt="image" src="https://github.com/user-attachments/assets/5d666e4c-5050-4f65-b240-23f9969dd169" /> |

## Tech stack

| Layer | Technology |
|---|---|
| Language | C++17 & TSQL | 
| UI | Qt Widgets (`QMainWindow`, `QStackedWidget`, `QTableWidget`, custom QSS theming) |
| Database access | Qt SQL (`QSqlQuery`) via ODBC |
| Database | Microsoft SQL Server (Express) |
| Settings | `QSettings` |

## Architecture

WorkSphere follows a simple layered structure:

- **Domain model** (`models/`): an `Employee` base class with `Worker` and `Manager` subclasses.
  Polymorphism (`dynamic_cast`) decides how each type is displayed, edited and counted in payroll.
- **Persistence**: a database layer exposing `loadEmployees`, `addWorker`, `addManager`,
  `updateEmployee` and `removeEmployee`. Projects and assignments use parameterized `QSqlQuery`
  statements directly.
- **UI** (`WorkSphereWindow`): a single main window with a sidebar and a stacked page per section.
  Forms are built in code and rebuilt on demand so lists always reflect the database.
- **Theming**: one palette struct and one stylesheet template with `%token%` placeholders, so the
  entire look of the app (including dialogs) switches from a single function.

## Getting started

### Prerequisites

- Windows 10 or 11
- [Qt 6](https://www.qt.io/download) (Qt 5.15 also works) with Qt Creator or CMake
- A C++17 compiler (MSVC 2019+ or MinGW)
- [SQL Server Express](https://www.microsoft.com/sql-server/sql-server-downloads)
- [ODBC Driver 17 for SQL Server](https://learn.microsoft.com/sql/connect/odbc/download-odbc-driver-for-sql-server)

### Build

```bash
git clone https://github.com/MIhajloS07/WorkSphere.git
cd WorkSphere
```

**With Qt Creator:** open the project file, pick a kit and press **Run**.

**With CMake:**

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="<path-to-Qt>"
cmake --build build --config Release
```
## Configuration

The connection string is defined in the `WorkSphereWindow` constructor:

```
DRIVER={ODBC Driver 17 for SQL Server};
SERVER=LOCALHOST\SQLEXPRESS;
DATABASE=WorkSphere;
Trusted_Connection=yes;
Encrypt=yes;
TrustServerCertificate=yes;
```

Change `SERVER` to match your instance. The app uses Windows authentication by default; to use a
SQL login, replace `Trusted_Connection=yes` with `UID=...;PWD=...;` (and keep credentials out of
version control).

User preferences (currency, date format, theme) are stored with `QSettings` under the
organization `WorkSphere` and application `WorkSphereApp`.

## Usage

1. **Dashboard**: review the totals, search, and select a row to edit or delete it.
2. **Employees**: add a worker or a manager. The last field switches between *Position* and *Bonus*.
3. **Projects**: create a project with a name and deadline; its status is calculated automatically.
4. **Assignments**: pick a project, then an employee, and press *Assign employee*.
5. **Settings**: switch theme, currency and date format.

To edit an employee, select the row on the dashboard and click **Edit employee**. A dialog opens
with the current values; invalid input keeps the dialog open and explains what to fix.

## Project structure

```
WorkSphere/
├── models/               # Employee, Worker, Manager, Project data models
├── widgetmodels/         # CircularProgressWidget
├── workspherewindow.h    # header file for worshperewindow 
├── workspherewindow.cpp  # Main window: UI, theming, handlers
├── workspherewindow.ui   # Qt Designer layout
├── logo.svg              # Application logo
├── CMakeLists.txt
├── .gitignore
├── .qtcreator
├── build
├── main.cpp              # Main class for call Application instance
└── database/             # SQL schema and model(recommended)
```

## Database schema
| Db Schema |
|---|
| <img width="605" height="485" alt="image" src="https://github.com/user-attachments/assets/d4a714e6-9e87-4a89-91d8-a293e2743c48" /> |

## Roadmap

- [ ] Real-time currency exchange rates
- [ ] Role-based login
- [ ] Export employees and projects to CSV / PDF
- [ ] Remove employees from a single project
- [ ] Unit tests for the domain model
- [ ] Linux and macOS support

## Contributing

Contributions are welcome.

1. Fork the repository
2. Create a branch: `git checkout -b feature/my-feature`
3. Commit your changes: `git commit -m "Add my feature"`
4. Push the branch: `git push origin feature/my-feature`
5. Open a pull request

## License

Distributed under the MIT License. See `LICENSE` for details.

---

<div align="center">
Made with C++ and Qt
</div>
