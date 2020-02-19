struct {{ procedure.name }}_out_data {
    {%- for parameter in procedure.parameters.values() if parameter.is_output %}
    {{ parameter.kind.str_out(parameter.name) }};
    {%- endfor %}
};

struct {{ procedure.name }}_out_service {
    struct {{ context.NAME }}_handle* handle;
    struct {{ procedure.name }}_out_data* set;
    int size;
    int length;
    int count;
};

struct {{ procedure.name }}_out {
    struct {{ procedure.name }}_out_service service;
    struct {{ procedure.name }}_out_data data;
};

void {{ procedure.name }}_init(struct {{ procedure.name }}_out* iter, struct {{ context.NAME }}_handle* handle
{%- for param in procedure.parameters.values() -%}
    {%- if param.is_input -%}
        , {{ param.signature }}
    {%- endif -%}
{%- endfor -%}
);
int {{ procedure.name }}_next(struct {{ procedure.name }}_out* iter);