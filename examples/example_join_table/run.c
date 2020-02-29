#include <stdio.h>
#include <time.h>
#include "db.c"
#include "db.h"

int main() {
    char owners[4][6] = {"Maria", "Sasha", "Natal", "Tanya"};
    db_open_write("FILE2");
    add_owner_pet(100, "Maria", "Shuia");
    add_owner_pet(1001, "Sasha", "Kawai");
    add_owner_pet(4, "Natal", "Kazem");
    add_owner_pet(54, "Tanya", "Charl");
    db_close_write();
    struct db_handle *handle1 = db_open_read("FILE2");
    struct get_pet_by_owner_out iter1;
    int i;
    for (i = 0; i < 4; i++) {
        printf("%s\t", owners[i]);
        get_pet_by_owner_init(&iter1, handle1, owners[i]);
        if (get_pet_by_owner_next(&iter1))
            printf("%s\n", iter1.data.pname);
    }
    db_close_read(handle1);

    return 0;
}
