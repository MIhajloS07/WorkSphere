#include "Manager.h"
#include "Employee.h"
#include <iostream>
#include <string>

Manager::Manager(int id, std::string name, float salary, float bonus)
    : Employee(id, name, salary) {
    this->bonus = bonus;
}

void Manager::setBonus(float bonus) {
    this->bonus = bonus;
}

float Manager::getBonus() const {
    return this->bonus;
}

void Manager::displayInfo() const {
    Employee::displayInfo();
    std::cout << " | Bonus: " << this->bonus;
}

void Manager::increaseSalary(float amount) {
    salary += (amount + this->bonus);
}