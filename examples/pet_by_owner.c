#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "pet_by_owner.h"

#ifndef PAGE_SIZE
	#define PAGE_SIZE 4096
#endif

#define MILLDB_BUFFER_INIT_SIZE 32

struct MILLDB_buffer_info {
	uint64_t size;
	uint64_t count;
};

#define owner_header_count 0
#define pet_header_count 1

struct MILLDB_header {
	uint64_t count[2];
	uint64_t data_offset[2];
	uint64_t index_offset[2];
};

#define MILLDB_HEADER_SIZE (sizeof(struct MILLDB_header))

struct owner_tree_item {
	int32_t key;
	uint64_t offset;
};

struct owner_node {
	struct owner_tree_item data;
	struct owner_node** childs;
	uint64_t n;
};

struct pet_tree_item {
	int32_t key;
	uint64_t offset;
};

struct pet_node {
	struct pet_tree_item data;
	struct pet_node** childs;
	uint64_t n;
};

#define MILLDB_FILE_MODE_CLOSED -1
#define MILLDB_FILE_MODE_READ 0
#define MILLDB_FILE_MODE_WRITE 1

struct pet_by_owner_handle {
	FILE* file;
	int mode;
	struct MILLDB_header* header;

	struct owner_node* owner_root;
	struct pet_node* pet_root;
};

struct owner {
	int32_t oid;
	char oname[6];
	int32_t pet_id;
};

struct owner_tree_item* owner_tree_item_new() {
	struct owner_tree_item* new = malloc(sizeof(struct owner_tree_item));
	memset(new, 0, sizeof(*new));
	return new;
}

void owner_tree_item_free(struct owner_tree_item* deleted) {
	free(deleted);
	return;
}

#define owner_CHILDREN (PAGE_SIZE / sizeof(struct owner_tree_item))

union owner_page {
	struct owner items[owner_CHILDREN];
	uint8_t as_bytes[PAGE_SIZE];
};

int owner_compare(struct owner* s1, struct owner* s2) {
	if ((s1->oid > s2->oid))
		return 1;
	else if ((s1->oid < s2->oid))
		return -1;

	if (strncmp(s1->oname, s2->oname, 6) > 0)
		return 1;
	else if (strncmp(s1->oname, s2->oname, 6) < 0)
		return -1;

	if ((s1->pet_id > s2->pet_id))
		return 1;
	else if ((s1->pet_id < s2->pet_id))
		return -1;

	return 0;
}

struct owner* owner_new() {
	struct owner* new = malloc(sizeof(struct owner));
	memset(new, 0, sizeof(*new));
	return new;
}

void owner_free(struct owner* deleted) {
	free(deleted);
	return;
}

struct owner** owner_buffer = NULL;
struct MILLDB_buffer_info owner_buffer_info;

void owner_buffer_init() {
	owner_buffer_info.size = MILLDB_BUFFER_INIT_SIZE;
	owner_buffer_info.count = 0;
	owner_buffer = calloc(owner_buffer_info.size, sizeof(struct owner));
}

void owner_buffer_add(struct owner* inserted) {
	if (owner_buffer_info.count >= owner_buffer_info.size) {
		owner_buffer_info.size *= 2;
		owner_buffer = realloc(owner_buffer, owner_buffer_info.size * sizeof(struct owner*));
	}
	owner_buffer[owner_buffer_info.count++] = inserted;
}

void owner_buffer_free() {
	if (owner_buffer == NULL)
		return;

	for (uint64_t i = 0; i < owner_buffer_info.count; i++) {
		owner_free(owner_buffer[i]);
	}
	free(owner_buffer);
}

int owner_sort_compare(const void* a, const void* b) {
	return owner_compare((struct owner*)a, (struct owner*)b);
}

uint64_t owner_write(FILE* file) {
	qsort(owner_buffer, owner_buffer_info.count, sizeof(struct owner*), owner_sort_compare);
	for (uint64_t i = 0; i < owner_buffer_info.count; i++) {
		fwrite(owner_buffer[i], sizeof(struct owner), 1, file);
	}

	uint64_t page_size = owner_CHILDREN, items = 0;
	while (page_size < owner_buffer_info.count) {
		for (uint64_t i = 0; i < owner_buffer_info.count; i += page_size) {
			struct owner_tree_item *item = owner_tree_item_new();
			item->key = owner_buffer[i]->oid;
			item->offset = i * sizeof(struct owner);
			fwrite(item, sizeof(struct owner_tree_item), 1, file);
			items++;
			owner_tree_item_free(item);
		}
		page_size *= owner_CHILDREN;
	}

	struct owner_tree_item *item = owner_tree_item_new();
	item->key = owner_buffer[0]->oid;
	item->offset = 0;
	fwrite(item, sizeof(struct owner_tree_item), 1, file);
	owner_tree_item_free(item);
	return items + 1;
}

void owner_index_clean(struct owner_node* node) {
	if (node == NULL)
		return;

	for (uint64_t i = 0; i < node->n; i++)
		owner_index_clean(node->childs[i]);

	if (node->childs)
		free(node->childs);
	free(node);
}

void owner_index_load(struct pet_by_owner_handle* handle) {
	if (handle->header->count[owner_header_count] == 0) {
		handle->owner_root = NULL;
		return;
	}
	int32_t levels = log(handle->header->count[owner_header_count]) / log(owner_CHILDREN) + 1;
	uint64_t previous_level_count = 0, count = 0;
	struct owner_node** previous_level = NULL;
	struct owner_node** current_level = NULL;

	for (int32_t level = 1; level <= levels; level++) {
		uint64_t current_level_count = (handle->header->count[owner_header_count] + pow(owner_CHILDREN, level) - 1) / pow(owner_CHILDREN, level);
		current_level = calloc(current_level_count, sizeof(struct owner_node*));

		for (uint64_t i = 0; i < current_level_count; i++) {
			fseek(handle->file, handle->header->index_offset[owner_header_count] + (count++) * sizeof(struct owner_tree_item), SEEK_SET);
			struct owner_tree_item* current_tree_item = malloc(sizeof(struct owner_tree_item));
			uint64_t size = fread(current_tree_item, sizeof(struct owner_tree_item), 1, handle->file);  if (size == 0) return;
			current_level[i] = malloc(sizeof(struct owner_node));
			memcpy(&(current_level[i]->data), current_tree_item, sizeof(struct owner_tree_item));
			free(current_tree_item);
		}

		for (uint64_t i = 0; i < current_level_count; i++) {
			if (!previous_level) {
				current_level[i]->childs = NULL;
				current_level[i]->n = 0;
				continue;
			}

			current_level[i]->childs = calloc(owner_CHILDREN, sizeof(struct owner_node*));
			uint64_t j;
			for (j = 0; j < owner_CHILDREN; j++) {
				uint64_t k = i * owner_CHILDREN + j;
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
	handle->owner_root = current_level[0];
	free(current_level);
}

void add_owner_pet_1(int32_t oid, const char* oname) {
	struct owner* inserted = owner_new();
	inserted->oid = oid;
	memcpy(inserted->oname, oname, 6);
	inserted->pet_id = ++Pet_sequence;
	owner_buffer_add(inserted);
}

struct pet {
	int32_t pid;
	char pname[6];
};

struct pet_tree_item* pet_tree_item_new() {
	struct pet_tree_item* new = malloc(sizeof(struct pet_tree_item));
	memset(new, 0, sizeof(*new));
	return new;
}

void pet_tree_item_free(struct pet_tree_item* deleted) {
	free(deleted);
	return;
}

#define pet_CHILDREN (PAGE_SIZE / sizeof(struct pet_tree_item))

union pet_page {
	struct pet items[pet_CHILDREN];
	uint8_t as_bytes[PAGE_SIZE];
};

int pet_compare(struct pet* s1, struct pet* s2) {
	if ((s1->pid > s2->pid))
		return 1;
	else if ((s1->pid < s2->pid))
		return -1;

	if (strncmp(s1->pname, s2->pname, 6) > 0)
		return 1;
	else if (strncmp(s1->pname, s2->pname, 6) < 0)
		return -1;

	return 0;
}

struct pet* pet_new() {
	struct pet* new = malloc(sizeof(struct pet));
	memset(new, 0, sizeof(*new));
	return new;
}

void pet_free(struct pet* deleted) {
	free(deleted);
	return;
}

struct pet** pet_buffer = NULL;
struct MILLDB_buffer_info pet_buffer_info;

void pet_buffer_init() {
	pet_buffer_info.size = MILLDB_BUFFER_INIT_SIZE;
	pet_buffer_info.count = 0;
	pet_buffer = calloc(pet_buffer_info.size, sizeof(struct pet));
}

void pet_buffer_add(struct pet* inserted) {
	if (pet_buffer_info.count >= pet_buffer_info.size) {
		pet_buffer_info.size *= 2;
		pet_buffer = realloc(pet_buffer, pet_buffer_info.size * sizeof(struct pet*));
	}
	pet_buffer[pet_buffer_info.count++] = inserted;
}

void pet_buffer_free() {
	if (pet_buffer == NULL)
		return;

	for (uint64_t i = 0; i < pet_buffer_info.count; i++) {
		pet_free(pet_buffer[i]);
	}
	free(pet_buffer);
}

int pet_sort_compare(const void* a, const void* b) {
	return pet_compare((struct pet*)a, (struct pet*)b);
}

uint64_t pet_write(FILE* file) {
	qsort(pet_buffer, pet_buffer_info.count, sizeof(struct pet*), pet_sort_compare);
	for (uint64_t i = 0; i < pet_buffer_info.count; i++) {
		fwrite(pet_buffer[i], sizeof(struct pet), 1, file);
	}

	uint64_t page_size = pet_CHILDREN, items = 0;
	while (page_size < pet_buffer_info.count) {
		for (uint64_t i = 0; i < pet_buffer_info.count; i += page_size) {
			struct pet_tree_item *item = pet_tree_item_new();
			item->key = pet_buffer[i]->pid;
			item->offset = i * sizeof(struct pet);
			fwrite(item, sizeof(struct pet_tree_item), 1, file);
			items++;
			pet_tree_item_free(item);
		}
		page_size *= pet_CHILDREN;
	}

	struct pet_tree_item *item = pet_tree_item_new();
	item->key = pet_buffer[0]->pid;
	item->offset = 0;
	fwrite(item, sizeof(struct pet_tree_item), 1, file);
	pet_tree_item_free(item);
	return items + 1;
}

void pet_index_clean(struct pet_node* node) {
	if (node == NULL)
		return;

	for (uint64_t i = 0; i < node->n; i++)
		pet_index_clean(node->childs[i]);

	if (node->childs)
		free(node->childs);
	free(node);
}

void pet_index_load(struct pet_by_owner_handle* handle) {
	if (handle->header->count[pet_header_count] == 0) {
		handle->pet_root = NULL;
		return;
	}
	int32_t levels = log(handle->header->count[pet_header_count]) / log(pet_CHILDREN) + 1;
	uint64_t previous_level_count = 0, count = 0;
	struct pet_node** previous_level = NULL;
	struct pet_node** current_level = NULL;

	for (int32_t level = 1; level <= levels; level++) {
		uint64_t current_level_count = (handle->header->count[pet_header_count] + pow(pet_CHILDREN, level) - 1) / pow(pet_CHILDREN, level);
		current_level = calloc(current_level_count, sizeof(struct pet_node*));

		for (uint64_t i = 0; i < current_level_count; i++) {
			fseek(handle->file, handle->header->index_offset[pet_header_count] + (count++) * sizeof(struct pet_tree_item), SEEK_SET);
			struct pet_tree_item* current_tree_item = malloc(sizeof(struct pet_tree_item));
			uint64_t size = fread(current_tree_item, sizeof(struct pet_tree_item), 1, handle->file);  if (size == 0) return;
			current_level[i] = malloc(sizeof(struct pet_node));
			memcpy(&(current_level[i]->data), current_tree_item, sizeof(struct pet_tree_item));
			free(current_tree_item);
		}

		for (uint64_t i = 0; i < current_level_count; i++) {
			if (!previous_level) {
				current_level[i]->childs = NULL;
				current_level[i]->n = 0;
				continue;
			}

			current_level[i]->childs = calloc(pet_CHILDREN, sizeof(struct pet_node*));
			uint64_t j;
			for (j = 0; j < pet_CHILDREN; j++) {
				uint64_t k = i * pet_CHILDREN + j;
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
	handle->pet_root = current_level[0];
	free(current_level);
}

void add_owner_pet_2(const char* pname) {
	struct pet* inserted = pet_new();
	inserted->pid = Pet_sequence;
	memcpy(inserted->pname, pname, 6);
	pet_buffer_add(inserted);
}

void add_owner_pet(int32_t oid, const char* oname, const char* pname) {
	add_owner_pet_1(oid, oname);
	add_owner_pet_2(pname);
}

void get_pet_by_pid_add(struct get_pet_by_pid_out* iter, struct get_pet_by_pid_out_data* selected) {
	struct get_pet_by_pid_out_service* service = &(iter->service);
	if (service->set == NULL) {
		service->size = MILLDB_BUFFER_INIT_SIZE;
		service->set = calloc(service->size, sizeof(struct get_pet_by_pid_out));
	}
	if (service->length >= service->size) {
		service->size = service->size * 2;
		service->set = realloc(service->set, service->size * sizeof(struct get_pet_by_pid_out));
	}
	memcpy(&(service->set[service->length++]), selected, sizeof(struct get_pet_by_pid_out_data));
}

void get_pet_by_pid_1(struct get_pet_by_pid_out* iter, int32_t id) {
//table pet	cond: pid = @id
	struct pet_by_owner_handle* handle = iter->service.handle;
	struct get_pet_by_pid_out_data* inserted = malloc(sizeof(struct get_pet_by_pid_out_data));
//TABLE pet
	uint64_t offset = 0;

	struct pet_node* node = handle->pet_root;
	uint64_t i = 0;
	while (1) {
		if (node->data.key == id || node->childs == NULL) {
			offset = node->data.offset;
			break;
		}
		if (node->childs[i]->data.key > id && i > 0) {
			node = node->childs[i-1];
			i = 0;
			continue;
		}
		if (i == node->n-1) {
			node = node->childs[i];
			i = 0;
			continue;
		}
		i++;
	}

	offset += handle->header->data_offset[pet_header_count];
	
	while (1) {
		fseek(handle->file, offset, SEEK_SET);
		union pet_page page;
		uint64_t size = fread(&page, sizeof(struct pet), pet_CHILDREN, handle->file);  if (size == 0) return;

		for (uint64_t i = 0; i < pet_CHILDREN; i++) {
			const char* p_pname= page.items[i].pname;
			int32_t c_pid= page.items[i].pid;
			const char* c_pname= page.items[i].pname;
			if (c_pid > id || offset + i * sizeof(struct pet) >= handle->header->index_offset[pet_header_count]) {
				free(inserted);
				return;
			}
			if (c_pid == id) {
				memcpy(inserted->pname, c_pname, 6); inserted->pname[6] = '\0';
				get_pet_by_pid_add(iter, inserted);
			}
		}
		offset += pet_CHILDREN * sizeof(struct pet);
	}

}

void get_pet_by_pid_init(struct get_pet_by_pid_out* iter, struct pet_by_owner_handle* handle, int32_t id) {
	memset(iter, 0, sizeof(*iter));
	iter->service.handle = handle;
	iter->service.set = NULL;
	iter->service.size = 0;
	iter->service.count = 0;
	iter->service.length = 0;

	get_pet_by_pid_1(iter, id);
}

int get_pet_by_pid_next(struct get_pet_by_pid_out* iter) {
	if (iter == NULL)
		return 0;

	struct get_pet_by_pid_out_service* service = &(iter->service);

	if (service->set != NULL && service->count < service->length) {
		memcpy(&iter->data, &(service->set[service->count]), sizeof(struct get_pet_by_pid_out_data));
		service->count++;
		return 1;
	} else {
		free(service->set);
	}

	return 0;
}

struct pet_by_owner_handle* pet_by_owner_write_handle = NULL;

void pet_by_owner_open_write(const char* filename) {
	FILE* file;
	if (!(file = fopen(filename, "wb")))
		return;

	pet_by_owner_write_handle = malloc(sizeof(struct pet_by_owner_handle));
	pet_by_owner_write_handle->file = file;
	pet_by_owner_write_handle->mode = MILLDB_FILE_MODE_WRITE;

	owner_buffer_init();
	pet_buffer_init();
}

int pet_by_owner_save(struct pet_by_owner_handle* handle) {
	if (handle && handle->mode == MILLDB_FILE_MODE_WRITE) {
		struct MILLDB_header* header = malloc(sizeof(struct MILLDB_header));
		fseek(handle->file, MILLDB_HEADER_SIZE, SEEK_SET);

		uint64_t owner_index_count = 0;
		if (owner_buffer_info.count > 0)
			owner_index_count = owner_write(handle->file);

		uint64_t pet_index_count = 0;
		if (pet_buffer_info.count > 0)
			pet_index_count = pet_write(handle->file);

		uint64_t offset = MILLDB_HEADER_SIZE;

		header->count[owner_header_count] = owner_buffer_info.count;
		header->data_offset[owner_header_count] = offset;
		offset += owner_buffer_info.count * sizeof(struct owner);
		header->index_offset[owner_header_count] = offset;
		offset += owner_index_count * sizeof(struct owner_tree_item);

		header->count[pet_header_count] = pet_buffer_info.count;
		header->data_offset[pet_header_count] = offset;
		offset += pet_buffer_info.count * sizeof(struct pet);
		header->index_offset[pet_header_count] = offset;
		offset += pet_index_count * sizeof(struct pet_tree_item);


		fseek(handle->file, 0, SEEK_SET);
		fwrite(header, MILLDB_HEADER_SIZE, 1, handle->file);
		free(header);
	}
	return 0;
}

void pet_by_owner_close_write(void) {
	if (pet_by_owner_write_handle == NULL)
		return;

	pet_by_owner_save(pet_by_owner_write_handle);

	owner_buffer_free();
	pet_buffer_free();

	fclose(pet_by_owner_write_handle->file);
	free(pet_by_owner_write_handle);
}

struct pet_by_owner_handle* pet_by_owner_open_read(const char* filename) {
	FILE* file;
	if (!(file = fopen(filename, "rb")))
		return NULL;

	struct pet_by_owner_handle* handle = malloc(sizeof(struct pet_by_owner_handle));
	handle->file = file;
	handle->mode = MILLDB_FILE_MODE_READ;

	fseek(handle->file, 0, SEEK_SET);
	struct MILLDB_header* header = malloc(MILLDB_HEADER_SIZE);
	uint64_t size = fread(header, MILLDB_HEADER_SIZE, 1, handle->file);  if (size == 0) return NULL;
	handle->header = header;

	owner_index_load(handle);
	pet_index_load(handle);

	return handle;
}

void pet_by_owner_close_read(struct pet_by_owner_handle* handle) {
	if (handle == NULL)
		return;

	fclose(handle->file);

	if (handle->owner_root)
		owner_index_clean(handle->owner_root);

	if (handle->pet_root)
		pet_index_clean(handle->pet_root);


	free(handle->header);
	free(handle);
}

