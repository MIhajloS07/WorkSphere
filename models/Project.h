#ifndef PROJECT_H
#define PROJECT_H
#include "Employee.h"
#include <string>
#include <vector>
#include <QDate>

class Project {
private:
    std::string name;
    QDate deadline;
    std::vector<Employee*> employeesOnProject;

public:
    Project(std::string name, QDate deadline);

    void addEmployee(Employee* employee);
    bool removeEmployee(int id);

    void displayProject() const;

    std::string getName() const;
    QDate getDeadline() const;
};
#endif // PROJECT_H
