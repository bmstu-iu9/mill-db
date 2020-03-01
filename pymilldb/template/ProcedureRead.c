
void {{ procedure.name }}_add(struct {{ procedure.name }}_out* iter, struct {{ procedure.name }}_out_data* selected) {
    struct {{ procedure.name }}_out_service* service = &(iter->service);
    if (service->set == NULL) {
        service->size = MILLDB_BUFFER_INIT_SIZE;
        service->set = calloc(service->size, sizeof(struct {{ procedure.name }}_out));
    }
    if (service->length >= service->size) {
        service->size = service->size * 2;
        service->set = realloc(service->set, service->size * sizeof(struct {{ procedure.name }}_out));
    }
    memcpy(&(service->set[service->length++]), selected, sizeof(struct {{ procedure.name }}_out_data));
}
{% include 'SelectStatement.c' %}

void {{ procedure.name }}_init(struct {{ procedure.name }}_out* iter, struct {{ context.NAME }}_handle* handle
{%- for param in procedure.parameters.values() -%}
    {%- if param.is_input -%}
        , {{ param.signature }}
    {%- endif -%}
{%- endfor -%}
) {
    memset(iter, 0, sizeof(*iter));
    iter->service.handle = handle;
    iter->service.set = NULL;
    iter->service.size = 0;
    iter->service.count = 0;
    iter->service.length = 0;

    {{ procedure.name }}_1(iter
    {%- for param in procedure.parameters.values() -%}
        {%- if param.mode == 'IN' -%}
            , {{ param.name }}
        {%- endif -%}
    {%- endfor -%}
    );
}

int {{ procedure.name }}_next(struct {{ procedure.name }}_out* iter) {
    if (iter == NULL)
        return 0;

    struct {{ procedure.name }}_out_service* service = &(iter->service);

    if (service->set != NULL && service->count < service->length) {
        memcpy(&iter->data, &(service->set[service->count]), sizeof(struct {{ procedure.name }}_out_data));
        service->count++;
        return 1;
    } else {
        free(service->set);
    }
    return 0;
}