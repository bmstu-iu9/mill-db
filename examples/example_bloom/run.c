#include <stdio.h>
#include <time.h>
#include "db.c"
#include "db.h"

#define FILE_NAME "db.db"

int main() {
   db_open_write(FILE_NAME);
   for (int i = 0; i < 100000; i++) {
      for (int j = 0; j < 10; j++) {
         add(i * 10 + j, j);
      }
   }
   db_close_write();
   struct db_handle *handle = db_open_read(FILE_NAME);

   int check[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
   int count = 0;
   struct timespec time_start, time_end;

   struct get_out iter1;
   clock_gettime(CLOCK_REALTIME, &time_start);
   get_init(&iter1, handle, 7);
   while (get_next(&iter1)) {
      check[iter1.data.oval]++;
      count++;
   }
   clock_gettime(CLOCK_REALTIME, &time_end);
   printf("Check 7: %d\n", check[7]);
   printf("Check 8: %d\n", check[8]);
   printf("Check 9: %d\n", check[9]);
   printf("Count: %d ", count);
   printf("Time: %d\n", time_end.tv_nsec - time_start.tv_nsec);

   db_close_read(handle);
   return 0;
}
