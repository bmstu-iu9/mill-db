#include <stdio.h>
#include <stdlib.h>
#include "sample.h"

int main() {
	sample_open("Persons.out");

	add_position(0, "Employee");
	add_position(1, "Manager");
	add_position(2, "TerritorialManager");

	add_person(0, "Steve Mason", 27, 0);
	add_person(1, "Sidney Crosby", 32, 1);
	add_person(2, "Antti Niemi", 30, 1);
	add_person(3, "Mike Modano", 45, 2);

	struct get_person_by_id_out_struct* iter;

	get_person_by_id_init(&iter, 2);
	while (get_person_by_id_next(&iter))
		printf("%s\n", iter->name);

	sample_close();

	return 0;
}
