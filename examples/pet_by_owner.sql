CREATE SEQUENCE Pet_sequence;

CREATE TABLE owner (
	oid int pk,
	oname text,
	pet_id int
);

CREATE TABLE pet (
	pid int pk,
	pname text
);

CREATE PROCEDURE add_owner_pet(@oid int in,@oname text in,@pname text in)
BEGIN
	INSERT TABLE owner VALUES (@oid, @oname,NEXTVAL(Pet_sequence));
	INSERT TABLE pet VALUES (CURRVAL(Pet_sequence),@pname);
END;

CREATE PROCEDURE get_pet_by_pid(@id int in, @pname text out)
BEGIN
    SELECT pname SET @pname FROM pet WHERE pid=@id;
END;