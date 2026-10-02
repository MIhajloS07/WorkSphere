#include <iostream>
#include "Database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

Database::Database(const std::string& connStr) {
    db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName(QString::fromStdString(connStr));

    if (!db.isOpen()) {
        if (!db.open()) {
            throw std::runtime_error(
                "Error connecting to SQL server: " +
                db.lastError().text().toStdString()
            );
        }
    }
    createTables();
}

void Database::createTables() {
    QSqlQuery query(db);

    if (!query.exec("EXEC dbo.CreateEmployeesTable")) {
        std::cerr << "Upozorenje / Greška pri kreiranju tabele Employees: "
                  << query.lastError().text().toStdString() << std::endl;
    }

    if (!query.exec("EXEC dbo.CreateProjectsTable")) {
        std::cerr << "Upozorenje / Greška pri kreiranju tabele Projects: "
                  << query.lastError().text().toStdString() << std::endl;
    }
}

void Database::addWorker(const Worker& worker) {
    try {
        QSqlQuery query(db);
        query.prepare("EXEC dbo.AddWorker ?, ?, ?");

        query.bindValue(0, QString::fromStdString(worker.getName()));
        query.bindValue(1, worker.getSalary());
        query.bindValue(2, QString::fromStdString(worker.getPosition()));

        if (!query.exec()){
            throw std::runtime_error("Error adding worker: " + query.lastError().text().toStdString());
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding worker: " << e.what() << std::endl;
    }
}

void Database::addManager(const Manager& manager) {
    try {
        QSqlQuery query(db);
        query.prepare("EXEC dbo.AddManager ?, ?, ?");

        query.bindValue(0, QString::fromStdString(manager.getName()));
        query.bindValue(1, manager.getSalary());
        query.bindValue(2, manager.getBonus());

        if (!query.exec()){
            throw std::runtime_error(
                "Error adding manager: " +
                query.lastError().text().toStdString()
                );
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding manager: " << e.what() << std::endl;
    }
}

void Database::addProject(const Project& project) {
    try {
        QSqlQuery query(db);
        query.prepare("EXEC dbo.AddProject ?, ?");

        query.bindValue(0, QString::fromStdString(project.getName()));
        query.bindValue(1, project.getDeadline());

        if (!query.exec()){
            throw std::runtime_error(
                "Error adding project: " +
                query.lastError().text().toStdString()
                );
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding project: " << e.what() << std::endl;
    }
}

void Database::removeEmployee(int id) {
    try {
        QSqlQuery query(db);
        query.prepare("EXEC dbo.RemoveEmployee ?");
        query.bindValue(0, id);

        if (!query.exec()){
            throw std::runtime_error(
                "Error removing employee: " +
                query.lastError().text().toStdString()
                );
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error removing employee: " << e.what() << std::endl;
    }
}

void Database::removeProject(int id) {
    try {
        QSqlQuery query(db);
        query.prepare("EXEC dbo.RemoveProject ?");
        query.bindValue(0, id);

        if (!query.exec()){
            throw std::runtime_error(
                "Error removing project: " +
                query.lastError().text().toStdString()
                );
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error removing project: " << e.what() << std::endl;
    }
}

void Database::updateSalary(int id, float newSalary) {
    try {
        QSqlQuery query(db);
        query.prepare("EXEC dbo.UpdateEmployeeSalary ?, ?");
        query.bindValue(0, id);
        query.bindValue(1, newSalary);

        if (!query.exec()){
            throw std::runtime_error(
                "Error updating employee salary: " +
                query.lastError().text().toStdString()
                );
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error updating employee salary: " << e.what() << std::endl;
    }
}

std::vector<std::unique_ptr<Employee>> Database::loadEmployees() {
    std::vector<std::unique_ptr<Employee>> employees;

    try {
        QSqlQuery query(db);
        if (!query.exec("SELECT id, name, salary, type, position, bonus FROM Employees")) {
            throw std::runtime_error(
                "Error loading employees: " +
                query.lastError().text().toStdString()
                );
        }

        while (query.next()) {
            int id = query.value("id").toInt();
            std::string name = query.value("name").toString().toStdString();
            float salary = query.value("salary").toFloat();
            std::string type = query.value("type").toString().toStdString();

            if (type == "WORKER") {
                std::string position = query.value("position").toString().toStdString();
                employees.push_back(
                    std::make_unique<Worker>(position, id, name, salary)
                    );
            }
            else if (type == "MANAGER") {
                float bonus = query.value("bonus").toFloat();
                employees.push_back(
                    std::make_unique<Manager>(id, name, salary, bonus)
                    );
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading employees: " << e.what() << std::endl;
    }
    return employees;
}

std::vector<std::unique_ptr<Project>> Database::loadProjects() {
    std::vector<std::unique_ptr<Project>> projects;

    try {
        QSqlQuery query(db);
        if (!query.exec("SELECT name, deadline FROM Projects")) {
            throw std::runtime_error(
                "Error loading projects: " +
                query.lastError().text().toStdString()
                );
        }

        while (query.next()) {
            std::string name = query.value("name").toString().toStdString();
            QDate deadline = query.value("deadline").toDate();

            projects.push_back(
                std::make_unique<Project>(name, deadline)
                );
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading projects: " << e.what() << std::endl;
    }
    return projects;
}

void Database::updateEmployee(const Employee& employee) {
    try {
        QSqlQuery query(db);

        // Provjeravamo da li je u pitanju Menadžer ili Radnik
        if (auto manager = dynamic_cast<const Manager*>(&employee)) {
            query.prepare("EXEC dbo.UpdateManager ?, ?, ?, ?");
            query.bindValue(0, manager->getId());
            query.bindValue(1, QString::fromStdString(manager->getName()));
            query.bindValue(2, manager->getSalary());
            query.bindValue(3, manager->getBonus());
        }
        else if (auto worker = dynamic_cast<const Worker*>(&employee)) {
            query.prepare("EXEC dbo.UpdateWorker ?, ?, ?, ?");
            query.bindValue(0, worker->getId());
            query.bindValue(1, QString::fromStdString(worker->getName()));
            query.bindValue(2, worker->getSalary());
            query.bindValue(3, QString::fromStdString(worker->getPosition()));
        }

        if (!query.exec()) {
            throw std::runtime_error(
                "Error updating employee: " +
                query.lastError().text().toStdString()
                );
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error updating employee: " << e.what() << std::endl;
        throw; // Prosljeđujemo grešku dalje ka UI-ju kako bi QMessageBox mogao da je prikaže
    }
}

void Database::updateProject(int id, const std::string& name, const QDate& deadline) {
    try {
        QSqlQuery query(db);
        query.prepare("EXEC dbo.UpdateProject ?, ?, ?");
        query.bindValue(0, id);
        query.bindValue(1, QString::fromStdString(name));
        query.bindValue(2, deadline);

        if (!query.exec()) {
            throw std::runtime_error(
                "Error updating project: " +
                query.lastError().text().toStdString()
                );
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error updating project: " << e.what() << std::endl;
        throw;
    }
}