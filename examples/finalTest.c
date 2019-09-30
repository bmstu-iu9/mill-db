#include <stdio.h>
#include <time.h>
#include "pet_by_owner.h"

int main() {
    int owners[6] = {5, 2, 1, 4, 3, 6};
    pet_by_owner_open_write("FILE2");
    add_owner_pet(1, "Vova",  "Ni");
    add_owner_pet(2, "Petya", "Chi");
    add_owner_pet(3, "Maria", "Shuia");
    add_owner_pet(4, "Sasha", "Kawai");
    add_owner_pet(5, "Natal", "Kazem");
    add_owner_pet(6, "Tanya", "Charl");
    pet_by_owner_close_write();
    struct pet_by_owner_handle *handle1 = pet_by_owner_open_read("FILE2");
    struct get_pet_by_pid_out iter1;
    int i;
    for (i = 0; i < 6; i++) {
        printf("%d\n", owners[i]);
        get_pet_by_pid_init(&iter1, handle1, owners[i]);

        int flag = 1;
        while (get_pet_by_pid_next(&iter1)) {
            printf("%s\n", iter1.data.out);
            flag = 0;
        }

        if (flag) {
            printf("\n");
        }
    }
    pet_by_owner_close_read(handle1);

    return 0;
}