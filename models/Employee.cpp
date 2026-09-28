#include <iostream>
#include "Employee.h"
#include <string>

Employee::Employee(int id, std::string name, float salary) {
    this->id = id;
    this->name = name;
    this->salary = salary;
}

Employee::~Employee() = default;

void Employee::setId(int id) {
    this->id = id;
}

void Employee::setName(std::string name) {
    this->name = name;
}

void Employee::setSalary(float salary) {
    this->salary = salary;
}

int Employee::getId() const {
    return this->id;
}

std::string Employee::getName() const {
    return this->name;
}

float Employee::getSalary() const {
    return this->salary;
}

void Employee::displayInfo() const {
    std::cout << "\n == Basic Information ==" << std::endl;
    std::cout << "ID: " << this->id
              << " | Name: " << this->name
              << " | Salary: " << this->salary << " RSD";
}

void Employee::increaseSalary(float amount) {
    this->salary += amount;
}