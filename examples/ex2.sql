CREATE TABLE Data (
	id int,
	value float,
	weight double
);

CREATE INDEX Data_PK on Data (id);

CREATE INDEX Data_value on Data (value, weight);
