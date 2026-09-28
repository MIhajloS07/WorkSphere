#ifndef MANAGER_H
#define MANAGER_H
#include "Employee.h"
class Manager : public Employee {
private:
    float bonus;
public:
    Manager(int id, std::string name, float salary, float bonus);
    void setBonus(float bonus);
    float getBonus() const;
    void displayInfo() const override;
    void increaseSalary(float amount) override;
};
#endif // MANAGER_H
