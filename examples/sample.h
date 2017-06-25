#ifndef SAMPLE_H
#define SAMPLE_H

#include <stdint.h>

void sample_open_write(const char* filename);
void sample_close_write(void);

struct sample_handle;

struct sample_handle* sample_open_read(const char* filename);
void sample_close_read(struct sample_handle*);

void add_person(int32_t id, const char* name);

struct get_person_out_data {
	int32_t id;
	char name[33];
};

struct get_person_out {
	struct sample_handle* handle;
	struct get_person_out_data* set;
	int size;
	int count;
	struct get_person_out_data data;
};

void get_person_init(struct get_person_out* iter, struct sample_handle* handle, int id);
int get_person_next(struct get_person_out* iter);

#endif