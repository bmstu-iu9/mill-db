#ifndef TEST_LOGIC_H
#define TEST_LOGIC_H

#include <stdint.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

struct test_logic_handle;

void add_person(int32_t id, const char* name, int32_t age);

struct get_people_name_older_than_age_out_data {
	char name[101];
};

struct get_people_name_older_than_age_out_service {
	struct test_logic_handle* handle;
	struct get_people_name_older_than_age_out_data* set;
	int size;
	int length;
	int count;
};

struct get_people_name_older_than_age_out {
	struct get_people_name_older_than_age_out_service service;
	struct get_people_name_older_than_age_out_data data;
};

void get_people_name_older_than_age_init(struct get_people_name_older_than_age_out* iter, struct test_logic_handle* handle, int32_t age);
int get_people_name_older_than_age_next(struct get_people_name_older_than_age_out* iter);

struct get_people_name_with_id_out_data {
	char name[101];
};

struct get_people_name_with_id_out_service {
	struct test_logic_handle* handle;
	struct get_people_name_with_id_out_data* set;
	int size;
	int length;
	int count;
};

struct get_people_name_with_id_out {
	struct get_people_name_with_id_out_service service;
	struct get_people_name_with_id_out_data data;
};

void get_people_name_with_id_init(struct get_people_name_with_id_out* iter, struct test_logic_handle* handle, int32_t id1, int32_t id2, int32_t id3, int32_t id4);
int get_people_name_with_id_next(struct get_people_name_with_id_out* iter);

struct get_people_name_with_id_2_out_data {
	char name[101];
};

struct get_people_name_with_id_2_out_service {
	struct test_logic_handle* handle;
	struct get_people_name_with_id_2_out_data* set;
	int size;
	int length;
	int count;
};

struct get_people_name_with_id_2_out {
	struct get_people_name_with_id_2_out_service service;
	struct get_people_name_with_id_2_out_data data;
};

void get_people_name_with_id_2_init(struct get_people_name_with_id_2_out* iter, struct test_logic_handle* handle, int32_t id);
int get_people_name_with_id_2_next(struct get_people_name_with_id_2_out* iter);

void test_logic_open_write(const char* filename);
void test_logic_close_write(void);

struct test_logic_handle* test_logic_open_read(const char* filename);
void test_logic_close_read(struct test_logic_handle* handle);

#endif

