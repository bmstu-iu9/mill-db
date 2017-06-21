#ifndef SAMPLE_H
#define SAMPLE_H

void add_person(int32_t id, char name[101], int32_t age, int32_t position);

void add_position(int32_t id, char name[21]);

struct get_person_by_id_out_struct {
	char name[101];
	int32_t age;
	int32_t position;
};

int get_person_by_id_init(struct get_person_by_id_out_struct** iter, int32_t id);
int get_person_by_id_next(struct get_person_by_id_out_struct** iter);

int sample_open(char* filename);
int sample_close();

#endif

