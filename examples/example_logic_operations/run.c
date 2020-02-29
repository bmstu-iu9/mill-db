#include <stdio.h>
#include <time.h>
#include "db.c"
#include "db.h"

#define FILE_NAME "FILE_TEST"

int main() {
    // Write sample people with their name and ages to file.
    char people[4][100] = {"Ivan", "Sergey", "Nikolas", "Daniel"};
    db_open_write(FILE_NAME);
    add_person(0, people[0], 18);
    add_person(1, people[1], 19);
    add_person(2, people[2], 20);
    add_person(3, people[3], 18);
    db_close_write();
    struct db_handle *handle = db_open_read(FILE_NAME);

    // Test
    printf("age > 18:\n");
    struct get_people_name_older_than_age_out iter1;
    get_people_name_older_than_age_init(&iter1, handle, 18);
    while (get_people_name_older_than_age_next(&iter1)) {
        printf("%s\n", iter1.data.name);
    }
    printf("\n");

    // Test
    printf("id >= 0 and id < 3 and id > 0 and id <= 3:\n");
    struct get_people_name_with_id_out iter2;
    get_people_name_with_id_init(&iter2, handle, 0, 3, 0, 3);
    while (get_people_name_with_id_next(&iter2)) {
        printf("%s\n", iter2.data.name);
    }
    printf("\n");

    // Test
    printf("id <= 2:\n");
    struct get_people_name_with_id_2_out iter3;
    get_people_name_with_id_2_init(&iter3, handle, 2);
    while (get_people_name_with_id_2_next(&iter3)) {
        printf("%s\n", iter3.data.name);
    }
    printf("\n");

    // Test
    printf("not id >= 1 or (id = 1 and id >= 1):\n");
    struct get_people_name_with_id_3_out iter4;
    get_people_name_with_id_3_init(&iter4, handle, 1);
    while (get_people_name_with_id_3_next(&iter4)) {
        printf("%s\n", iter4.data.name);
    }
    printf("\n");

    db_close_read(handle);
    return 0;
}
