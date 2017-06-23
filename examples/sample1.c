#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#ifndef PAGE_SIZE
	#define PAGE_SIZE 4096 
#endif

#define MILLDB_BUFFER_INIT_SIZE 32

/*
 *	MillDB buffer
 */
struct MILLDB_buffer_info {
	uint64_t size;
	uint64_t count;
};

/*
 *	Returns count of last tree_item childs.
 *	n -- total number of rows
 *  clildren -- default number of childs 
 */
inline uint32_t MILLDB_last_child_count(uint64_t n, uint32_t children) {
	return (n - children) % children + children;
}

/*
 *	Returns number of tree levels
 *	n -- total number of rows
 *  clildren -- default number of childs 
 */
uint32_t MILLDB_levels(uint64_t n, uint32_t children) {
	for (uint32_t levels = 1; levels < 20; levels++) {
		uint64_t x = 2 * pow(children, levels) + (levels - 2);
		if (x >= n)
			return levels;
	}
	return -1;
}

/*
 *	MillDB header
 */
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

struct Person_tree_item** Person_tree_item_buffer = NULL;
struct MILLDB_buffer_info Person_tree_item_buffer_info;

void Person_tree_item_buffer_init() {
	Person_tree_item_buffer_info.size = MILLDB_BUFFER_INIT_SIZE;
	Person_tree_item_buffer_info.count = 0;
	Person_tree_item_buffer = calloc(Person_tree_item_buffer_info.size, sizeof(struct Person_tree_item));
}

void Person_tree_item_add(struct Person_tree_item* inserted) {
	if (Person_tree_item_buffer_info.count >= Person_tree_item_buffer_info.size) {
		Person_tree_item_buffer_info.size *= 2;
		Person_tree_item_buffer = realloc(Person_tree_item_buffer, Person_tree_item_buffer_info.size * sizeof(struct Person_tree_item*));
	}
	Person_tree_item_buffer[Person_tree_item_buffer_info.count++] = inserted;
}

void Person_tree_item_free(struct Person_tree_item* deleted) {
	free(deleted);
	return;
}

void Person_tree_item_buffer_free() {
	for (uint64_t i = 0; i < Person_tree_item_buffer_info.count; i++) {
		Person_tree_item_free(Person_tree_item_buffer[i]);
	}
	free(Person_tree_item_buffer);
}

#define Person_CHILDREN (PAGE_SIZE / sizeof(struct Person_tree_item))

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

	if (strncmp(s1->name, s1->name, 4) == 1)
		return 1;
	else if (strncmp(s1->name, s1->name, 4) == -1)
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

struct Person_Node {
	struct Person** keys;
	struct Person_Node** C;
	uint64_t n;
	uint8_t is_leaf;
};

struct Person_Node* Person_root = NULL;

struct Person_Node* Person_Node_new(uint8_t is_leaf) {
	struct Person_Node* Person_Node_temp = malloc(sizeof(struct Person_Node));
	Person_Node_temp->keys = malloc((2*Person_CHILDREN-1) * sizeof(struct Person));
	Person_Node_temp->C = malloc((2*Person_CHILDREN) * sizeof(struct Person_Node));
	Person_Node_temp->n = 0;
	Person_Node_temp->is_leaf = is_leaf;
	return Person_Node_temp;
}

void Person_split(struct Person_Node* this, int i, struct Person_Node* y) {
	struct Person_Node* z = Person_Node_new(y->is_leaf);
	z->n = Person_CHILDREN - 1;

	for (int j = 0; j < Person_CHILDREN-1; j++)
		z->keys[j] = y->keys[j+Person_CHILDREN];

	if (!y->is_leaf) {
		for (int j = 0; j < Person_CHILDREN; j++)
			z->C[j] = y->C[j+Person_CHILDREN];
	}

	y->n = Person_CHILDREN - 1;

	for (int j = this->n; j >= i+1; j--)
		this->C[j+1] = this->C[j];

	this->C[i+1] = z;

	for (int j = this->n-1; j >= i; j--)
		this->keys[j+1] = this->keys[j];

	this->keys[i] = y->keys[Person_CHILDREN-1];
	this->n = this->n + 1;
}

void Person_insert_routine(struct Person_Node* this, struct Person* k) {
	int i;

	i = this->n-1;
	if (this->is_leaf == 1) {
		while (i >= 0 && Person_compare(this->keys[i], k) == 1) {
			this->keys[i+1] = this->keys[i];
			i--;
		}

		this->keys[i+1] = k;
		this->n = this->n+1;
	} else {
		while (i >= 0 && Person_compare(this->keys[i], k) == 1)
			i--;

		if (this->C[i+1]->n == 2*Person_CHILDREN-1) {
			Person_split(this, i+1, this->C[i+1]);

			if (this->keys[i+1] < k)
				i++;
		}
		Person_insert_routine(this->C[i+1], k);
	}
}

void Person_insert(struct Person* inserted) {
	if (Person_root == NULL) {
		Person_root = Person_Node_new(1);
		Person_root->keys[0] = inserted;
		Person_root->n = 1;
	} else {
		if (Person_root->n == 2*Person_CHILDREN-1) {
			struct Person_Node* s = Person_Node_new(0);
			s->C[0] = Person_root;

			Person_split(s, 0, Person_root);

			int i = 0;
			if (Person_compare(s->keys[0], inserted) == -1)
				i++;
			Person_insert_routine(s->C[i], inserted);
			Person_root = s;
		} else {
			Person_insert_routine(Person_root, inserted);
		}
	}
}




/* 
 * PERSON buffer
 */

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


int Person_traverse_and_clean(FILE* file, struct Person_Node* node, int offset_) {
	uint64_t total_children_count = 0;
	int current_offset = offset_, effective_offset = offset_;

	for (int i = 0; i <= node->n; i++) {
		if (!node->is_leaf) {
			int child_count = Person_traverse_and_clean(file, node->C[i], current_offset);
			total_children_count += child_count;
			current_offset += child_count * sizeof(struct Person);

			if (effective_offset == offset_)
				effective_offset = current_offset;
		}

		if (i == node->n) 
			break;

		fwrite(node->keys[i], sizeof(struct Person), 1, file);

		total_children_count++;
		current_offset += sizeof(struct Person);
	}

	// printf("total_children_count=%ld, is_leaf=%d, offset=%d, key=%d\n", 
	// 	total_children_count, total_children_count == node->n, effective_offset, node->keys[0]->id);

	struct Person_tree_item* tree_item = Person_tree_item_new();
	tree_item->key = node->keys[0]->id;
	tree_item->offset = effective_offset;
	Person_tree_item_add(tree_item);

	free(node->keys);
	free(node->C);
	free(node);

	return total_children_count;
}



FILE* sample1_file = NULL;
char sample1_file_opened = 0;
char* sample1_file_opened_mode = NULL;

#define MILLDB_IF_WRITEMODE(mode)	(strncmp(mode, "w", 1) == 0)
#define MILLDB_IF_READMODE(mode)	(strncmp(mode, "r", 1) == 0)

int sample1_open(char* filename, char* mode) {
	if (!sample1_file_opened) {
		if (!(sample1_file = fopen(filename, mode)))
				return 1;

		if (MILLDB_IF_READMODE(mode)) {

		}

		if (MILLDB_IF_WRITEMODE(mode)) {
			Person_buffer_init();
		}

		sample1_file_opened = 1;
		sample1_file_opened_mode = mode;
	} else
		printf("tt_close(): sample1_file_opened == 1\n");

	return 0;
}

int sample1_save() {
	if (sample1_file_opened && MILLDB_IF_WRITEMODE(sample1_file_opened_mode) && Person_buffer_info.count > 0) {

		// Prepare and write header
		struct MILLDB_header* header = malloc(sizeof(struct MILLDB_header));
		header->count = Person_buffer_info.count;
		header->data_offset = MILLDB_HEADER_SIZE;
		header->index_offset = header->data_offset + Person_buffer_info.count * sizeof(struct Person);
		fwrite(header, MILLDB_HEADER_SIZE, 1, sample1_file);
		free(header);

		// Construct heavy tree
		for (uint64_t i = 0; i < Person_buffer_info.count; i++)
			Person_insert(Person_buffer[i]);

		// Init buffer for items of light tree, that will be saved to disk
		Person_tree_item_buffer_init();

		// Collect items of light tree and clean heavy tree
		Person_traverse_and_clean(sample1_file, Person_root, 0);

		// Write light tree to disk
		for (uint64_t i = 0; i < Person_tree_item_buffer_info.count; i++) {
			fwrite(Person_tree_item_buffer[i], sizeof(struct Person_tree_item), 1, sample1_file);
		}

		// Clean light tree, deallocate
		Person_tree_item_buffer_free();
	} else
		printf("tt_save(): fail\n");

	return 0;
}

int sample1_close() {
	if (sample1_file_opened) {
		if (MILLDB_IF_READMODE(sample1_file_opened_mode)) {
		}

		if (MILLDB_IF_WRITEMODE(sample1_file_opened_mode)) {
			sample1_save();
			Person_buffer_free();
		}

		fclose(sample1_file);
		sample1_file = NULL;
		sample1_file_opened_mode = NULL;
		sample1_file_opened = 0;
	} else
		printf("tt_close(): sample1_file_opened == 0\n");

	return 0;
}

// struct Person* Person_read(FILE* file, int32_t offset) {
// 	fseek(file, offset, SEEK_SET);
// 	struct Person* new = Person_new();
// 	fread(new, sizeof(struct Person), 1, file);
// 	return new;
// }

int writeproc_1(int32_t param1, const char* name) {
	struct Person* inserted = Person_new();

	inserted->id = param1;
	memcpy(inserted->name, name, 4);

	// Just add it to storage in a row
	Person_buffer_add(inserted);

	return 0;
}

int writeproc(int32_t param1, const char* name) {
	return writeproc_1(param1, name);
}

struct get_person_by_id_out_struct* get_person_by_id_data = NULL;
int get_person_by_id_size = 0;
int get_person_by_id_iter_count = 0;

struct get_person_by_id_out_struct {
	char name[5];
};

void get_person_by_id_1(int32_t id) {
	fseek(sample1_file, 0, SEEK_SET);
	struct MILLDB_header* header = malloc(MILLDB_HEADER_SIZE);
	fread(header, MILLDB_HEADER_SIZE, 1, sample1_file);

	for (uint64_t i = 0; i < header->count; i++) {
		fseek(sample1_file, MILLDB_HEADER_SIZE + i * sizeof(struct Person), SEEK_SET);
		struct Person* current = malloc(sizeof(struct Person));
		fread(current, sizeof(struct Person), 1, sample1_file);

		if (current->id == id) {
			get_person_by_id_data = malloc(sizeof(struct get_person_by_id_out_struct));	
			memcpy(get_person_by_id_data->name, current->name, 4);
			get_person_by_id_data->name[4] = '\0';
			free(current);
			get_person_by_id_size = 1;
			break;
		}

		free(current);
	}

	free(header);
	return;
}

int get_person_by_id_init(struct get_person_by_id_out_struct* iter, int32_t id) {
	memset(iter, 0, sizeof(*iter));

	get_person_by_id_data = NULL;
	get_person_by_id_size = 0;
	get_person_by_id_iter_count = 0;

	get_person_by_id_1(id);

	return 0;
}

int get_person_by_id_next(struct get_person_by_id_out_struct* iter) {
	if (get_person_by_id_data != NULL && get_person_by_id_iter_count < get_person_by_id_size) {
		memcpy(iter, get_person_by_id_data, sizeof(*iter));
		get_person_by_id_iter_count++;
		return 1;
	}

	if (get_person_by_id_data != NULL) {
		free(get_person_by_id_data);
	}

	return 0;
}

int main() {
	sample1_open("FILE", "w");

	for (int i = 0; i < 30; i++)
		writeproc(i, "aaaa");

	writeproc(555, "afaa");

	sample1_close();

	sample1_open("FILE", "r");

	struct get_person_by_id_out_struct iter;

	get_person_by_id_init(&iter, 555);
	while (get_person_by_id_next(&iter))
		printf("%s\n", iter.name);

	sample1_close();

	return 0;
}