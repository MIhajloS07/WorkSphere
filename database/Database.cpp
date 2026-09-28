#include <iostream>
#include "Database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

Database::Database(const std::string& connStr) {
    try {
        db = QSqlDatabase::addDatabase("QODBC");
        db.setDatabaseName(
            "DRIVER={ODBC Driver 18 for SQL Server};"
            "SERVER=localhost;"
            "DATABASE=WorkSphere;"
            "Trusted_Connection=yes;"
            "TrustServerCertificate=yes;"
        );
        if (!db.isOpen()) {
            throw std::runtime_error(
                "Error connecting to SQL server: " +
                db.lastError().text().toStdString()
            );
        }
        createTables();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}

void Database::createTables() {
    try {
        QSqlQuery query(db);
        // Execute CreateEmployeesTable stored procedure
        if (!query.exec("EXEC dbo.CreateEmployeesTable")){
            throw std::runtime_error(
                "Error executing CreateEmployeesTable stored procedure: " +
                query.lastError().text().toStdString()
            );
        }
        // Execute CreateProjectsTable stored procedure
        if (!query.exec("EXEC dbo.CreateProjectsTable")){
            throw std::runtime_error(
                "Error executing CreateProjectsTable stored procedure: " +
                query.lastError().text().toStdString()
            );
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error creating tables: "
                  << e.what() << std::endl;
    }
}

void Database::addWorker(const Worker& worker) {
    try {
        QSqlQuery query(db);
        query.prepare("EXEC dbo.AddWorker ?, ?, ?, ?");

        query.bindValue(0, worker.getId());
        query.bindValue(1, QString::fromStdString(worker.getName()));
        query.bindValue(2, worker.getSalary());
        query.bindValue(3, QString::fromStdString(worker.getPosition()));

        if (!query.exec()){
            throw std::runtime_error(
                "Error adding worker: " +
                query.lastError().text().toStdString()
            );
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding worker: "
                  << e.what() << std::endl;
    }
}

void Database::addManager(const Manager& manager) {
    try {
        QSqlQuery query(db);
        query.prepare("EXEC dbo.AddManager ?, ?, ?, ?");

        query.bindValue(0, manager.getId());
        query.bindValue(1, QString::fromStdString(manager.getName()));
        query.bindValue(2, manager.getSalary());
        query.bindValue(3, manager.getBonus());

        if (!query.exec()){
            throw std::runtime_error(
                "Error adding manager: " +
                query.lastError().text().toStdString()
            );
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error adding manager: "
                  << e.what() << std::endl;
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
        std::cerr << "Error adding project: "
                  << e.what() << std::endl;
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
        std::cerr << "Error removing employee: "
                  << e.what() << std::endl;
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
        std::cerr << "Error removing project: "
                  << e.what() << std::endl;
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
        std::cerr << "Error updating employee salary: "
                  << e.what() << std::endl;
    }
}

std::vector<std::unique_ptr<Employee>> Database::loadEmployees() {
    std::vector<std::unique_ptr<Employee>> employees;

    try {
        QSqlQuery query(db);
        query.prepare("SELECT * FROM Employees");

        if (!query.exec()) {
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
                // Add in employees vector
                employees.push_back(
                    std::make_unique<Worker>(
                        position,
                        id,
                        name,
                        salary
                    )
                );
            }
            else if (type == "MANAGER") {
                float bonus = query.value("bonus").toFloat();
                // Add in employees vector
                employees.push_back(
                    std::make_unique<Manager>(
                        id,
                        name,
                        salary,
                        bonus
                    )
                );
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading employees: "
                  << e.what() << std::endl;
    }
    return employees;
}

std::vector<std::unique_ptr<Project>> Database::loadProjects() {
    std::vector<std::unique_ptr<Project>> projects;

    try {
        QSqlQuery query(db);
        query.prepare("SELECT * FROM Projects");

        if (!query.exec()) {
            throw std::runtime_error(
                "Error loading projects: " +
                query.lastError().text().toStdString()
                );
        }

        while (query.next()) {
            std::string name =
                query.value("name").toString().toStdString();

            QDate deadline =
                query.value("deadline").toDate();

            projects.push_back(
                std::make_unique<Project>(
                    name,
                    deadline
                )
            );
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading projects: "
                  << e.what() << std::endl;
    }
    return projects;
}