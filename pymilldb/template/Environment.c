#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "{{ context.NAME }}.h"

#ifndef PAGE_SIZE
    #define PAGE_SIZE 4096
#endif

struct bloom_filter {
    char *cell;
    size_t cell_size;
    size_t *seeds;
    size_t seeds_size;
};

int _get_bool(const char *cell, size_t n) {
    return (int)cell[n/8] & (int)(1 << n%8);
}

void _set_bool(char *cell, size_t n) {
    cell[n/8] = cell[n/8] | ((char)1 << n%8);
}

size_t _hash_str(size_t seed1, size_t seed2, size_t mod, const char *str, size_t str_size) {
    unsigned int res = 1;
    for (int i = 0; i < (int)str_size; i++) {
        res = (seed1 * res + seed2 * str[i]) % mod;
    }
    return res;
}

size_t _generate_seed_str(char *str, size_t str_size) {
    return _hash_str(37, 1, 64, str, str_size); // random numbers
}

struct bloom_filter *new_bf(size_t set_size, double fail_share) {
    fail_share = fail_share < 0.01 ? 0.01 :
                 fail_share > 0.99 ? 0.99 : fail_share;
    size_t cell_size = (-1.0 * (double)set_size * log(fail_share)) / pow(log(2),2);
    cell_size = cell_size < 1 ? 1 : cell_size;
    size_t hashes_size = log(2) * (double)cell_size / set_size;
    hashes_size = hashes_size < 1 ? 1 : hashes_size;
    struct bloom_filter *bf = calloc(1, sizeof(struct bloom_filter));
    size_t buf_size = cell_size/8;
    if (cell_size%8 != 0) {
        buf_size++;
    }
    bf->cell = calloc(buf_size, sizeof(char));
    bf->cell_size = cell_size;
    bf->seeds = calloc(hashes_size, sizeof(size_t));
    bf->seeds_size = hashes_size;
    for (int i = 0; i < (int)hashes_size; i++) {
        bf->seeds[i] = i * 2.5 + cell_size / 10;
    }
    return bf;
}

void delete_bf(struct bloom_filter *bf) {
    free(bf->cell);
    free(bf->seeds);
    free(bf);
}

void add_bf(struct bloom_filter *bf, void *item, size_t item_size) {
    size_t seed_str = _generate_seed_str(item, item_size);
    for (int i = 0; i < bf->seeds_size; i++) {
        _set_bool(bf->cell, _hash_str(bf->seeds[i], seed_str, bf->cell_size, item, item_size));
    }
}

int check_bf(struct bloom_filter *bf, void *item, size_t item_size) {
    size_t seed_str = _generate_seed_str(item, item_size);
    for (int i = 0; i < bf->seeds_size; i++) {
        if (!_get_bool(bf->cell, _hash_str(bf->seeds[i], seed_str, bf->cell_size, item, item_size))) {
            return 0;
        }
    }
    return 1;
}

#define MILLDB_BUFFER_INIT_SIZE 32

struct MILLDB_buffer_info {
    uint64_t size;
    uint64_t count;
};
{% for table in context.TABLES.values() %}
#define {{ table.name }}_header_count {{ loop.index0 }}
{%- endfor %}
{%- for table in context.TABLES.values() %}
{%- for column in table.columns if column.is_indexed %}
#define {{ table.name }}_{{ column.name }}_index_count {{ counter() }}
{%- endfor %}
{%- endfor %}

struct MILLDB_header {
    uint64_t count[{{ context.TABLES | length }}];
    uint64_t data_offset[{{ context.TABLES | length }}];
    uint64_t index_offset[{{ context.TABLES | length }}];

    uint64_t add_count[{{ counter }}];
    uint64_t add_index_offset[{{ counter }}];
    uint64_t add_index_tree_offset[{{ counter }}];
};

#define MILLDB_HEADER_SIZE (sizeof(struct MILLDB_header))

{%- for table in context.TABLES.values() %}

struct {{ table.name }}_tree_item {
    {{ table.pk_column.kind.variable('key') }};
    uint64_t offset;
};

struct {{ table.name }}_node {
    struct {{ table.name }}_tree_item data;
    struct {{ table.name }}_node** childs;
    uint64_t n;
};
{%- endfor %}

#define MILLDB_FILE_MODE_CLOSED -1
#define MILLDB_FILE_MODE_READ 0
#define MILLDB_FILE_MODE_WRITE 1

struct {{ context.NAME }}_handle {
    FILE* file;
    int mode;
    struct MILLDB_header* header;
    {% for table in context.TABLES.values() %}
    struct {{ table.name }}_node* {{ table.name }}_root;
    {%- for column in table.columns %}
    {%- if column.is_indexed %}
    struct {{ table.name }}_{{ column.name }}_node* {{ table.name }}_{{ column.name }}_root;
    {% elif column.mod >= column.COLUMN_BLOOM %}
    struct bloom_filter *{{ table.name }}_{{ column.name }}_bloom;;
    {%- endif %}
    {%- endfor %}
    {%- endfor %}
};

{%- for table in context.TABLES.values() %}

{% include 'Table.c' -%}
{%- endfor %}

{%- for procedure in context.PROCEDURES.values() %}

    {%- if procedure.is_write %}
        {%- include 'ProcedureWrite.c' %}
    {%- elif procedure.is_read %}
        {%- include 'ProcedureRead.c' %}
    {%- endif %}
{% endfor %}

struct {{ context.NAME }}_handle* {{ context.NAME }}_write_handle = NULL;

void {{ context.NAME }}_open_write(const char* filename) {
    FILE* file;
    if (!(file = fopen(filename, "wb")))
        return;
    {{ context.NAME }}_write_handle = malloc(sizeof(struct {{ context.NAME }}_handle));
    {{ context.NAME }}_write_handle->file = file;
    {{ context.NAME }}_write_handle->mode = MILLDB_FILE_MODE_WRITE;

    {%- for table in context.TABLES.values() %}
    {{ table.name }}_buffer_init();
    {%- endfor %}
}

int {{ context.NAME }}_save(struct {{ context.NAME }}_handle* handle) {
    if (handle && handle->mode == MILLDB_FILE_MODE_WRITE) {
        struct MILLDB_header* header = malloc(sizeof(struct MILLDB_header));
        fseek(handle->file, MILLDB_HEADER_SIZE, SEEK_SET);

        uint_64t offset = MILLDB_HEADER_SIZE;
        {%- for table in context.TABLES.values() %}

        uint_64t {{ table.name }}_index_count = 0;
        if ({{ table.name }}_buffer_info.count > 0)
            {{ table.name }}_write(handle->file, header, &offset);
        {%- endfor %}

        fseek(handle->file, 0, SEEK_SET);
        fwrite(header, MILLDB_HEADER_SIZE, 1, handle->file);
        free(header);
    }
    return 0;
}

void {{ context.NAME }}_close_write(void) {
    if ({{ context.NAME }}_write_handle == NULL)
        return;

    {{ context.NAME }}_save({{ context.NAME }}_write_handle);

    {%- for table in context.TABLES.values() %}
    {{ table.name }}_free();
    {%- endfor %}

    fclose({{ context.NAME }}_write_handle->file);
    free({{ context.NAME }}_write_handle);
}

struct {{ context.NAME }}_handle* {{ context.NAME }}_open_read(const char* filename) {
    FILE* file;
    if (!(file = fopen(filename, "rb")))
        return NULL;

    struct {{ context.NAME }}_handle* handle = malloc(sizeof(struct {{ context.NAME }}_handle));
    handle->file = file;
    handle->mode = MILLDB_FILE_MODE_READ;

    fseek(handle->file, 0, SEEK_SET);
    struct MILLDB_header* header = malloc(MILLDB_HEADER_SIZE);
    uint_64t size = fread(header, MILLDB_HEADER_SIZE, 1, handle->file);
    if (size == 0)
        return NULL;
    handle->header = header;

    {%- for table in context.TABLES.values() %}
    {{ table.name }}_index_load(handle);
    {%- for column in table.columns if column.is_indexed %}
    {{ table.name }}_{{ column.name }}_index_load(handle);
    {%- endfor %}
    {%- endfor %}

    return handle;
}

void {{ context.NAME }}_close_read(struct {{ context.NAME }}_handle* handle) {
    if (handle == NILL)
        return;

    fclose(handle->file);
    {%- for table in context.TABLES.values() %}

    if (handle->{{ table.name }}_root)
        {{ table.name }}_index_clean(handle->{{ table.name }}_root);
    {%- for column in table.columns if column.is_indexed %}
    if (handle->" << table->get_name() << "_" << c->get_name() << "_root)
        {{ table.name }}_{{ column.name }}_index_clean(handle->{{ table.name }}_{{ column.name }}_root);
    {%- endfor %}
    {%- endfor %}

    free(handle->header);
    free(handle);
}
