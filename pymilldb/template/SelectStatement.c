{%- for statement in procedure.statements %}

void {{ procedure.name }}_{{ loop.index }}(struct {{ procedure.name }}_out* iter
    {%- for param in procedure.parameters.values() -%}
        {%- if param.is_input -%}
            {{ ', ' ~ param.signature }}
        {%- endif -%}
    {%- endfor -%}
) {
    {%- for table_data in statement.tables.values() %}
    {%- set table = table_data['table'] %}
    {%- for cond in table_data['conditions'] %}
    /* table {{ table.name }}    cond: {{ cond }} */
    {%- endfor %}
    {%- endfor %}
    {%- for table_data in statement.tables.values() %}
    {%- set table = table_data['table'] %}
    {%- for cond in table_data['conditions'] if cond.obj_left.mod >= cond.obj_left.COLUMN_BLOOM %}
    if (!is_{{ table.name }}_{{ cond.obj_left.name }}_bloom(iter->service.handle, {{ cond.obj_right.name }})) {
        return;
    }
    {%- endfor %}
    {%- endfor %}
    struct {{ context.NAME }}_handle* handle = iter->service.handle;
    struct {{ procedure.name }}_out_data* inserted = malloc(sizeof(struct {{ procedure.name }}_out_data));
    {%- set is_ind, table_data, ind_column = statement.check_ind() %}
    {%- if is_ind %}
    {%- set table = table_data['table'] %}
    {%- set conditions = table_data['conditions'] %}
    uint64_t info_offset;

    struct {{ table.name }}_{{ ind_column.name }}_node* node = handle->{{ table.name }}_{{ ind_column.name }}_root;
    uint64_t i = 0;
    while (1) {
        {%- if isinstance(ind_column, context.Char) %}
        if (!strncmp(node->data.key, {{ ind_column.name }}, {{ ind_column.kind.size }}) || node->childs == NULL)
        {%- else %}
        if (node->data.key == {{ ind_column.name }} || node->childs == NULL)
        {%- endif %} {
            info_offset = node->data.offset;
            break;
        }
        {%- if isinstance(ind_column, context.Char) %}
        if (strncmp(node->childs[i]->data.key, {{ ind_column.name }}, {{ ind_column.kind.size }}) > 0 && i > 0)
        {%- else %}
        if (node->childs[i]->data.key == {{ ind_column.name }} > 0 && i > 0)
        {%- endif %} {
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

    uint64_t *offsets;
    uint64_t off_count = 0;

    int break_flag = 0;
    while (1) {
        fseek(handle->file, info_offset, SEEK_SET);
        struct {{ table.name }}_{{ ind_column.name }}_index_item items[{{ table.name }}_{{ ind_column.name }}_CHILDREN];
        uint64_t size = fread(items, sizeof(struct {{ table.name }}_{{ ind_column.name }}_index_item), {{ table.name }}_{{ ind_column.name }}_CHILDREN, handle->file);
        if (size == 0)
            return;

        for (uint64_t i = 0; i < {{ table.name }}_{{ ind_column.name }}_CHILDREN; i++) {
            {%- if isinstance(ind_column, context.Char) %}
            if (strncmp(items[i].key, {{ ind_column.name }}, {{ ind_column.kind.size }}) > 0 || info_offset + i * sizeof(struct {{ table.name }}_{{ ind_column.name }}_index_item) >= handle->header->add_index_tree_offset[{{ table.name }}_{{ ind_column.name }}_index_count])
            {%- else %}
            if (items[i].key > {{ ind_column.name }} || info_offset + i * sizeof(struct {{ table.name }}_{{ ind_column.name }}_index_item) >= handle->header->add_index_tree_offset[{{ table.name }}_{{ ind_column.name }}_index_count])
            {%- endif %} {
                free(inserted);
                return;
            }
            {%- if isinstance(ind_column, context.Char) %}
            if (!strncmp(items[i].key, {{ ind_column.name }}, {{ ind_column.kind.size }}))
            {%- else %}
            if (items[i].key == {{ ind_column.name }})
            {%- endif %} {
                off_count = items[i].count;
                offsets = malloc(sizeof(uint64_t) * off_count);

                fseek(handle->file, items[i].offset, SEEK_SET);
                fread(offsets, sizeof(uint64_t), off_count, handle->file);

                break_flag = 1;
                break;
            }
        }
        if (break_flag) {
            break;
        }
        info_offset += {{ table.name }}_{{ ind_column.name }}_CHILDREN * sizeof(struct {{ table.name }}_{{ ind_column.name }}_index_item);
    }
    for (uint64_t i = 0; i < off_count; i++) {
        struct {{ table.name }} *item = malloc(sizeof(struct {{ table.name }}));
        fseek(handle->file, offsets[i], SEEK_SET);
        fread(item, sizeof(struct {{ table.name }}), 1, handle->file);
        {%- for condition in conditions if not condition.obj_left.kind.is_indexed %}
        {% set column = condition.obj_left %}
        {%- if isinstance(column, context.Char) %}
        if (strncmp(item->{{ column.name }}, {{ column.name }}, {{ column.kind.size }}))
        {%- else %}
        if (item->{{ column.name }} != {{ column.name }})
        {%- endif %} {
            free(item);
            continue;
        }
        {%- endfor %}

        {%- for selection in statement.selections %}
        {{ selection.column.kind.str_column_for_select(selection.column.name) }} = item {{ selection.column.name }};
        {%- endfor %}

        {%- for selection in statement.selections %}
        {{ selection.column.kind.select_expr(selection.parameter.name, selection.column.name) }}
        {%- endfor %}
        {{ context.NAME }}_add(iter, inserted);
    }
    free(inserted);
    {%- else %}
    {%- for table_data in statement.tables.values() %}
{#-tabs #}    {%- set tabs = '    ' * 3 * loop.index0 %}
{#-tabs #}    {%- set table = table_data['table'] %}
{#-tabs #}    {%- set conditions = table_data['conditions'] %}
{#-tabs #}    {%- set table_selections = table_data['selections'] %}
{{ tabs }}    /* TABLE {{ table.name }} */
{#-tabs #}    {%- set _ = statement.check_table_pk(table_data) %}
{{ tabs }}    uint64_t offset = 0;
{#-tabs #}    {%- if table_data['has_pk_cond'] %}
{#-tabs #}    {%- set rhs = 'c_' ~ conditions[0].obj_right.name if isinstance(conditions[0], context.ConditionWithOnlyColumns) else conditions[0].obj_right.name %}
{{ tabs }}    struct {{ table.name }}_node* node = handle->{{ table.name }}_root;
{{ tabs }}    uint64_t i = 0;
{{ tabs }}    while (1) {
{{ tabs }}        if (node->data.key == {{ rhs }} || node->childs == NULL) {
{{ tabs }}            offset = node->data.offset;
{{ tabs }}            break;
{{ tabs }}        }
{{ tabs }}        if (node->childs[i]->data.key > {{ rhs }} && i > 0) {
{{ tabs }}            node = node->childs[i-1];
{{ tabs }}            i = 0;
{{ tabs }}            continue;
{{ tabs }}        }
{{ tabs }}        if (i == node->n-1) {
{{ tabs }}            node = node->childs[i];
{{ tabs }}            i = 0;
{{ tabs }}            continue;
{{ tabs }}        }
{{ tabs }}        i++;
{{ tabs }}    }
{#-tabs #}    {%- endif %}
{{ tabs }}    offset += handle->header->data_offset[{{ table.name }}_header_count];
{#-tabs #}    {#- Check PK bounds #}
{#-tabs #}    {%- set lower, up = statement.condition_tree.calculate_pk_bounds() %}
{{ tabs }}    int32_t id_bound_l = {{ lower or '0' }};
{{ tabs }}    int32_t id_bound_u = {{ up or '2147483647' }};
{{ tabs }}    offset += id_bound_l * sizeof(struct {{ table.name }});
{#-tabs #}
{{ tabs }}    while (1) {
{{ tabs }}        fseek(handle->file, offset, SEEK_SET);
{{ tabs }}        union {{ table.name }}_page page;
{{ tabs }}        uint64_t size = fread(&page, sizeof(struct {{ table.name }}), {{ table.name }}_CHILDREN, handle->file);
{{ tabs }}        if (size == 0)
{{ tabs }}            return;
{#-tabs #}
{{ tabs }}        for (uint64_t i = 0; i < {{ table.name }}_CHILDREN; i++) {
{#-tabs #}            {%- for selection in table_selections %}
{{ tabs }}            {{ selection.column.kind.str_param_for_select(selection.column.name) }} = page.items[i].{{ selection.column.name }};
{#-tabs #}            {%- endfor %}
{#-tabs #}
{#-tabs #}            {%- for column in table.columns.values() %}
{{ tabs }}            {{ column.kind.str_column_for_select(column.name) }} = page.items[i].{{ column.name }};
{#-tabs #}            {%- endfor %}
{#-tabs #}
{#-tabs #}            {%- if table_data['has_pk_cond'] %}
{#-tabs #}            {#- if there is a condition on Primary Key #}
{#-tabs #}            {%- if isinstance(conditions[0], context.ConditionWithOnlyColumns) %}
{{ tabs }}            if (c_{{ conditions[0].obj_left.name }} > {{ rhs }} || offset + i * sizeof(struct {{ table.name }}) >= handle->header->index_offset[{{ table.name }}_header_count]) {
{{ tabs }}                free(inserted);
{{ tabs }}                return;
{{ tabs }}            }
{{ tabs }}            if (c_{{ conditions[0].obj_left.name }} == {{ rhs }})
{#-tabs #}            {%- else %}
{#-tabs #}            {%- if up %}
{{ tabs }}            if (offset + i * sizeof(struct {{ table.name }}) > handle->header->data_offset[{{ table.name }}_header_count] + id_bound_u * sizeof(struct {{ table.name }}))
{#-tabs #}            {%- else %}
{{ tabs }}            if (offset + i * sizeof(struct {{ table.name }}) >= handle->header->index_offset[{{ table.name }}_header_count])
{#-tabs #}            {%- endif %} {
{{ tabs }}                free(inserted);
{{ tabs }}                return;
{{ tabs }}            }
{{ tabs }}            if (1)
{#-tabs #}            {%- endif %}
{#-tabs #}            {%- else %}
{#-tabs #}            {#- no conditions on Primary Key #}
{{ tabs }}            if (offset + i * sizeof(struct {{ table.name }}) >= handle->header->index_offset[{{ table.name }}_header_count]) {
{{ tabs }}                free(inserted);
{{ tabs }}                return;
{{ tabs }}            }
{{ tabs }}            if (1)
{#-tabs #}            {%- endif %} {
{#-tabs #}                {%- set index = statement.remove_join_conditions(table_data) %}
{{ tabs }}                if(!({{ statement.condition_tree }}))
{{ tabs }}                    continue;
{#-tabs #}                {%- if loop.index == statement.tables | length %}
{#-tabs #}                {%- for selection in statement.selections %}
{{ tabs }}                {{ selection.column.kind.select_expr(selection.parameter.name, selection.column.name) }}
{#-tabs #}                {%- endfor %}
{{ tabs }}                {{ procedure.name }}_add(iter, inserted);
{#-tabs #}                {%- endif %}
    {%- endfor %}
    {%- for table_data in statement.tables.values() | reverse %}
    {%- set table = table_data['table'] %}
    {%- set tabs = '    ' * ((statement.tables | length) - loop.index) %}
{{ tabs }}            }
{{ tabs }}        }
{{ tabs }}        offset += {{ table.name }}_CHILDREN * sizeof(struct {{ table.name }});
{{ tabs }}    }
    {%- endfor %}
    {%- endif %}
}
{%- endfor %}
