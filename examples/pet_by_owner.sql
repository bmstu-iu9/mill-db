CREATE SEQUENCE Pet_sequence;

CREATE TABLE owner (
	oid int pk,
	oname char(6),
	pet_id int
);

CREATE TABLE pet (
	pid int pk,
	pname char(6)
);

CREATE PROCEDURE add_owner_pet(@oid int in,@oname char(6) in,@pname char(6) in)
BEGIN
	INSERT TABLE owner VALUES (@oid, @oname,NEXTVAL(Pet_sequence));
	INSERT TABLE pet VALUES (CURRVAL(Pet_sequence),@pname);
END;

CREATE PROCEDURE get_pet_by_pid(@id int in, @pname char(6) out)
BEGIN
    SELECT pname SET @pname FROM pet WHERE pid=@id;
END;