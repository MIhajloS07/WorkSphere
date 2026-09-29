# SQL Server stored procedures for WorkSphere


``` sql
-- PROJECTS
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE CreateProjectsTable
AS
BEGIN
        SET NOCOUNT ON;
        IF NOT EXISTS (
                SELECT *
        FROM sys.tables
        WHERE name = 'Projects'
        )
        BEGIN
                CREATE TABLE Projects(
                        id INT IDENTITY(1,1) PRIMARY KEY,
                        name NVARCHAR(100) NOT NULL,
                        deadline DATE NOT NULL
                );
        END
END;
GO
```
``` sql
--EMPLOYEES
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE CreateEmployeesTable
AS
BEGIN
        SET NOCOUNT ON;
        IF NOT EXISTS (
                SELECT *
        FROM sys.tables
        WHERE name = 'Employees'
        )
        BEGIN
                CREATE TABLE Employees(
                        id INT PRIMARY KEY,
                        name NVARCHAR(100) NOT NULL,
                        salary DECIMAL(10, 2) NOT NULL,
                        type NVARCHAR(20) NOT NULL,
                        position NVARCHAR(100),
                        bonus DECIMAL(10,2)
                );
        END
END;
GO
```

```sql
--CREATE WORKER
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE AddWorker
    @id INT,
    @name NVARCHAR(100),
    @salary DECIMAL(10,2),
    @position NVARCHAR(100)
AS
BEGIN
        SET NOCOUNT ON;
    INSERT INTO Employees(id, name, salary, type, position) 
    VALUES (@id, @name, @salary, 'WORKER', @position);
END
GO
```

```sql
--CREATE MANAGER
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE AddManager
    @id INT,
    @name NVARCHAR(100),
    @salary DECIMAL(10,2),
    @bonus DECIMAL(10,2)
AS
BEGIN
        SET NOCOUNT ON;
    INSERT INTO Employees(id, name, salary, type, bonus)
    VALUES(@id, @name, @salary, 'MANAGER', @bonus)
END
GO
```

``` sql
--CREATE PROJECT
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE AddProject
        @name NVARCHAR(100),
        @deadline DATE
AS
BEGIN
        SET NOCOUNT ON;
        INSERT INTO Projects(name, deadline)
        VALUES(@name, @deadline)
END
GO
```

```sql
--DELETE EMPLOYEE
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE RemoveEmployee
        @id INT
AS
BEGIN
        SET NOCOUNT ON;
        DELETE FROM Employees 
        WHERE id = @id;
END
GO
```

``` sql
--DELETE PROJECT
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE RemoveProject
        @id INT
AS
BEGIN
        SET NOCOUNT ON;
        DELETE FROM Projects
        WHERE id = @id;
END
GO
```

``` sql
--UPDATE EMPLOYEE SALARY
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE UpdateEmployeeSalary
    @id INT,
    @newSalary DECIMAL(10,2)
AS
BEGIN
        SET NOCOUNT ON;
        UPDATE Employees 
        SET salary = @newSalary
        WHERE id = @id;
END
GO
```
