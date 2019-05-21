#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "test_logic.h"

#ifndef PAGE_SIZE
	#define PAGE_SIZE 4096
#endif

#define MILLDB_BUFFER_INIT_SIZE 32

struct MILLDB_buffer_info {
	uint64_t size;
	uint64_t count;
};

#define person_header_count 0

struct MILLDB_header {
	uint64_t count[1];
	uint64_t data_offset[1];
	uint64_t index_offset[1];
};

#define MILLDB_HEADER_SIZE (sizeof(struct MILLDB_header))

struct person_tree_item {
	int32_t key;
	uint64_t offset;
};

struct person_node {
	struct person_tree_item data;
	struct person_node** childs;
	uint64_t n;
};

#define MILLDB_FILE_MODE_CLOSED -1
#define MILLDB_FILE_MODE_READ 0
#define MILLDB_FILE_MODE_WRITE 1

struct test_logic_handle {
	FILE* file;
	int mode;
	struct MILLDB_header* header;

	struct person_node* person_root;
};

struct person {
	int32_t id;
	int32_t age;
	char name[100];
};

struct person_tree_item* person_tree_item_new() {
	struct person_tree_item* new = malloc(sizeof(struct person_tree_item));
	memset(new, 0, sizeof(*new));
	return new;
}

void person_tree_item_free(struct person_tree_item* deleted) {
	free(deleted);
	return;
}

#define person_CHILDREN (PAGE_SIZE / sizeof(struct person_tree_item))

union person_page {
	struct person items[person_CHILDREN];
	uint8_t as_bytes[PAGE_SIZE];
};

int person_compare(struct person* s1, struct person* s2) {
	if ((s1->id > s2->id))
		return 1;
	else if ((s1->id < s2->id))
		return -1;

	if ((s1->age > s2->age))
		return 1;
	else if ((s1->age < s2->age))
		return -1;

	if (strncmp(s1->name, s2->name, 100) > 0)
		return 1;
	else if (strncmp(s1->name, s2->name, 100) < 0)
		return -1;

	return 0;
}

struct person* person_new() {
	struct person* new = malloc(sizeof(struct person));
	memset(new, 0, sizeof(*new));
	return new;
}

void person_free(struct person* deleted) {
	free(deleted);
	return;
}

struct person** person_buffer = NULL;
struct MILLDB_buffer_info person_buffer_info;

void person_buffer_init() {
	person_buffer_info.size = MILLDB_BUFFER_INIT_SIZE;
	person_buffer_info.count = 0;
	person_buffer = calloc(person_buffer_info.size, sizeof(struct person));
}

void person_buffer_add(struct person* inserted) {
	if (person_buffer_info.count >= person_buffer_info.size) {
		person_buffer_info.size *= 2;
		person_buffer = realloc(person_buffer, person_buffer_info.size * sizeof(struct person*));
	}
	person_buffer[person_buffer_info.count++] = inserted;
}

void person_buffer_free() {
	if (person_buffer == NULL)
		return;

	for (uint64_t i = 0; i < person_buffer_info.count; i++) {
		person_free(person_buffer[i]);
	}
	free(person_buffer);
}

int person_sort_compare(const void* a, const void* b) {
	return person_compare((struct person*)a, (struct person*)b);
}

uint64_t person_write(FILE* file) {
	qsort(person_buffer, person_buffer_info.count, sizeof(struct person*), person_sort_compare);
	for (uint64_t i = 0; i < person_buffer_info.count; i++) {
		fwrite(person_buffer[i], sizeof(struct person), 1, file);
	}

	uint64_t page_size = person_CHILDREN, items = 0;
	while (page_size < person_buffer_info.count) {
		for (uint64_t i = 0; i < person_buffer_info.count; i += page_size) {
			struct person_tree_item *item = person_tree_item_new();
			item->key = person_buffer[i]->id;
			item->offset = i * sizeof(struct person);
			fwrite(item, sizeof(struct person_tree_item), 1, file);
			items++;
			person_tree_item_free(item);
		}
		page_size *= person_CHILDREN;
	}

	struct person_tree_item *item = person_tree_item_new();
	item->key = person_buffer[0]->id;
	item->offset = 0;
	fwrite(item, sizeof(struct person_tree_item), 1, file);
	person_tree_item_free(item);
	return items + 1;
}

void person_index_clean(struct person_node* node) {
	if (node == NULL)
		return;

	for (uint64_t i = 0; i < node->n; i++)
		person_index_clean(node->childs[i]);

	if (node->childs)
		free(node->childs);
	free(node);
}

void person_index_load(struct test_logic_handle* handle) {
	if (handle->header->count[person_header_count] == 0) {
		handle->person_root = NULL;
		return;
	}
	int32_t levels = log(handle->header->count[person_header_count]) / log(person_CHILDREN) + 1;
	uint64_t previous_level_count = 0, count = 0;
	struct person_node** previous_level = NULL;
	struct person_node** current_level = NULL;

	for (int32_t level = 1; level <= levels; level++) {
		uint64_t current_level_count = (handle->header->count[person_header_count] + pow(person_CHILDREN, level) - 1) / pow(person_CHILDREN, level);
		current_level = calloc(current_level_count, sizeof(struct person_node*));

		for (uint64_t i = 0; i < current_level_count; i++) {
			fseek(handle->file, handle->header->index_offset[person_header_count] + (count++) * sizeof(struct person_tree_item), SEEK_SET);
			struct person_tree_item* current_tree_item = malloc(sizeof(struct person_tree_item));
			uint64_t size = fread(current_tree_item, sizeof(struct person_tree_item), 1, handle->file);  if (size == 0) return;
			current_level[i] = malloc(sizeof(struct person_node));
			memcpy(&(current_level[i]->data), current_tree_item, sizeof(struct person_tree_item));
			free(current_tree_item);
		}

		for (uint64_t i = 0; i < current_level_count; i++) {
			if (!previous_level) {
				current_level[i]->childs = NULL;
				current_level[i]->n = 0;
				continue;
			}

			current_level[i]->childs = calloc(person_CHILDREN, sizeof(struct person_node*));
			uint64_t j;
			for (j = 0; j < person_CHILDREN; j++) {
				uint64_t k = i * person_CHILDREN + j;
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
	handle->person_root = current_level[0];
	free(current_level);
}

void add_person_1(int32_t id, int32_t age, const char* name) {
	struct person* inserted = person_new();
	inserted->id = id;
	inserted->age = age;
	memcpy(inserted->name, name, 100);
	person_buffer_add(inserted);
}

void add_person(int32_t id, const char* name, int32_t age) {
	add_person_1(id, age, name);
}

void get_people_not_equal_age_1_add(struct get_people_not_equal_age_1_out* iter, struct get_people_not_equal_age_1_out_data* selected) {
	struct get_people_not_equal_age_1_out_service* service = &(iter->service);
	if (service->set == NULL) {
		service->size = MILLDB_BUFFER_INIT_SIZE;
		service->set = calloc(service->size, sizeof(struct get_people_not_equal_age_1_out));
	}
	if (service->length >= service->size) {
		service->size = service->size * 2;
		service->set = realloc(service->set, service->size * sizeof(struct get_people_not_equal_age_1_out));
	}
	memcpy(&(service->set[service->length++]), selected, sizeof(struct get_people_not_equal_age_1_out_data));
}

void get_people_not_equal_age_1_1(struct get_people_not_equal_age_1_out* iter, int32_t age) {
//table person	cond: age <> @age
	struct test_logic_handle* handle = iter->service.handle;
	struct get_people_not_equal_age_1_out_data* inserted = malloc(sizeof(struct get_people_not_equal_age_1_out_data));
//TABLE person
	uint64_t offset = 0;

	offset += handle->header->data_offset[person_header_count];
	
	while (1) {
		fseek(handle->file, offset, SEEK_SET);
		union person_page page;
		uint64_t size = fread(&page, sizeof(struct person), person_CHILDREN, handle->file);  if (size == 0) return;

		for (uint64_t i = 0; i < person_CHILDREN; i++) {
			const char* p_name= page.items[i].name;
			int32_t c_id= page.items[i].id;
			int32_t c_age= page.items[i].age;
			const char* c_name= page.items[i].name;
			if (offset + i * sizeof(struct person) >= handle->header->index_offset[person_header_count]) {
				free(inserted);
				return;
			}
			if (1) {
				if (!(c_age != age))
					continue;

				memcpy(inserted->name, c_name, 100); inserted->name[100] = '\0';
				get_people_not_equal_age_1_add(iter, inserted);
			}
		}
		offset += person_CHILDREN * sizeof(struct person);
	}

}

void get_people_not_equal_age_1_init(struct get_people_not_equal_age_1_out* iter, struct test_logic_handle* handle, int32_t age) {
	memset(iter, 0, sizeof(*iter));
	iter->service.handle = handle;
	iter->service.set = NULL;
	iter->service.size = 0;
	iter->service.count = 0;
	iter->service.length = 0;

	get_people_not_equal_age_1_1(iter, age);
}

int get_people_not_equal_age_1_next(struct get_people_not_equal_age_1_out* iter) {
	if (iter == NULL)
		return 0;

	struct get_people_not_equal_age_1_out_service* service = &(iter->service);

	if (service->set != NULL && service->count < service->length) {
		memcpy(&iter->data, &(service->set[service->count]), sizeof(struct get_people_not_equal_age_1_out_data));
		service->count++;
		return 1;
	} else {
		free(service->set);
	}

	return 0;
}

void get_people_not_equal_age_2_add(struct get_people_not_equal_age_2_out* iter, struct get_people_not_equal_age_2_out_data* selected) {
	struct get_people_not_equal_age_2_out_service* service = &(iter->service);
	if (service->set == NULL) {
		service->size = MILLDB_BUFFER_INIT_SIZE;
		service->set = calloc(service->size, sizeof(struct get_people_not_equal_age_2_out));
	}
	if (service->length >= service->size) {
		service->size = service->size * 2;
		service->set = realloc(service->set, service->size * sizeof(struct get_people_not_equal_age_2_out));
	}
	memcpy(&(service->set[service->length++]), selected, sizeof(struct get_people_not_equal_age_2_out_data));
}

void get_people_not_equal_age_2_1(struct get_people_not_equal_age_2_out* iter, int32_t age) {
//table person	cond: NOT age = @age
	struct test_logic_handle* handle = iter->service.handle;
	struct get_people_not_equal_age_2_out_data* inserted = malloc(sizeof(struct get_people_not_equal_age_2_out_data));
//TABLE person
	uint64_t offset = 0;

	offset += handle->header->data_offset[person_header_count];
	
	while (1) {
		fseek(handle->file, offset, SEEK_SET);
		union person_page page;
		uint64_t size = fread(&page, sizeof(struct person), person_CHILDREN, handle->file);  if (size == 0) return;

		for (uint64_t i = 0; i < person_CHILDREN; i++) {
			const char* p_name= page.items[i].name;
			int32_t c_id= page.items[i].id;
			int32_t c_age= page.items[i].age;
			const char* c_name= page.items[i].name;
			if (offset + i * sizeof(struct person) >= handle->header->index_offset[person_header_count]) {
				free(inserted);
				return;
			}
			if (1) {
				if ((c_age == age))
					continue;

				memcpy(inserted->name, c_name, 100); inserted->name[100] = '\0';
				get_people_not_equal_age_2_add(iter, inserted);
			}
		}
		offset += person_CHILDREN * sizeof(struct person);
	}

}

void get_people_not_equal_age_2_init(struct get_people_not_equal_age_2_out* iter, struct test_logic_handle* handle, int32_t age) {
	memset(iter, 0, sizeof(*iter));
	iter->service.handle = handle;
	iter->service.set = NULL;
	iter->service.size = 0;
	iter->service.count = 0;
	iter->service.length = 0;

	get_people_not_equal_age_2_1(iter, age);
}

int get_people_not_equal_age_2_next(struct get_people_not_equal_age_2_out* iter) {
	if (iter == NULL)
		return 0;

	struct get_people_not_equal_age_2_out_service* service = &(iter->service);

	if (service->set != NULL && service->count < service->length) {
		memcpy(&iter->data, &(service->set[service->count]), sizeof(struct get_people_not_equal_age_2_out_data));
		service->count++;
		return 1;
	} else {
		free(service->set);
	}

	return 0;
}

void get_people_older_or_same_age_add(struct get_people_older_or_same_age_out* iter, struct get_people_older_or_same_age_out_data* selected) {
	struct get_people_older_or_same_age_out_service* service = &(iter->service);
	if (service->set == NULL) {
		service->size = MILLDB_BUFFER_INIT_SIZE;
		service->set = calloc(service->size, sizeof(struct get_people_older_or_same_age_out));
	}
	if (service->length >= service->size) {
		service->size = service->size * 2;
		service->set = realloc(service->set, service->size * sizeof(struct get_people_older_or_same_age_out));
	}
	memcpy(&(service->set[service->length++]), selected, sizeof(struct get_people_older_or_same_age_out_data));
}

void get_people_older_or_same_age_1(struct get_people_older_or_same_age_out* iter, int32_t age) {
//table person	cond: age >= @age
	struct test_logic_handle* handle = iter->service.handle;
	struct get_people_older_or_same_age_out_data* inserted = malloc(sizeof(struct get_people_older_or_same_age_out_data));
//TABLE person
	uint64_t offset = 0;

	offset += handle->header->data_offset[person_header_count];
	
	while (1) {
		fseek(handle->file, offset, SEEK_SET);
		union person_page page;
		uint64_t size = fread(&page, sizeof(struct person), person_CHILDREN, handle->file);  if (size == 0) return;

		for (uint64_t i = 0; i < person_CHILDREN; i++) {
			const char* p_name= page.items[i].name;
			int32_t c_id= page.items[i].id;
			int32_t c_age= page.items[i].age;
			const char* c_name= page.items[i].name;
			if (offset + i * sizeof(struct person) >= handle->header->index_offset[person_header_count]) {
				free(inserted);
				return;
			}
			if (1) {
				if (!(c_age >= age))
					continue;

				memcpy(inserted->name, c_name, 100); inserted->name[100] = '\0';
				get_people_older_or_same_age_add(iter, inserted);
			}
		}
		offset += person_CHILDREN * sizeof(struct person);
	}

}

void get_people_older_or_same_age_init(struct get_people_older_or_same_age_out* iter, struct test_logic_handle* handle, int32_t age) {
	memset(iter, 0, sizeof(*iter));
	iter->service.handle = handle;
	iter->service.set = NULL;
	iter->service.size = 0;
	iter->service.count = 0;
	iter->service.length = 0;

	get_people_older_or_same_age_1(iter, age);
}

int get_people_older_or_same_age_next(struct get_people_older_or_same_age_out* iter) {
	if (iter == NULL)
		return 0;

	struct get_people_older_or_same_age_out_service* service = &(iter->service);

	if (service->set != NULL && service->count < service->length) {
		memcpy(&iter->data, &(service->set[service->count]), sizeof(struct get_people_older_or_same_age_out_data));
		service->count++;
		return 1;
	} else {
		free(service->set);
	}

	return 0;
}

void get_people_older_than_age_add(struct get_people_older_than_age_out* iter, struct get_people_older_than_age_out_data* selected) {
	struct get_people_older_than_age_out_service* service = &(iter->service);
	if (service->set == NULL) {
		service->size = MILLDB_BUFFER_INIT_SIZE;
		service->set = calloc(service->size, sizeof(struct get_people_older_than_age_out));
	}
	if (service->length >= service->size) {
		service->size = service->size * 2;
		service->set = realloc(service->set, service->size * sizeof(struct get_people_older_than_age_out));
	}
	memcpy(&(service->set[service->length++]), selected, sizeof(struct get_people_older_than_age_out_data));
}

void get_people_older_than_age_1(struct get_people_older_than_age_out* iter, int32_t age) {
//table person	cond: age > @age
	struct test_logic_handle* handle = iter->service.handle;
	struct get_people_older_than_age_out_data* inserted = malloc(sizeof(struct get_people_older_than_age_out_data));
//TABLE person
	uint64_t offset = 0;

	offset += handle->header->data_offset[person_header_count];
	
	while (1) {
		fseek(handle->file, offset, SEEK_SET);
		union person_page page;
		uint64_t size = fread(&page, sizeof(struct person), person_CHILDREN, handle->file);  if (size == 0) return;

		for (uint64_t i = 0; i < person_CHILDREN; i++) {
			const char* p_name= page.items[i].name;
			int32_t c_id= page.items[i].id;
			int32_t c_age= page.items[i].age;
			const char* c_name= page.items[i].name;
			if (offset + i * sizeof(struct person) >= handle->header->index_offset[person_header_count]) {
				free(inserted);
				return;
			}
			if (1) {
				if (!(c_age > age))
					continue;

				memcpy(inserted->name, c_name, 100); inserted->name[100] = '\0';
				get_people_older_than_age_add(iter, inserted);
			}
		}
		offset += person_CHILDREN * sizeof(struct person);
	}

}

void get_people_older_than_age_init(struct get_people_older_than_age_out* iter, struct test_logic_handle* handle, int32_t age) {
	memset(iter, 0, sizeof(*iter));
	iter->service.handle = handle;
	iter->service.set = NULL;
	iter->service.size = 0;
	iter->service.count = 0;
	iter->service.length = 0;

	get_people_older_than_age_1(iter, age);
}

int get_people_older_than_age_next(struct get_people_older_than_age_out* iter) {
	if (iter == NULL)
		return 0;

	struct get_people_older_than_age_out_service* service = &(iter->service);

	if (service->set != NULL && service->count < service->length) {
		memcpy(&iter->data, &(service->set[service->count]), sizeof(struct get_people_older_than_age_out_data));
		service->count++;
		return 1;
	} else {
		free(service->set);
	}

	return 0;
}

void get_people_younger_or_same_age_add(struct get_people_younger_or_same_age_out* iter, struct get_people_younger_or_same_age_out_data* selected) {
	struct get_people_younger_or_same_age_out_service* service = &(iter->service);
	if (service->set == NULL) {
		service->size = MILLDB_BUFFER_INIT_SIZE;
		service->set = calloc(service->size, sizeof(struct get_people_younger_or_same_age_out));
	}
	if (service->length >= service->size) {
		service->size = service->size * 2;
		service->set = realloc(service->set, service->size * sizeof(struct get_people_younger_or_same_age_out));
	}
	memcpy(&(service->set[service->length++]), selected, sizeof(struct get_people_younger_or_same_age_out_data));
}

void get_people_younger_or_same_age_1(struct get_people_younger_or_same_age_out* iter, int32_t age) {
//table person	cond: age <= @age
	struct test_logic_handle* handle = iter->service.handle;
	struct get_people_younger_or_same_age_out_data* inserted = malloc(sizeof(struct get_people_younger_or_same_age_out_data));
//TABLE person
	uint64_t offset = 0;

	offset += handle->header->data_offset[person_header_count];
	
	while (1) {
		fseek(handle->file, offset, SEEK_SET);
		union person_page page;
		uint64_t size = fread(&page, sizeof(struct person), person_CHILDREN, handle->file);  if (size == 0) return;

		for (uint64_t i = 0; i < person_CHILDREN; i++) {
			const char* p_name= page.items[i].name;
			int32_t c_id= page.items[i].id;
			int32_t c_age= page.items[i].age;
			const char* c_name= page.items[i].name;
			if (offset + i * sizeof(struct person) >= handle->header->index_offset[person_header_count]) {
				free(inserted);
				return;
			}
			if (1) {
				if (!(c_age <= age))
					continue;

				memcpy(inserted->name, c_name, 100); inserted->name[100] = '\0';
				get_people_younger_or_same_age_add(iter, inserted);
			}
		}
		offset += person_CHILDREN * sizeof(struct person);
	}

}

void get_people_younger_or_same_age_init(struct get_people_younger_or_same_age_out* iter, struct test_logic_handle* handle, int32_t age) {
	memset(iter, 0, sizeof(*iter));
	iter->service.handle = handle;
	iter->service.set = NULL;
	iter->service.size = 0;
	iter->service.count = 0;
	iter->service.length = 0;

	get_people_younger_or_same_age_1(iter, age);
}

int get_people_younger_or_same_age_next(struct get_people_younger_or_same_age_out* iter) {
	if (iter == NULL)
		return 0;

	struct get_people_younger_or_same_age_out_service* service = &(iter->service);

	if (service->set != NULL && service->count < service->length) {
		memcpy(&iter->data, &(service->set[service->count]), sizeof(struct get_people_younger_or_same_age_out_data));
		service->count++;
		return 1;
	} else {
		free(service->set);
	}

	return 0;
}

void get_people_younger_than_age_add(struct get_people_younger_than_age_out* iter, struct get_people_younger_than_age_out_data* selected) {
	struct get_people_younger_than_age_out_service* service = &(iter->service);
	if (service->set == NULL) {
		service->size = MILLDB_BUFFER_INIT_SIZE;
		service->set = calloc(service->size, sizeof(struct get_people_younger_than_age_out));
	}
	if (service->length >= service->size) {
		service->size = service->size * 2;
		service->set = realloc(service->set, service->size * sizeof(struct get_people_younger_than_age_out));
	}
	memcpy(&(service->set[service->length++]), selected, sizeof(struct get_people_younger_than_age_out_data));
}

void get_people_younger_than_age_1(struct get_people_younger_than_age_out* iter, int32_t age) {
//table person	cond: age < @age
	struct test_logic_handle* handle = iter->service.handle;
	struct get_people_younger_than_age_out_data* inserted = malloc(sizeof(struct get_people_younger_than_age_out_data));
//TABLE person
	uint64_t offset = 0;

	offset += handle->header->data_offset[person_header_count];
	
	while (1) {
		fseek(handle->file, offset, SEEK_SET);
		union person_page page;
		uint64_t size = fread(&page, sizeof(struct person), person_CHILDREN, handle->file);  if (size == 0) return;

		for (uint64_t i = 0; i < person_CHILDREN; i++) {
			const char* p_name= page.items[i].name;
			int32_t c_id= page.items[i].id;
			int32_t c_age= page.items[i].age;
			const char* c_name= page.items[i].name;
			if (offset + i * sizeof(struct person) >= handle->header->index_offset[person_header_count]) {
				free(inserted);
				return;
			}
			if (1) {
				if (!(c_age < age))
					continue;

				memcpy(inserted->name, c_name, 100); inserted->name[100] = '\0';
				get_people_younger_than_age_add(iter, inserted);
			}
		}
		offset += person_CHILDREN * sizeof(struct person);
	}

}

void get_people_younger_than_age_init(struct get_people_younger_than_age_out* iter, struct test_logic_handle* handle, int32_t age) {
	memset(iter, 0, sizeof(*iter));
	iter->service.handle = handle;
	iter->service.set = NULL;
	iter->service.size = 0;
	iter->service.count = 0;
	iter->service.length = 0;

	get_people_younger_than_age_1(iter, age);
}

int get_people_younger_than_age_next(struct get_people_younger_than_age_out* iter) {
	if (iter == NULL)
		return 0;

	struct get_people_younger_than_age_out_service* service = &(iter->service);

	if (service->set != NULL && service->count < service->length) {
		memcpy(&iter->data, &(service->set[service->count]), sizeof(struct get_people_younger_than_age_out_data));
		service->count++;
		return 1;
	} else {
		free(service->set);
	}

	return 0;
}

struct test_logic_handle* test_logic_write_handle = NULL;

void test_logic_open_write(const char* filename) {
	FILE* file;
	if (!(file = fopen(filename, "wb")))
		return;

	test_logic_write_handle = malloc(sizeof(struct test_logic_handle));
	test_logic_write_handle->file = file;
	test_logic_write_handle->mode = MILLDB_FILE_MODE_WRITE;

	person_buffer_init();
}

int test_logic_save(struct test_logic_handle* handle) {
	if (handle && handle->mode == MILLDB_FILE_MODE_WRITE) {
		struct MILLDB_header* header = malloc(sizeof(struct MILLDB_header));
		fseek(handle->file, MILLDB_HEADER_SIZE, SEEK_SET);

		uint64_t person_index_count = 0;
		if (person_buffer_info.count > 0)
			person_index_count = person_write(handle->file);

		uint64_t offset = MILLDB_HEADER_SIZE;

		header->count[person_header_count] = person_buffer_info.count;
		header->data_offset[person_header_count] = offset;
		offset += person_buffer_info.count * sizeof(struct person);
		header->index_offset[person_header_count] = offset;
		offset += person_index_count * sizeof(struct person_tree_item);


		fseek(handle->file, 0, SEEK_SET);
		fwrite(header, MILLDB_HEADER_SIZE, 1, handle->file);
		free(header);
	}
	return 0;
}

void test_logic_close_write(void) {
	if (test_logic_write_handle == NULL)
		return;

	test_logic_save(test_logic_write_handle);

	person_buffer_free();

	fclose(test_logic_write_handle->file);
	free(test_logic_write_handle);
}

struct test_logic_handle* test_logic_open_read(const char* filename) {
	FILE* file;
	if (!(file = fopen(filename, "rb")))
		return NULL;

	struct test_logic_handle* handle = malloc(sizeof(struct test_logic_handle));
	handle->file = file;
	handle->mode = MILLDB_FILE_MODE_READ;

	fseek(handle->file, 0, SEEK_SET);
	struct MILLDB_header* header = malloc(MILLDB_HEADER_SIZE);
	uint64_t size = fread(header, MILLDB_HEADER_SIZE, 1, handle->file);  if (size == 0) return NULL;
	handle->header = header;

	person_index_load(handle);

	return handle;
}

void test_logic_close_read(struct test_logic_handle* handle) {
	if (handle == NULL)
		return;

	fclose(handle->file);

	if (handle->person_root)
		person_index_clean(handle->person_root);


	free(handle->header);
	free(handle);
}

