#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "sample.h"

#define NODE_SIZE 10

struct Person_struct {
	int32_t id;
	char name[101];
	int32_t age;
	int32_t position;
};

struct Person_struct* Person_struct_new() {
	struct Person_struct* new = malloc(sizeof(struct Person_struct));
	new->id = 0;
	memset(new->name, 0, 101);
	new->age = 0;
	new->position = 0;
	return new;
}

int Person_struct_compare(struct Person_struct* s1, struct Person_struct* s2) {
	if ((s1->id > s2->id)) {
		return 1;
	} else if ((s1->id < s2->id)) {
		return -1;
	}

	return 0;

}

struct Person_Node {
	struct Person_struct** keys;
	struct Person_Node** C;
	int n;
	bool is_leaf;
};

struct Person_Node* Person_root = NULL;

struct Person_Node* Person_Node_new(bool is_leaf) {
	struct Person_Node* Person_Node_temp = malloc(sizeof(struct Person_Node));
	Person_Node_temp->keys = malloc((2*NODE_SIZE-1) * sizeof(struct Person_struct));
	Person_Node_temp->C = malloc((2*NODE_SIZE) * sizeof(struct Person_Node));
	Person_Node_temp->n = 0;
	Person_Node_temp->is_leaf = is_leaf;
	return Person_Node_temp;
}

void Person_clean(struct Person_Node* node) {
	if (node == NULL)
		return;

	int i;
	for (i = 0; i < node->n; i++) {
		if (!node->is_leaf)
			Person_clean(node->C[i]);
		free(node->keys[i]);
	}

	if (!node->is_leaf)
		Person_clean(node->C[i]);

	free(node->keys);
	free(node->C);
	free(node);
}

struct Person_struct* Person_search(struct Person_Node* this, struct Person_struct* searched) {
	int i = 0;
	while (i < this->n && Person_struct_compare(searched, this->keys[i]) == 1) {
		i++;
	}

	if (i != this->n && Person_struct_compare(this->keys[i], searched) == 0) {
		return this->keys[i];
	}
	if (this->is_leaf)
		return NULL;
	return Person_search(this->C[i], searched);
}

void Person_split_child(struct Person_Node* this, int i, struct Person_Node* y) {
	struct Person_Node* z = Person_Node_new(y->is_leaf);
	z->n = NODE_SIZE - 1;

	for (int j = 0; j < NODE_SIZE-1; j++)
		z->keys[j] = y->keys[j+NODE_SIZE];

	if (! y->is_leaf) {
		for (int j = 0; j < NODE_SIZE; j++)
			z->C[j] = y->C[j+NODE_SIZE];
	}

	y->n = NODE_SIZE - 1;

	for (int j = this->n; j >= i+1; j--)
		this->C[j+1] = this->C[j];

	this->C[i+1] = z;

	for (int j = this->n-1; j >= i; j--)
		this->keys[j+1] = this->keys[j];

	this->keys[i] = y->keys[NODE_SIZE-1];
	this->n = this->n + 1;
}

void Person_insert_routine(struct Person_Node* this, struct Person_struct* k) {
	int i;
	for (i = 0; i < this->n; i++) {
		if (Person_struct_compare(this->keys[i], k) == 0) {
			printf("error while insert (duplicate key)\n");
			free(k);
			return;
		}
	}

	i = this->n-1;
	if (this->is_leaf == true) {
		while (i >= 0 && Person_struct_compare(this->keys[i], k) == 1) {
			this->keys[i+1] = this->keys[i];
			i--;
		}

		this->keys[i+1] = k;
		this->n = this->n+1;
	} else {
		while (i >= 0 && Person_struct_compare(this->keys[i], k) == 1)
			i--;

		if (this->C[i+1]->n == 2*NODE_SIZE-1) {
			Person_split_child(this, i+1, this->C[i+1]);

			if (this->keys[i+1] < k)
				i++;
		}
		Person_insert_routine(this->C[i+1], k);
	}
}

void Person_insert(struct Person_struct* inserted) {
	if (Person_root == NULL) {
		Person_root = Person_Node_new(true);
		Person_root->keys[0] = inserted;
		Person_root->n = 1;
	} else {
		if (Person_root->n == 2*NODE_SIZE-1) {
			struct Person_Node* s = Person_Node_new(false);
			s->C[0] = Person_root;

			Person_split_child(s, 0, Person_root);

			int i = 0;
			if (Person_struct_compare(s->keys[0], inserted) == -1)
				i++;
			Person_insert_routine(s->C[i], inserted);
			Person_root = s;
		} else {
			Person_insert_routine(Person_root, inserted);
		}
	}
}

void Person_serialize(FILE* file, struct Person_Node* node) {
	if (node == NULL)
		return;
	int i;
	for (i = 0; i < node->n; i++) {
		if (!node->is_leaf)
			Person_serialize(file, node->C[i]);
		fprintf(file, "(%d,\"%s\",%d,%d)\n", 
			node->keys[i]->id, 
			node->keys[i]->name, 
			node->keys[i]->age, 
			node->keys[i]->position
		);
	}
	if (!node->is_leaf)
		Person_serialize(file, node->C[i]);
}

void add_person_1(int32_t id, char name[101], int32_t age, int32_t position) {
	struct Person_struct* arg = Person_struct_new();
	arg->id = id;
	strncpy(arg->name, name, 101);
	arg->age = age;
	arg->position = position;
	Person_insert(arg);
}

void add_person(int32_t id, char name[101], int32_t age, int32_t position) {
	add_person_1(id, name, age, position);
}

struct Positions_struct {
	int32_t id;
	char name[21];
};

struct Positions_struct* Positions_struct_new() {
	struct Positions_struct* new = malloc(sizeof(struct Positions_struct));
	new->id = 0;
	memset(new->name, 0, 21);
	return new;
}

int Positions_struct_compare(struct Positions_struct* s1, struct Positions_struct* s2) {
	if ((s1->id > s2->id)) {
		return 1;
	} else if ((s1->id < s2->id)) {
		return -1;
	}

	return 0;

}

struct Positions_Node {
	struct Positions_struct** keys;
	struct Positions_Node** C;
	int n;
	bool is_leaf;
};

struct Positions_Node* Positions_root = NULL;

struct Positions_Node* Positions_Node_new(bool is_leaf) {
	struct Positions_Node* Positions_Node_temp = malloc(sizeof(struct Positions_Node));
	Positions_Node_temp->keys = malloc((2*NODE_SIZE-1) * sizeof(struct Positions_struct));
	Positions_Node_temp->C = malloc((2*NODE_SIZE) * sizeof(struct Positions_Node));
	Positions_Node_temp->n = 0;
	Positions_Node_temp->is_leaf = is_leaf;
	return Positions_Node_temp;
}

void Positions_clean(struct Positions_Node* node) {
	if (node == NULL)
		return;

	int i;
	for (i = 0; i < node->n; i++) {
		if (!node->is_leaf)
			Positions_clean(node->C[i]);
		free(node->keys[i]);
	}

	if (!node->is_leaf)
		Positions_clean(node->C[i]);

	free(node->keys);
	free(node->C);
	free(node);
}

struct Positions_struct* Positions_search(struct Positions_Node* this, struct Positions_struct* searched) {
	int i = 0;
	while (i < this->n && Positions_struct_compare(searched, this->keys[i]) == 1) {
		i++;
	}

	if (i != this->n && Positions_struct_compare(this->keys[i], searched) == 0) {
		return this->keys[i];
	}
	if (this->is_leaf)
		return NULL;
	return Positions_search(this->C[i], searched);
}

void Positions_split_child(struct Positions_Node* this, int i, struct Positions_Node* y) {
	struct Positions_Node* z = Positions_Node_new(y->is_leaf);
	z->n = NODE_SIZE - 1;

	for (int j = 0; j < NODE_SIZE-1; j++)
		z->keys[j] = y->keys[j+NODE_SIZE];

	if (! y->is_leaf) {
		for (int j = 0; j < NODE_SIZE; j++)
			z->C[j] = y->C[j+NODE_SIZE];
	}

	y->n = NODE_SIZE - 1;

	for (int j = this->n; j >= i+1; j--)
		this->C[j+1] = this->C[j];

	this->C[i+1] = z;

	for (int j = this->n-1; j >= i; j--)
		this->keys[j+1] = this->keys[j];

	this->keys[i] = y->keys[NODE_SIZE-1];
	this->n = this->n + 1;
}

void Positions_insert_routine(struct Positions_Node* this, struct Positions_struct* k) {
	int i;
	for (i = 0; i < this->n; i++) {
		if (Positions_struct_compare(this->keys[i], k) == 0) {
			printf("error while insert (duplicate key)\n");
			free(k);
			return;
		}
	}

	i = this->n-1;
	if (this->is_leaf == true) {
		while (i >= 0 && Positions_struct_compare(this->keys[i], k) == 1) {
			this->keys[i+1] = this->keys[i];
			i--;
		}

		this->keys[i+1] = k;
		this->n = this->n+1;
	} else {
		while (i >= 0 && Positions_struct_compare(this->keys[i], k) == 1)
			i--;

		if (this->C[i+1]->n == 2*NODE_SIZE-1) {
			Positions_split_child(this, i+1, this->C[i+1]);

			if (this->keys[i+1] < k)
				i++;
		}
		Positions_insert_routine(this->C[i+1], k);
	}
}

void Positions_insert(struct Positions_struct* inserted) {
	if (Positions_root == NULL) {
		Positions_root = Positions_Node_new(true);
		Positions_root->keys[0] = inserted;
		Positions_root->n = 1;
	} else {
		if (Positions_root->n == 2*NODE_SIZE-1) {
			struct Positions_Node* s = Positions_Node_new(false);
			s->C[0] = Positions_root;

			Positions_split_child(s, 0, Positions_root);

			int i = 0;
			if (Positions_struct_compare(s->keys[0], inserted) == -1)
				i++;
			Positions_insert_routine(s->C[i], inserted);
			Positions_root = s;
		} else {
			Positions_insert_routine(Positions_root, inserted);
		}
	}
}

void Positions_serialize(FILE* file, struct Positions_Node* node) {
	if (node == NULL)
		return;
	int i;
	for (i = 0; i < node->n; i++) {
		if (!node->is_leaf)
			Positions_serialize(file, node->C[i]);
		fprintf(file, "(%d,\"%s\")\n", 
			node->keys[i]->id, 
			node->keys[i]->name
		);
	}
	if (!node->is_leaf)
		Positions_serialize(file, node->C[i]);
}

void add_position_1(int32_t id, char name[21]) {
	struct Positions_struct* arg = Positions_struct_new();
	arg->id = id;
	strncpy(arg->name, name, 21);
	Positions_insert(arg);
}

void add_position(int32_t id, char name[21]) {
	add_position_1(id, name);
}

struct get_person_by_id_out_struct** get_person_by_id_data = NULL;
int get_person_by_id_size = 0;
int get_person_by_id_iter_count = 0;

void get_person_by_id_1(struct get_person_by_id_out_struct** iter, int32_t id) {
	struct Person_struct* searched = malloc(sizeof(struct Person_struct));
	searched->id = id;
	struct Person_struct* res = Person_search(Person_root, searched);
	free(searched);

	if (res != NULL) {
		struct get_person_by_id_out_struct* returned = malloc(sizeof(struct get_person_by_id_out_struct));
		strncpy(returned->name, res->name, 101);
		returned->age = res->age;
		returned->position = res->position;
		get_person_by_id_size += 1;
		get_person_by_id_data = realloc(get_person_by_id_data, get_person_by_id_size * sizeof(struct get_person_by_id_out_struct));
		get_person_by_id_data[get_person_by_id_size - 1] = returned;
	}
}

int get_person_by_id_init(struct get_person_by_id_out_struct** iter, int32_t id) {
	get_person_by_id_data = NULL;
	get_person_by_id_size = 0;
	get_person_by_id_iter_count = 0;

	get_person_by_id_1(iter, id);

	if (get_person_by_id_data != NULL)
		*iter = get_person_by_id_data[0];

	return 0;
}

int get_person_by_id_next(struct get_person_by_id_out_struct** iter) {
	if (get_person_by_id_data != NULL && get_person_by_id_iter_count < get_person_by_id_size) {
		iter++;
		get_person_by_id_iter_count++;
		return 1;
	}

	for (int i = 0; i < get_person_by_id_size; i++)
		free(get_person_by_id_data[i]);
	free(get_person_by_id_data);

	return 0;
}

char* sample_filename;

#define MILLDB_OUTPUT_MAX_LINE_LENGTH 512
#define MILLDB_OUTPUT_TABLE_DIRECTIVE ("#table ")

int sample_open(char* filename) {
	sample_filename = filename;
	FILE* file;

	if ((file = fopen(sample_filename, "r")) == NULL)
		return 0;

	int table_index = -1;
	char* source = malloc(MILLDB_OUTPUT_MAX_LINE_LENGTH);

	while (!feof(file)) {
		if (fgets(source, MILLDB_OUTPUT_MAX_LINE_LENGTH, file) == NULL)
			break;

		char* line = source;
		line[strlen(line) - 1] = '\0';

		if (line[0] == '(') {
			line++;
			line[strlen(line) - 1] = '\0';
				if (table_index == 0) {
				struct Person_struct* arg = Person_struct_new();
				char* token = strtok(line, ",");

				int i = 0;
				while (token != NULL) {
					switch (i) {
						case 0:
							sscanf(token, "%d", &(arg->id));
							break;

						case 1:
							memcpy(arg->name, token + sizeof(char), strlen(token)-2);
							break;

						case 2:
							sscanf(token, "%d", &(arg->age));
							break;

						case 3:
							sscanf(token, "%d", &(arg->position));
							break;

					}

					if (i >= 4)
						break;

					token = strtok(NULL, ",");
					i++;
				}
				Person_insert(arg);

			}

				if (table_index == 1) {
				struct Positions_struct* arg = Positions_struct_new();
				char* token = strtok(line, ",");

				int i = 0;
				while (token != NULL) {
					switch (i) {
						case 0:
							sscanf(token, "%d", &(arg->id));
							break;

						case 1:
							memcpy(arg->name, token + sizeof(char), strlen(token)-2);
							break;

					}

					if (i >= 2)
						break;

					token = strtok(NULL, ",");
					i++;
				}
				Positions_insert(arg);

			}

			continue;
		}

		if (line[0] == '#') {
			line += sizeof(char) * strlen(MILLDB_OUTPUT_TABLE_DIRECTIVE);

			if (strcmp(line, "Person") == 0)
				table_index = 0;

			if (strcmp(line, "Positions") == 0)
				table_index = 1;

			continue;
		}

		break;
	}

	free(source);

	fclose(file);
	return 0;
}

int sample_close() {
	FILE* file = fopen(sample_filename, "w");
	if (Person_root != NULL) {
		fprintf(file, MILLDB_OUTPUT_TABLE_DIRECTIVE);
		fprintf(file, "Person\n");
		Person_serialize(file, Person_root);
		Person_clean(Person_root);
		Person_root = NULL;
	}

	if (Positions_root != NULL) {
		fprintf(file, MILLDB_OUTPUT_TABLE_DIRECTIVE);
		fprintf(file, "Positions\n");
		Positions_serialize(file, Positions_root);
		Positions_clean(Positions_root);
		Positions_root = NULL;
	}

	fclose(file);
	return 0;

}

