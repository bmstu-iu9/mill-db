struct {{ table.name }} {
    {%- for column in table.columns.values() %}
    {{ column.kind.variable(column.name) }};
    {%- endfor %}
};
{%- set first_index = table.columns.values() | selectattr('is_indexed') | first %}
{%- if first_index %}

struct {{ table.name }}_1 {
    struct {{ table.name }} *item;
    uint64_t offset;
};
{%- endif %}

struct {{ table.name }}_tree_item* {{ table.name }}_tree_item_new() {
    struct {{ table.name }}_tree_item* new = malloc(sizeof(struct {{ table.name }}_tree_item));
    memset(new, 0, sizeof(*new));
    return new;
}

void {{ table.name }}_tree_item_free(struct {{ table.name }}_tree_item* deleted) {
    free(deleted);
    return;
}

{%- if first_index %}

struct {{ table.name }}_{{ first_index.name }}_index_tree_item* {{ table.name }}_{{ first_index.name }}_tree_item_new() {
    struct {{ table.name }}_{{ first_index.name }}_index_tree_item* new = malloc(sizeof(struct {{ table.name }}_{{ first_index.name }}_index_tree_item));
    memset(new, 0, sizeof(*new));
    return new;

}

void {{ table.name }}_{{ first_index.name }}_tree_item_free(struct {{ table.name }}_{{ first_index.name }}_index_tree_item* deleted) {
    free(deleted);
    return;
}

#define {{ table.name }}_{{ first_index.name }}_CHILDREN (PAGE_SIZE / sizeof(struct {{ table.name }}_{{ first_index.name }}_index_tree_item))
{%- endif %}

#define {{ table.name }}_CHILDREN (PAGE_SIZE / sizeof(struct {{ table.name }}_tree_item))

union {{ table.name }}_page {
    struct {{ table.name }} items[{{ table.name }}_CHILDREN];
    uint8_t as_bytes[PAGE_SIZE];
};

int {{ table.name }}_compare(struct {{ table.name }}* s1, struct {{ table.name }}* s2) {
    {%- for column in table.columns.values() %}
    if ({{ column.kind.compare_greater_expr('s1', column.name, 's2', column.name) }})
        return 1;
    else if ({{ column.kind.compare_less_expr('s1', column.name, 's2', column.name) }})
        return -1;
    {% endfor %}
    return 0;
}
{%- for column in table.columns.values() if column.is_indexed %}

int {{ table.name }}_{{ column.name }}_compare(struct {{ table.name }}* s1, struct {{ table.name }}* s2) {
    if ({{ column.kind.compare_greater_expr('s1', column.name, 's2', column.name) }})
        return 1;
    else if ({{ column.kind.compare_less_expr('s1', column.name, 's2', column.name) }})
        return -1;
    {%- for column_2 in table.columns.values() if column_2 != column %}

    if ({{ column.kind.compare_greater_expr('s1', column_2.name, 's2', column_2.name) }})
        return 1;
    else if ({{ column.kind.compare_less_expr('s1', column_2.name, 's2', column_2.name) }})
        return -1;
    {%- endfor %}

    return 0;
}
{%- endfor %}

struct {{ table.name }}* {{ table.name }}_new() {
    struct {{ table.name }}* new = malloc(sizeof(struct {{ table.name }}));
    memset(new, 0, sizeof(*new));
    return new;
}

void {{ table.name }}_free(struct {{ table.name }}* deleted) {
    free(deleted);
    return;
}

struct {{ table.name }}** {{ table.name }}_buffer = NULL;
struct MILLDB_buffer_info {{ table.name }}_buffer_info;

void {{ table.name }}_buffer_init() {
    {{ table.name }}_buffer_info.size = MILLDB_BUFFER_INIT_SIZE;
    {{ table.name }}_buffer_info.count = 0;
    {{ table.name }}_buffer = calloc({{ table.name }}_buffer_info.size, sizeof(struct {{ table.name }}));
}

void {{ table.name }}_buffer_add(struct {{ table.name }}* inserted) {
    if ({{ table.name }}_buffer_info.count >= {{ table.name }}_buffer_info.size) {
        {{ table.name }}_buffer_info.size *= 2;
        {{ table.name }}_buffer = realloc({{ table.name }}_buffer, {{ table.name }}_buffer_info.size * sizeof(struct {{ table.name }}*));
    }
    {{ table.name }}_buffer[{{ table.name }}_buffer_info.count++] = inserted;
}

void {{ table.name }}_buffer_free() {
    if ({{ table.name }}_buffer == NULL)
        return;

    for (uint64_t i = 0; i < {{ table.name }}_buffer_info.count; i++) {
        {{ table.name }}_free({{ table.name }}_buffer[i]);
    }
    free({{ table.name }}_buffer);
}

int {{ table.name }}_sort_compare(const void* a, const void* b) {
    return {{ table.name }}_compare(*((struct {{ table.name }}**)a), *((struct {{ table.name }}**)b));
}
{%- for column in table.columns.values() if column.is_indexed %}

int {{ table.name }}_{{ column.name }}_sort_compare(const void* a, const void* b) {
    return {{ table.name }}_{{ column.name }}_compare(((struct {{ table.name }}_1*)a)->item, ((struct {{ table.name }}_1*)b)->item);
}
{%- endfor %}

void {{ table.name }}_write(FILE* file, struct MILLDB_header* header, uint64_t *offset) {
    qsort({{ table.name }}_buffer, {{ table.name }}_buffer_info.count, sizeof(struct {{ table.name }}*), {{ table.name }}_sort_compare);
    for (uint64_t i = 0; i < {{ table.name }}_buffer_info.count; i++) {
        fwrite({{ table.name }}_buffer[i], sizeof(struct {{ table.name }}), 1, file);
    }

    uint64_t page_size = {{ table.name }}_CHILDREN, ind_items = 0;
    while (page_size < {{ table.name }}_buffer_info.count) {
        for (uint64_t i = 0; i < {{ table.name }}_buffer_info.count; i += page_size) {
            struct {{ table.name }}_tree_item* item = {{ table.name }}_tree_item_new();
            {%- if isinstance(table.pk_column.kind, context.Char) %}
            strncpy(item->key, {{ table.name }}_buffer[i], {{ table.pk_column.name }}, {{ table.pk_column.kind.size }});
            {%- else %}
            item->key = {{ table.name }}_buffer[i]->{{ table.pk_column.name }};
            {%- endif %}
            item->offset = i * sizeof(struct {{ table.name }});
            fwrite(item, sizeof(struct {{ table.name }}_tree_item), 1, file);
            ind_items++;
            {{ table.name }}_tree_item_free(item);
        }
        page_size *= {{ table.name }}_CHILDREN;
    }

    struct {{ table.name }}_tree_item* item = {{ table.name }}_tree_item_new();
    {%- if isinstance(table.pk_column.kind, context.Char) %}
    strncpy(item->key, {{ table.name }}_buffer[0], {{ table.pk_column.name }}, {{ table.pk_column.kind.size }});
    {%- else %}
    item->key = {{ table.name }}_buffer[0]->{{ table.pk_column.name }};
    {%- endif %}
    item->offset = 0;
    fwrite(item, sizeof(struct {{ table.name }}_tree_item), 1, file);
    {{ table.name }}_tree_item_free(item);
    ind_items++;
    header->count[{{ table.name }}_header_count] = {{ table.name }}_buffer_info.count;
    header->data_offset[{{ table.name }}_header_count] = *offset;
    (*offset) += {{ table.name }}_buffer_info.count * sizeof(struct {{ table.name }});
    header->index_offset[{{ table.name }}_header_count] = *offset;
    (*offset) += ind_items * sizeof(struct {{ table.name }}_tree_item);
    {%- if first_index %}

    struct {{ table.name }}_1 *ind_buf = malloc({{ table.name }}_buffer_info.count * sizeof(struct {{ table.name }}_1));
    for (uint64_t i = 0; i < {{ table.name }}_buffer_info.count; i++) {
        struct {{ table.name }}_1 elem;
        elem.offset = i * sizeof(struct {{ table.name }}) + header->data_offset[{{ table.name }}_header_count];
        elem.item = {{ table.name }}_buffer[i];
        ind_buf[i] = elem;
    }
    {%- endif %}
    {%- for column in table.columns.values() if column.is_indexed %}

    qsort(ind_buf, {{ table.name }}_buffer_info.count, sizeof(struct {{ table.name }}_1), {{ table.name }}_{{ column.name }}_sort_compare);
    struct {{ table.name }}_{{ column.name }}_index_item* {{ column.name }}_items = calloc({{ table.name }}_buffer_info.count, sizeof(struct {{ table.name }}_{{ column.name }}_index_item));
    uint64_t count_{{ column.name }} = 0;
    for (uint64_t i = 0; i < {{ table.name }}_buffer_info.count;) {
        struct {{ table.name }}_{{ column.name }}_index_item ind_it;
        {%- if isinstance(column.kind, context.Char) %}
        memcpy(ind_it.key, ind_buf[i].item->{{ column.name }}, {{ column.kind.size }});
        ind_it.count = 1;
        while (i + ind_it.count < {{ table.name }}_buffer_info.count && !strncmp(ind_it.key, ind_buf[i + ind_it.count].item->{{ column.name }}, {{ column.kind.size }}))
        {%- else %}
        ind_it.key = ind_buf[i].item->{{ column.name }};
        ind_it.count = 1;
        while (i + ind_it.count < {{ table.name }}_buffer_info.count && ind_it.key == ind_buf[i + ind_it.count].item->{{ column.name }})
        {%- endif %} {
            ind_it.count++;
        }
        ind_it.offset = *offset;

        for (int j = 0; j < ind_it.count; j++) {
            fwrite(&ind_buf[i + j].offset, sizeof(uint64_t), 1, file);
        }

        {{ column.name }}_items[count_{{ column.name }}] = ind_it;
        (*offset) += sizeof(uint64_t) * ind_it.count;
        count_{{ column.name }}++;

        i += ind_it.count;
    }

    header->add_index_offset[{{ table.name }}_{{ column.name }}_index_count] = *offset;
    header->add_count[{{ table.name }}_{{ column.name }}_index_count] = count_{{ column.name }};

    for (uint64_t i = 0; i < count_{{ column.name }}; i++) {
        fwrite(&{{ column.name }}_items[i], sizeof(struct {{ table.name }}_{{ column.name }}_index_item), 1, file);
    }

    (*offset) += sizeof(struct {{ table.name }}_{{ column.name }}_index_item) * count_{{ column.name }};
    header->add_index_tree_offset[{{ table.name }}_{{ column.name }}_index_count] = *offset;

    uint64_t page_size_{{ column.name }} = {{ table.name }}_{{ column.name }}_CHILDREN, ind_items_{{ column.name }} = 0;
    while (page_size_{{ column.name }} < count_{{ column.name }}) {
        for (uint64_t i = 0; i < count_{{ column.name }}; i += page_size_{{ column.name }}) {
            struct {{ table.name }}_{{ column.name }}_index_tree_item *item1 = {{ table.name }}_{{ column.name }}_tree_item_new();
            {%- if isinstance(column.kind, context.Char) %}
            memcpy(item1->key, {{ column.name }}_items[i].key, {{ column.kind.size }});
            {%- else %}
            item1->key = {{ column.name }}_items[i].key;
            {%- endif %}
            item1->offset = i * sizeof(struct {{ table.name }}_{{ column.name }}_index_item) + header->add_index_offset[{{ table.name }}_{{ column.name }}_index_count];
            fwrite(item1, sizeof(struct {{ table.name }}_{{ column.name }}_index_tree_item), 1, file);
            (*offset) += sizeof(struct {{ table.name }}_{{ column.name }}_index_tree_item);
            ind_items_{{ column.name }}++;
            {{ table.name }}_{{ column.name }}_tree_item_free(item1);
        }
        page_size_{{ column.name }} *= {{ table.name }}_{{ column.name }}_CHILDREN;
    }


    struct {{ table.name }}_{{ column.name }}_index_tree_item *it_{{ column.name }} = {{ table.name }}_{{ column.name }}_tree_item_new();
    {%- if isinstance(column.kind, context.Char) %}
    memcpy(it_{{ column.name }}->key, {{ column.name }}_items[0].key, {{ context.kind.size }});
    {%- else %}
    it_{{ column.name }}->key = {{ column.name }}_items[0].key;
    {%- endif %}
    it_{{ column.name }}->offset = header->add_index_offset[{{ table.name }}_{{ column.name }}_index_count];
    fwrite(it_{{ column.name }}, sizeof(struct {{ table.name }}_{{ column.name }}_index_tree_item), 1, file);
    {{ table.name }}_{{ column.name }}_tree_item_free(it_{{ column.name }});
    ind_items_{{ column.name }}++;
    free({{ column.name }}_items);
    (*offset) += ind_items_{{ column.name }};
    {%- endfor %}

    {%- if first_index %}
    free(ind_buf);
    {%- endif %}
}
{%- for column in table.columns.values() if column.mod >= column.COLUMN_BLOOM %}

{% set pointer = column.name if isinstance(column, context.Char) else '&' ~ column.name -%}
{% set size = 'sizeof(char)*' if isinstance(column, context.Char) else 'sizeof(' ~ column.name ~ ')' -%}
void add_{{ table.name }}_{{ column.name }}_bloom(struct {{ context.NAME }}_handle* handle, {{ column.kind.signature(column.name) }}) {
    add_bf(handle->{{ table.name }}_{{ column.name }}_bloom, (char *)({{ pointer }}), {{ size }});
}

int is_{{ table.name }}_{{ column.name }}_bloom(struct {{ context.NAME }}_handle* handle, {{ column.kind.signature(column.name) }}) {
    return check_bf(handle->{{ table.name }}_{{ column.name }}_bloom, (char *)({{ pointer }}), {{ size }});
}
{%- endfor %}

void {{ table.name }}_bloom_load(struct {{ context.NAME }}_handle* handle) {
    uint64_t count = handle->header->count[{{ table.name }}_header_count];
    {%- for column in table.columns.values() if column.mod >= column.COLUMN_BLOOM %}

    handle->{{ table.name }}_{{ column.name }}_bloom = new_bf(count, {{ column.fail_share }});
    {%- endfor %}

    uint64_t offset = handle->header->data_offset[{{ table.name }}_header_count];
    fseek(handle->file, offset, SEEK_SET);

    while (offset < handle->header->index_offset[{{ table.name }}_header_count]) {
        struct {{ table.name }} *current_item = malloc(sizeof(struct {{ table.name }}));
        fread(current_item, sizeof(struct {{ table.name }}), 1, handle->file);
        {%- for column in table.columns.values() if column.mod >= column.COLUMN_BLOOM %}
        add_{{ table.name }}_{{ column.name }}_bloom(handle, current_item->{{ column.name }});
        {%- endfor %}
        free(current_item);
        offset += sizeof(struct {{ table.name }});
    }
}

void {{ table.name }}_bloom_delete(struct {{ context.NAME }}_handle* handle) {
    {%- for column in table.columns.values() if column.mod >= column.COLUMN_BLOOM %}
    delete_bf(handle->{{ table.name }}_{{ column.name }}_bloom);
    {%- endfor %}
}

void {{ table.name }}_index_clean(struct {{ table.name }}_node* node) {
    if (node == NULL)
        return;

    for (uint64_t i = 0; i < node->n; i++)
        {{ table.name }}_index_clean(node->childs[i]);

    if (node->childs)
        free(node->childs);
    free(node);
}
{%- for column in table.columns.values() if column.is_indexed %}

void {{ table.name }}_{{ column.name }}_index_clean(struct {{ table.name }}_{{ column.name }}_node* node) {
    if (node == NULL)
        return;

    for (uint64_t i = 0; i < node->n; i++)
        {{ table.name }}_{{ column.name }}_index_clean(node->childs[i]);

    if (node->childs)
        free(node->childs);
    free(node);
}
{%- endfor %}

void {{ table.name }}_index_load(struct {{ context.NAME  }}_handle* handle) {
    if (handle->header->count[{{ table.name }}_header_count] == 0) {
        handle->{{ table.name }}_root = NULL;
        return;
    }
    int32_t levels = log(handle->header->count[{{ table.name }}_header_count]) / log({{ table.name }}_CHILDREN) + 1;
    uint64_t previous_level_count = 0, count = 0;
    struct {{ table.name }}_node** previous_level = NULL;
    struct {{ table.name }}_node** current_level = NULL;

    for (int32_t level = 1; level <= levels; level++) {
        uint64_t current_level_count = (handle->header->count[{{ table.name }}_header_count] + pow({{ table.name }}_CHILDREN, level) - 1) / pow({{ table.name }}_CHILDREN, level);
        current_level = calloc(current_level_count, sizeof(struct {{ table.name }}_node*));

        for (uint64_t i = 0; i < current_level_count; i++) {
            fseek(handle->file, handle->header->index_offset[{{ table.name }}_header_count] + (count++) * sizeof(struct {{ table.name }}_tree_item), SEEK_SET);
            struct {{ table.name }}_tree_item* current_tree_item = malloc(sizeof(struct {{ table.name }}_tree_item));
            uint64_t size = fread(current_tree_item, sizeof(struct {{ table.name }}_tree_item), 1, handle->file);
            if (size == 0)
                return;
            current_level[i] = malloc(sizeof(struct {{ table.name }}_node));
            memcpy(&(current_level[i]->data), current_tree_item, sizeof(struct {{ table.name }}_tree_item));
            free(current_tree_item);
        }

        for (uint64_t i = 0; i < current_level_count; i++) {
            if (!previous_level) {
                current_level[i]->childs = NULL;
                current_level[i]->n = 0;
                continue;
            }

            current_level[i]->childs = calloc({{ table.name }}_CHILDREN, sizeof(struct {{ table.name }}_node*));
            uint64_t j;
            for (j = 0; j < {{ table.name }}_CHILDREN; j++) {
                uint64_t k = i * {{ table.name }}_CHILDREN + j;
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
    handle->{{ table.name }}_root = current_level[0];
    free(current_level);
}
{%- for column in table.columns.values() if column.is_indexed %}

void {{ table.name }}_{{ column.name }}_index_load(struct {{ context.NAME }}_handle* handle) {
    if (handle->header->count[{{ table.name }}_header_count] == 0) {
        handle->{{ table.name }}_root = NULL;
        return;
    }
    int32_t levels = log(handle->header->add_count[{{ table.name }}_{{ column.name }}_index_count]) / log({{ table.name }}_{{ column.name }}_CHILDREN) + 1;
    uint64_t previous_level_count = 0, count = 0;
    struct {{ table.name }}_{{ column.name }}_node** previous_level = NULL;
    struct {{ table.name }}_{{ column.name }}_node** current_level = NULL;

    for (int32_t level = 1; level <= levels; level++) {
        uint64_t current_level_count = (handle->header->add_count[{{ table.name }}_{{ column.name }}_index_count] + pow({{ table.name }}_{{ column.name }}_CHILDREN, level) - 1) / pow({{ table.name }}_{{ column.name }}_CHILDREN, level);
        current_level = calloc(current_level_count, sizeof(struct {{ table.name }}_{{ column.name }}_node*));

        for (uint64_t i = 0; i < current_level_count; i++) {
            fseek(handle->file, handle->header->add_index_tree_offset[{{ table.name }}_{{ column.name }}_index_count] + (count++) * sizeof(struct {{ table.name }}_{{ column.name }}_index_tree_item), SEEK_SET);
            struct {{ table.name }}_{{ column.name }}_index_tree_item* current_tree_item = malloc(sizeof(struct {{ table.name }}_{{ column.name }}_index_tree_item));
            uint64_t size = fread(current_tree_item, sizeof(struct {{ table.name }}_{{ column.name }}_index_tree_item), 1, handle->file);
            if (size == 0)
                return;
            current_level[i] = malloc(sizeof(struct {{ table.name }}_{{ column.name }}_node));
            memcpy(&(current_level[i]->data), current_tree_item, sizeof(struct {{ table.name }}_{{ column.name }}_index_tree_item));
            free(current_tree_item);
        }

        for (uint64_t i = 0; i < current_level_count; i++) {
            if (!previous_level) {
                current_level[i]->childs = NULL;
                current_level[i]->n = 0;
                continue;
            }

            current_level[i]->childs = calloc({{ table.name }}_{{ column.name }}_CHILDREN, sizeof(struct {{ table.name }}_{{ column.name }}_node*));
            uint64_t j;
            for (j = 0; j < {{ table.name }}_{{ column.name }}_CHILDREN; j++) {
                uint64_t k = i * {{ table.name }}_{{ column.name }}_CHILDREN + j;
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
    handle->{{ table.name }}_{{ column.name }}_root = current_level[0];
    free(current_level);
}
{%- endfor %}
