#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "sample.h"

#ifndef PAGE_SIZE
	#define PAGE_SIZE 4096 
#endif

#define MILLDB_BUFFER_INIT_SIZE 32

struct MILLDB_buffer_info {
	uint64_t size;
	uint64_t count;
};

struct MILLDB_header {
	uint64_t count;
	uint64_t data_offset;
	uint64_t index_offset;
};

#define MILLDB_HEADER_SIZE (sizeof(struct MILLDB_header))

struct Person {
	int32_t id;
	char name[4];
};

struct Person_tree_item {
	int32_t key;
	uint64_t offset;
};

struct Person_tree_item* Person_tree_item_new() {
	struct Person_tree_item* new = malloc(sizeof(struct Person_tree_item));
	memset(new, 0, sizeof(*new));
	return new;
}

void Person_tree_item_free(struct Person_tree_item* deleted) {
	free(deleted);
	return;
}
#define Person_CHILDREN (PAGE_SIZE / sizeof(struct Person_tree_item))
//#define Person_CHILDREN (3)

union Person_page
{
	struct Person_tree_item items[Person_CHILDREN];
	uint8_t as_bytes[PAGE_SIZE];
};

int Person_compare(struct Person* s1, struct Person* s2) {
	if (s1->id > s2->id)
		return 1;
	else if (s1->id < s2->id)
		return -1;

	if (strncmp(s1->name, s1->name, 32) == 1)
		return 1;
	else if (strncmp(s1->name, s1->name, 32) == -1)
		return -1;

	return 0;
}

struct Person* Person_new() {
	struct Person* new = malloc(sizeof(struct Person));
	memset(new, 0, sizeof(*new));
	return new;
}

void Person_free(struct Person* deleted) {
	free(deleted);
	return;
}

struct Person** Person_buffer = NULL;
struct MILLDB_buffer_info Person_buffer_info;

void Person_buffer_init() {
	Person_buffer_info.size = MILLDB_BUFFER_INIT_SIZE;
	Person_buffer_info.count = 0;
	Person_buffer = calloc(Person_buffer_info.size, sizeof(struct Person));
}

void Person_buffer_add(struct Person* inserted) {
	if (Person_buffer_info.count >= Person_buffer_info.size) {
		Person_buffer_info.size *= 2;
		Person_buffer = realloc(Person_buffer, Person_buffer_info.size * sizeof(struct Person*));
	}
	Person_buffer[Person_buffer_info.count++] = inserted;
}

void Person_buffer_free() {
	if (Person_buffer == NULL)
		return;

	for (uint64_t i = 0; i < Person_buffer_info.count; i++) {
		Person_free(Person_buffer[i]);
	}
	free(Person_buffer);
}

uint64_t Person_partition(struct Person** buffer, uint64_t low, uint64_t high) {
	if (buffer == NULL)
		return -1;

	uint64_t pivot = (high - low) >> 1;
	uint64_t i = low, j = high;
	while (i < j) {
		while (Person_compare(buffer[i], buffer[pivot]) < 0)
			i++;
		while (Person_compare(buffer[j], buffer[pivot]) > 0)
			j--;
		if (i < j) {
			struct Person* temp = Person_new();
			memcpy(temp, buffer[i], sizeof(struct Person));
			memcpy(buffer[i], buffer[j], sizeof(struct Person));
			memcpy(buffer[j], temp, sizeof(struct Person));
			Person_free(temp);
		}
	}
	return i + 1;
}

void Person_sort(struct Person** buffer, uint64_t low, uint64_t high) {
	if (buffer == NULL)
		return;

//	printf("sorting %ld %ld\n", low, high);

	if (low < high) {
		uint64_t p = Person_partition(buffer, low, high);
		Person_sort(buffer, low, p-1);
		Person_sort(buffer, p, high);
	}
}

void Person_write(FILE* file) {
	Person_sort(Person_buffer, 0, Person_buffer_info.count - 1);

	for (uint64_t i = 0; i < Person_buffer_info.count; i++) {
		fwrite(Person_buffer[i], sizeof(struct Person), 1, file);
	}

	uint64_t page_size = Person_CHILDREN;
	while (page_size < Person_buffer_info.count) {
		for (uint64_t i = 0; i < Person_buffer_info.count; i += page_size) {
			struct Person_tree_item *item = Person_tree_item_new();

			item->key = Person_buffer[i]->id;
			item->offset = i * sizeof(struct Person);

			fwrite(item, sizeof(struct Person_tree_item), 1, file);

			Person_tree_item_free(item);
		}
		page_size *= Person_CHILDREN;
	}

	struct Person_tree_item *item = Person_tree_item_new();
	item->key = Person_buffer[0]->id;
	item->offset = 0;
	fwrite(item, sizeof(struct Person_tree_item), 1, file);
	Person_tree_item_free(item);
}

struct Person_Node {
	struct Person_tree_item data;
	struct Person_Node** childs;
	uint64_t n;

};

#define MILLDB_FILE_MODE_CLOSED -1
#define MILLDB_FILE_MODE_READ 0
#define MILLDB_FILE_MODE_WRITE 1

uint32_t sample_handle_counter = 0;

struct sample_handle {
	uint32_t id;
	FILE* file;
	int mode;

	struct MILLDB_header* header;

	struct Person_Node* Person_root;
};

struct sample_handle* sample_write_handle = NULL;

void sample_open_write(const char* filename) {
	FILE* file;
	if (!(file = fopen(filename, "wb")))
		return;

	sample_write_handle = malloc(sizeof(struct sample_handle));
	sample_write_handle->id = sample_handle_counter++;
	sample_write_handle->file = file;
	sample_write_handle->mode = MILLDB_FILE_MODE_WRITE;

	Person_buffer_init();
}


int sample_save(struct sample_handle* handle) {
	if (handle == NULL)
		return 0;

	if (handle->mode == MILLDB_FILE_MODE_WRITE) {

		// Prepare and write header
		struct MILLDB_header* header = malloc(sizeof(struct MILLDB_header));
		header->count = Person_buffer_info.count;
		header->data_offset = MILLDB_HEADER_SIZE;
		header->index_offset = header->data_offset + Person_buffer_info.count * sizeof(struct Person);
		fwrite(header, MILLDB_HEADER_SIZE, 1, handle->file);
		free(header);

		if (Person_buffer_info.count > 0)
			Person_write(handle->file);
	} else
		printf("sample_save(): fail\n");

	return 0;
}

void Person_index_clean(struct Person_Node* node) {
	if (node == NULL)
		return;

	for (uint64_t i = 0; i < node->n; i++)
		Person_index_clean(node->childs[i]);

	if (node->childs)
		free(node->childs);
	free(node);
}

void sample_close_write(void) {
	if (sample_write_handle == NULL)
		return;

	sample_save(sample_write_handle);
	Person_buffer_free();

	fclose(sample_write_handle->file);
	free(sample_write_handle);
}

struct sample_handle* sample_open_read(const char* filename) {
	FILE* file;
	if (!(file = fopen(filename, "rb")))
		return NULL;

	struct sample_handle* handle = malloc(sizeof(struct sample_handle));
	handle->id = sample_handle_counter++;
	handle->file = file;
	handle->mode = MILLDB_FILE_MODE_READ;

	fseek(handle->file, 0, SEEK_SET);
	struct MILLDB_header* header = malloc(MILLDB_HEADER_SIZE);
	fread(header, MILLDB_HEADER_SIZE, 1, handle->file);
	handle->header = header;

	int32_t levels = log(header->count) / log(Person_CHILDREN) + 1;
	struct Person_Node** previous_level = NULL;
	uint64_t previous_level_count;
	struct Person_Node** current_level = NULL;
	uint64_t current_level_count;
	uint64_t big_count_all = 0;

	for (int32_t level = 1; level <= levels; level++) {
		uint64_t current_level_count = (header->count + pow(Person_CHILDREN, level) - 1) / pow(Person_CHILDREN, level);

		current_level = calloc(current_level_count, sizeof(struct Person_Node*));
		for (uint64_t i = 0; i < current_level_count; i++) {
			fseek(handle->file, handle->header->index_offset + (big_count_all++) * sizeof(struct Person_tree_item), SEEK_SET);
			struct Person_tree_item* current_tree_item = malloc(sizeof(struct Person_tree_item));
			fread(current_tree_item, sizeof(struct Person_tree_item), 1, handle->file);
			current_level[i] = malloc(sizeof(struct Person_Node));
			memcpy(&(current_level[i]->data), current_tree_item, sizeof(struct Person_tree_item));
			free(current_tree_item);
		}

		for (uint64_t i = 0; i < current_level_count; i++) {
			if (!previous_level) {
				current_level[i]->childs = NULL;
				current_level[i]->n = 0;
				continue;
			}

			current_level[i]->childs = calloc(Person_CHILDREN, sizeof(struct Person_Node*));
			uint64_t j;
			for (j = 0; j < Person_CHILDREN; j++) {
				uint64_t k = i * Person_CHILDREN + j;
				if (k == previous_level_count)
					break;
				current_level[i]->childs[j] = previous_level[k];
			}
			current_level[i]->n = j;
		}

		if (previous_level)
			free(previous_level);

		previous_level = current_level;
		previous_level_count = current_level_count;
	}

	handle->Person_root = current_level[0];
	free(current_level);

	return handle;
}

void sample_close_read(struct sample_handle* handle) {
	if (handle == NULL)
		return;

	fclose(handle->file);

	if (handle->Person_root) {
		Person_index_clean(handle->Person_root);
	}

	free(handle->header);
	free(handle);
}

void add_person_1(int32_t id, const char* name) {
	struct Person* inserted = Person_new();

	inserted->id = id;
	memcpy(inserted->name, name, 4);

	// Just add it to storage in a row
	Person_buffer_add(inserted);
}

void add_person(int32_t id, const char* name) {
	add_person_1(id, name);
}


/*
 *  CREATE PROCEDURE get_person(@id int in, @name char(4) out)
 *  BEGIN
 *      SELECT name SET @name FROM Person WHERE id = @id;
 *  END;
 */
void get_person_1(struct get_person_out* iter, int32_t id) {

}

void get_person_init(struct get_person_out* iter, struct sample_handle* handle, int32_t id) {
	memset(iter, 0, sizeof(*iter));
	iter->handle = handle;
	iter->set = NULL;
	iter->size = 0;
	iter->count = 0;

	get_person_1(iter, id);
}

int get_person_next(struct get_person_out* iter) {
	if (iter == NULL)
		return 0;
	struct sample_handle* handle = iter->handle;

	if (handle == NULL)
		return 0;

	if (iter->set != NULL && iter->count < iter->size) {
		memcpy(&iter->data, iter->set, sizeof(struct get_person_out_data));
		iter->count++;
		return 1;
	} else {
		free(iter->set);
	}

	return 0;
}