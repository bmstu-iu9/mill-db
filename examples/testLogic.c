#include <stdio.h>
#include <time.h>
#include "test_logic.h"
#include "test_logic.c"

int main() {
	// Write sample people with their name and ages to file.
	char people[4][100] = {"Ivan", "Sergey", "Nikolas", "Daniel"};
	test_logic_open_write("FILE_TEST");
	add_person(1, people[0], 18);
	add_person(2, people[1], 19);
	add_person(3, people[2], 20);
	add_person(4, people[3], 18);
	test_logic_close_write();
	struct test_logic_handle* handle = test_logic_open_read("FILE_TEST");
	// Test '<'
	printf("age < 20:\n");
	struct get_people_younger_than_age_out iter1;
	get_people_younger_than_age_init(&iter1, handle, 20);
    while (get_people_younger_than_age_next(&iter1)) {
		printf("%s\n", iter1.data.name);
	}
	printf("\n");
	// Test '>'
	printf("age > 20:\n");
	struct get_people_older_than_age_out iter2;
	get_people_older_than_age_init(&iter2, handle, 20);
    while (get_people_older_than_age_next(&iter2)) {
		printf("%s\n", iter2.data.name);
	}
	printf("\n");
	// Test '<='
	printf("age <= 20:\n");
	struct get_people_younger_or_same_age_out iter3;
	get_people_younger_or_same_age_init(&iter3, handle, 20);
    while (get_people_younger_or_same_age_next(&iter3)) {
		printf("%s\n", iter3.data.name);
	}
	printf("\n");
	// Test '>='
	printf("age >= 20:\n");
	struct get_people_older_or_same_age_out iter4;
	get_people_older_or_same_age_init(&iter4, handle, 20);
    while (get_people_older_or_same_age_next(&iter4)) {
		printf("%s\n", iter4.data.name);
	}
	printf("\n");
	// Test '<>'
	printf("age <> 20:\n");
	struct get_people_not_equal_age_1_out iter5;
	get_people_not_equal_age_1_init(&iter5, handle, 20);
    while (get_people_not_equal_age_1_next(&iter5)) {
		printf("%s\n", iter5.data.name);
	}
	printf("\n");
	// Test 'NOT'
	printf("NOT age = 20:\n");
	struct get_people_not_equal_age_2_out iter6;
	get_people_not_equal_age_2_init(&iter6, handle, 20);
    while (get_people_not_equal_age_2_next(&iter6)) {
		printf("%s\n", iter6.data.name);
	}
	test_logic_close_read(handle);
	return 0;
}
