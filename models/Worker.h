#ifndef WORKER_H
#define WORKER_H
#include <string>
#include "Employee.h"

class Worker : public Employee {
private:
    std::string position;

public:
    Worker(std::string position, int id, std::string name, float salary);

    void setPosition(std::string position);
    std::string getPosition() const;

    void displayInfo() const override;
    void increaseSalary(float amount) override;
};
#endif // WORKER_H
