#ifndef EMPLOYEE_H
#define EMPLOYEE_H
#include <string>

class Employee {
private:
    int id;

protected:
    std::string name;
    float salary;

public:
    Employee(int id, std::string name, float salary);
    virtual ~Employee();

    void setId(int id);
    void setName(std::string name);
    void setSalary(float salary);

    int getId() const;
    std::string getName() const;
    float getSalary() const;

    virtual void displayInfo() const;
    virtual void increaseSalary(float amount);
};
#endif // EMPLOYEE_H
