#ifndef {{ context.NAME | upper }}_H
#define {{ context.NAME | upper }}_H

#include <stdint.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

struct {{ context.NAME }}_handle;
{% for sequence in context.SEQUENCES.values() %}
{%- include 'Sequence.h' %}
{% endfor %}

{%- for procedure in context.PROCEDURES.values() %}

    {%- if procedure.is_write %}
        {%- include 'ProcedureWrite.h' %}
    {%- elif procedure.is_read %}
        {%- include 'ProcedureRead.h' %}
    {%- endif %}
{% endfor %}

void {{ context.NAME }}_open_write(const char* filename);
void {{ context.NAME }}_close_write(void);

struct {{ context.NAME }}_handle* {{ context.NAME }}_open_read(const char* filename);
void {{ context.NAME }}_close_read(struct {{ context.NAME }}_handle* handle);

#endif