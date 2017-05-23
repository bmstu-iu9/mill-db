CREATE TABLE Data (
	id int,
	value float,
	weight double,
	description str
);

CREATE INDEX Data_PK on Data (id);

CREATE INDEX Data_value on Data (value, weight);
