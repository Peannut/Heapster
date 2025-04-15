#include "../../include/tracer/gc_tracer.h"
#include <string.h>

#define MAX_TYPES 32

#define MAX_ROOTS 128

static struct {
    gc_type_info_t types[MAX_TYPES];
    size_t count;
} type_registry = {{0}, 0};

static struct {
    void *objects[MAX_ROOTS];
    size_t count;
} root_set = {{0}, 0};

typedef struct object_type_map {
    void *obj;
    const char *type_name;
    struct object_type_map *next;
} object_type_map_t;

static object_type_map_t *type_map = NULL;

void gc_register_type(const char *type_name, gc_trace_func trace_func) {
    if (!type_name || !trace_func || type_registry.count >= MAX_TYPES) {
        return;
    }
    
    for (size_t i = 0; i < type_registry.count; i++) {
        if (strcmp(type_registry.types[i].name, type_name) == 0) {
            type_registry.types[i].trace = trace_func;
            return;
        }
    }
    
    type_registry.types[type_registry.count].name = type_name;
    type_registry.types[type_registry.count].trace = trace_func;
    type_registry.count++;
}

void gc_set_type(void *obj, const char *type_name) {
    if (!obj || !type_name) {
        return;
    }
    
    bool type_exists = false;
    for (size_t i = 0; i < type_registry.count; i++) {
        if (strcmp(type_registry.types[i].name, type_name) == 0) {
            type_exists = true;
            break;
        }
    }
    
    if (!type_exists) {
        return;
    }
    
    object_type_map_t *map = type_map;
    while (map) {
        if (map->obj == obj) {
            map->type_name = type_name;
            return;
        }
        map = map->next;
    }
    
    map = (object_type_map_t *)malloc(sizeof(object_type_map_t));
    if (!map) return;
    
    map->obj = obj;
    map->type_name = type_name;
    map->next = type_map;
    type_map = map;
}

static gc_trace_func find_trace_func(void *obj) {
    if (!obj) return NULL;
    
    object_type_map_t *map = type_map;
    while (map) {
        if (map->obj == obj) {
            for (size_t i = 0; i < type_registry.count; i++) {
                if (strcmp(type_registry.types[i].name, map->type_name) == 0) {
                    return type_registry.types[i].trace;
                }
            }
            break;
        }
        map = map->next;
    }
    
    return NULL;
}

void gc_add_root(void *root) {
    if (!root || root_set.count >= MAX_ROOTS) {
        return;
    }
    
    for (size_t i = 0; i < root_set.count; i++) {
        if (root_set.objects[i] == root) {
            return;
        }
    }
    
    root_set.objects[root_set.count++] = root;
}

void gc_remove_root(void *root) {
    if (!root) return;
    
    for (size_t i = 0; i < root_set.count; i++) {
        if (root_set.objects[i] == root) {
            for (size_t j = i; j < root_set.count - 1; j++) {
                root_set.objects[j] = root_set.objects[j + 1];
            }
            root_set.count--;
            return;
        }
    }
}

static void trace_object(void *obj) {
    if (!obj) return;
    
    gc_mark(obj);
    
    gc_trace_func trace = find_trace_func(obj);
    if (trace) {
        trace(obj);
    }
}

void gc_trace_all(void **roots, size_t count) {
    if (!roots) return;
    
    for (size_t i = 0; i < count; i++) {
        if (roots[i]) {
            trace_object(roots[i]);
        }
    }
}

size_t gc_collect_traced(void) {
    for (size_t i = 0; i < root_set.count; i++) {
        trace_object(root_set.objects[i]);
    }
    
    return gc_collect();
}

void gc_tracer_cleanup(void) {
    object_type_map_t *map = type_map;
    object_type_map_t *next;
    
    while (map) {
        next = map->next;
        free(map);
        map = next;
    }
    
    type_map = NULL;
    type_registry.count = 0;
    root_set.count = 0;
}