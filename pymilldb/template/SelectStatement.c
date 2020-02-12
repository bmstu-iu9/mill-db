{%- for statement in procedure.statements %}

void {{ procedure.name }}_{{ loop.index }}(
    struct {{ procedure.name }}_out* iter
    {%- set comma = joiner(', ') -%}
    {%- for param in procedure.parameters.values() -%}
        {%- if param.is_input -%}
            {{ comma() }}{{ param.signature }}
        {%- endif -%}
    {%- endfor -%}
) {
    {%- for table_data in statement.tables.values() %}
    {%- set table = table_data['table'] %}
    {%- for cond in table_data['conditions'] %}
    /* table {{ table.name }}    cond: {{ cond.print }} */
    {%- endfor %}
    {%- endfor %}
    {%- for table_data in statement.tables.values() %}
    {%- set table = table_data['table'] %}
    {%- for cond in table_data['conditions'] if cond.column.mod >= cond.column.COLUMN_BLOOM %}
    if (!is_{{ table.name }}_{{ cond.column.name }}_bloom(iter->service.handle, {{ cond.parameter.name }})) {
        return;
    }
    {%- endfor %}
    {%- endfor %}
    struct {{ context.NAME }}_handle* handle = iter->service.handle;
    struct {{ procedure.name }}_out_data* inserted = malloc(sizeof(struct {{ procedure.name }}_out_data));
    {%- for table_data in statement.tables.values() %}
    {%- set table = table_data['table'] %}
    /* TABLE {{ table.name }} */
    {%- endfor %}
}
{%- endfor %}
