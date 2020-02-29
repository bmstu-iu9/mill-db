void {{ procedure.name }}(
    {%- set comma = joiner(', ') -%}
    {%- for param in procedure.parameters.values() -%}
        {%- if param.signature -%}
            {{ comma() }}{{ param.signature }}
        {%- endif -%}
    {%- endfor -%}
);