#include "Worker.h"
#include <iostream>
#include <string>

Worker::Worker(std::string position, int id, std::string name, float salary)
    : Employee(id, name, salary) {
    this->position = position;
}

void Worker::setPosition(std::string position) {
    this->position = position;
}

std::string Worker::getPosition() const {
    return this->position;
}

void Worker::displayInfo() const {
    Employee::displayInfo();
    std::cout << " | Position: " << this->position;
}

void Worker::increaseSalary(float amount) {
    float tax = amount * 0.15f;
    salary += (amount - tax);
}