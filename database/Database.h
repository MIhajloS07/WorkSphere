#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <vector>
#include <memory>
#include <string>
#include "../models/Employee.h"
#include "../models/Worker.h"
#include "../models/Manager.h"
#include "../models/Project.h"

class Database {
private:
    QSqlDatabase db;
public:
    Database(const std::string& connStr);

    void createTables();

    void addWorker(const Worker& worker);
    void addManager(const Manager& manager);
    void addProject(const Project& project);

    void removeEmployee(int id);
    void removeProject(int id);

    void updateSalary(int id, float newSalary);

    std::vector<std::unique_ptr<Employee>> loadEmployees();
    std::vector<std::unique_ptr<Project>> loadProjects();
};

#endif // DATABASE_H
