#include <stdio.h>
#include <stdlib.h>
#include "sample.h"

int main() {
	sample_open_write("FILE");

	add_person(0, "Sidney Crosby");
	add_person(1, "Matt Duchene");
	add_person(2, "Marc-Andre Fleury");
	add_person(3, "Max Pacioretty");
	add_person(4, "Corey Perry");
	add_person(5, "Patrick Hornquist");
	add_person(6, "George Parros");

	sample_close_write();

	struct sample_handle* handle1 = sample_open_read("FILE");
	struct sample_handle* handle2 = sample_open_read("FILE");

	struct get_person_out iter1;
	struct get_person_out iter2;

	get_person_init(&iter1, handle1, 4);
	while (get_person_next(&iter1))
		printf("%s\n", iter1.data.name);

	get_person_init(&iter2, handle2, 2);
	while (get_person_next(&iter2))
		printf("%s\n", iter2.data.name);

	sample_close_read(handle1);
	sample_close_read(handle2);

	return 0;
}