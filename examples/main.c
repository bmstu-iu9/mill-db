#include <stdio.h>
#include <time.h>
#include "sample.h"

int main() {
	clock_t begin = clock();
	sample_open_write("FILE");


	for (int i = 0; i < 70000; i++) {
		add_person(70000-i, "sid7");
	}

//	add_person(0, "Sidney Crosby");
//	add_person(1, "Matt Duchene");
//	add_person(2, "Marc-Andre Fleury");
//	add_person(3, "Max Pacioretty");
//	add_person(4, "Corey Perry");
//	add_person(5, "Patric Hornqvist");
//	add_person(6, "George Parros");

	sample_close_write();
	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Duration=%f\n", time_spent);

//	struct sample_handle* handle1 = sample_open_read("FILE");
//	struct sample_handle* handle2 = sample_open_read("FILE");
//
//	struct get_person_out iter1;
//	struct get_person_out iter2;
//
//	get_person_init(&iter1, handle1, 34);
//	while (get_person_next(&iter1))
//		printf("%s\n", iter1.data.name);
//
//	sample_close_read(handle1);
//	sample_close_read(handle2);

	return 0;
}