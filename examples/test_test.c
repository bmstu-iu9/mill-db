#include <stdio.h>
#include <time.h>
#include "sample.h"

int main() {
	clock_t begin = clock();
	sample_open_write("FILE");


	for (int i = 0; i < 20; i++) {
		char* str[20] = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u"};
		add_person(15, str[i]);
	}

//	add_person(0, "Sidney Crosby");
//	add_person(1, "Matt Duchene");
//	add_person(2, "Marc-Andre Fleury");
//	add_person(3, "Max Pacioretty");
//	add_person(4, "Corey Perry");
//	add_person(5, "Patric Hornqvist");
//	add_person(6, "George Parros");

	sample_close_write();

	struct sample_handle* handle1 = sample_open_read("FILE");
//	struct sample_handle* handle2 = sample_open_read("FILE");
//
	struct get_person_out iter1;
//	struct get_person_out iter2;
//
	get_person_init(&iter1, handle1, 15);
	while (get_person_next(&iter1))
		printf("%s\n", iter1.data.name);
//
	sample_close_read(handle1);
//	sample_close_read(handle2);


	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Duration=%f\n", time_spent);

	return 0;
}