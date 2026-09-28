#include "Project.h"
#include <iostream>
#include <string>
#include <vector>
#include <QDate>

Project::Project(std::string name, QDate deadline) {
    this->name = name;
    this->deadline = deadline;
}

void Project::addEmployee(Employee* employee) {
    // Check whether an employee with the given ID already exists in the project
    for (size_t i = 0; i < employeesOnProject.size(); i++) {
        if (employeesOnProject[i]->getId() == employee->getId()) {
            std::cout << "Employee with ID " << employee->getId()
            << " already exists in the project!" << std::endl;
            return;
        }
    }

    employeesOnProject.push_back(employee);

    std::cout << "Employee with ID " << employee->getId()
              << " added to the project." << std::endl;
}

bool Project::removeEmployee(int id) {
    for (size_t i = 0; i < employeesOnProject.size(); i++) {
        if (employeesOnProject[i]->getId() == id) {
            employeesOnProject.erase(employeesOnProject.begin() + i);
            return true;
        }
    }

    return false; // Employee with the given ID was not found
}

void Project::displayProject() const {
    std::cout << "Project: " << this->name
              << ", deadline: " << this->deadline.toString("yyyy-MM-dd").toStdString() << std::endl;

    std::cout << "Employees on project: "
              << employeesOnProject.size() << std::endl;

    for (size_t i = 0; i < employeesOnProject.size(); i++) {
        employeesOnProject[i]->displayInfo();
    }
}

std::string Project::getName() const {
    return this->name;
}

QDate Project::getDeadline() const {
    return this->deadline;
}