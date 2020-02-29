CREATE TABLE person (
    id int pk,
    age int,
    name char(100)
);

CREATE PROCEDURE add_person(@id int in, @name char(100) in, @age int in)
BEGIN
    INSERT TABLE person VALUES (@id, @age, @name);
END;

CREATE PROCEDURE get_people_name_older_than_age(@age int in, @name char(100) out)
BEGIN
    SELECT name SET @name FROM person WHERE age > @age;
END;

CREATE PROCEDURE get_people_name_with_id(@id1 int in, @id2 int in, @id3 int in, @id4 int in, @name char(100) out)
BEGIN
    SELECT name SET @name FROM person WHERE id >= @id1 AND id < @id2 AND id > @id3 AND id <= @id4;
END;

CREATE PROCEDURE get_people_name_with_id_2(@id int in, @name char(100) out)
BEGIN
    SELECT name SET @name FROM person WHERE id <= @id;
END;

CREATE PROCEDURE get_people_name_with_id_3(@id int in, @name char(100) out)
BEGIN
    SELECT name SET @name FROM person WHERE (NOT id >= @id) OR (id = @id AND id >= @id);
END;
