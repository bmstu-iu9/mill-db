#include <stdio.h>
#include <time.h>
#include "test_master.c"
#include "test_master.h"

int main() {
    char owners[4][6] = {"Maria", "Sasha", "Natal", "Tanya"};
    test_master_open_write("FILE2");
    add_owner_pet(100, "Maria", "Shuia");
    add_owner_pet(1001, "Sasha", "Kawai");
    add_owner_pet(4, "Natal", "Kazem");
    add_owner_pet(54, "Tanya", "Charl");
    test_master_close_write();
    struct test_master_handle *handle1 = test_master_open_read("FILE2");
    struct get_pet_by_owner_out iter1;
    int i;
    for (i = 0; i < 4; i++) {
        printf("%s\t", owners[i]);
        get_pet_by_owner_init(&iter1, handle1, owners[i]);
        if (get_pet_by_owner_next(&iter1))
            printf("%s\n", iter1.data.pname);
    }
    test_master_close_read(handle1);

    return 0;
}
