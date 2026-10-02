# SQL Server stored procedures for WorkSphere


``` sql
USE [WorkSphere]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[AddManager]
    @name NVARCHAR(100),
    @salary DECIMAL(10,2),
    @bonus DECIMAL(10,2)
AS
BEGIN
    SET NOCOUNT ON;
    DECLARE @newId INT;
    SELECT @newId = ISNULL(MAX(id), 0) + 1 FROM Employees;

    INSERT INTO Employees(id, name, salary, type, bonus) 
    VALUES(@newId, @name, @salary, 'MANAGER', @bonus)
END
```

``` sql
USE [WorkSphere]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[AddProject]
	@name NVARCHAR(100),
	@deadline DATE
AS
BEGIN
	SET NOCOUNT ON;
	INSERT INTO Projects(name, deadline)
	VALUES(@name, @deadline)
END
```

```sql
USE [WorkSphere]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[AddWorker]
    @name NVARCHAR(100),
    @salary DECIMAL(10,2),
    @position NVARCHAR(100)
AS
BEGIN
    SET NOCOUNT ON;
    
    DECLARE @newId INT;
    SELECT @newId = ISNULL(MAX(id), 0) + 1 FROM Employees;

    INSERT INTO Employees(id, name, salary, type, position) 
    VALUES (@newId, @name, @salary, 'WORKER', @position);
END
```

```sql
USE [WorkSphere]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[CreateEmployeesTable]
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
```

``` sql
USE [WorkSphere]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[CreateProjectsTable]
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
```

```sql
USE [WorkSphere]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[RemoveEmployee]
	@id INT
AS
BEGIN
	SET NOCOUNT ON;
	DELETE FROM Employees 
	WHERE id = @id;
END
```

``` sql
USE [WorkSphere]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[RemoveProject]
	@id INT
AS
BEGIN
	SET NOCOUNT ON;
	DELETE FROM Projects
	WHERE id = @id;
END
```

``` sql
USE [WorkSphere]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[UpdateEmployeeSalary]
	@id INT,
    @newSalary DECIMAL(10,2)
AS
BEGIN
	SET NOCOUNT ON;
	UPDATE Employees 
	SET salary = @newSalary
	WHERE id = @id;
END
```

``` sql
USE [WorkSphere]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[UpdateManager]
    @Id INT,
    @Name NVARCHAR(100),
    @Salary FLOAT,
    @Bonus FLOAT
AS
BEGIN
    UPDATE Employees 
    SET name = @Name, salary = @Salary, bonus = @Bonus 
    WHERE id = @Id;
END
```

``` sql
USE [WorkSphere]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE PROCEDURE [dbo].[UpdateWorker]
    @Id INT,
    @Name NVARCHAR(100),
    @Salary FLOAT,
    @Position NVARCHAR(100)
AS
BEGIN
    UPDATE Employees 
    SET name = @Name, salary = @Salary, position = @Position 
    WHERE id = @Id;
END
```
