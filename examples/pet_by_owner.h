#ifndef PET_BY_OWNER_H
#define PET_BY_OWNER_H

#include <stdint.h>

struct pet_by_owner_handle;

uint64_t Pet_sequence = 0 ;
void add_owner_pet(int32_t oid, const char* oname, const char* pname);

struct get_pet_by_owner_out_data {
	char pname[7];
};

struct get_pet_by_owner_out_service {
	struct pet_by_owner_handle* handle;
	struct get_pet_by_owner_out_data* set;
	int size;
	int length;
	int count;
};

struct get_pet_by_owner_out {
	struct get_pet_by_owner_out_service service;
	struct get_pet_by_owner_out_data data;
};

void get_pet_by_owner_init(struct get_pet_by_owner_out* iter, struct pet_by_owner_handle* handle, const char* oname);
int get_pet_by_owner_next(struct get_pet_by_owner_out* iter);

void pet_by_owner_open_write(const char* filename);
void pet_by_owner_close_write(void);

struct pet_by_owner_handle* pet_by_owner_open_read(const char* filename);
void pet_by_owner_close_read(struct pet_by_owner_handle* handle);

#endif

