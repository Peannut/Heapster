#include "../../include/tracer/gc_tracer.h"
#include <string.h>

/* Maximum number of registered types */
#define MAX_TYPES 32

/* Maximum number of root objects */
#define MAX_ROOTS 128

/* Type registry */
static struct {
    gc_type_info_t types[MAX_TYPES];
    size_t count;
} type_registry = {{0}, 0};

/* Root set */
static struct {
    void *objects[MAX_ROOTS];
    size_t count;
} root_set = {{0}, 0};

/* Object type mapping */
typedef struct object_type_map {
    void *obj;
    const char *type_name;
    struct object_type_map *next;
} object_type_map_t;

static object_type_map_t *type_map = NULL;

/* Register a new object type with its trace function */
void gc_register_type(const char *type_name, gc_trace_func trace_func) {
    if (!type_name || !trace_func || type_registry.count >= MAX_TYPES) {
        return;
    }
    
    // Check if type already exists
    for (size_t i = 0; i < type_registry.count; i++) {
        if (strcmp(type_registry.types[i].name, type_name) == 0) {
            // Update existing type
            type_registry.types[i].trace = trace_func;
            return;
        }
    }
    
    // Add new type
    type_registry.types[type_registry.count].name = type_name;
    type_registry.types[type_registry.count].trace = trace_func;
    type_registry.count++;
}

/* Associate an object with a type */
void gc_set_type(void *obj, const char *type_name) {
    if (!obj || !type_name) {
        return;
    }
    
    // Check if type exists in registry
    bool type_exists = false;
    for (size_t i = 0; i < type_registry.count; i++) {
        if (strcmp(type_registry.types[i].name, type_name) == 0) {
            type_exists = true;
            break;
        }
    }
    
    if (!type_exists) {
        return; // Type not registered
    }
    
    // Check if object already has a type
    object_type_map_t *map = type_map;
    while (map) {
        if (map->obj == obj) {
            // Update existing mapping
            map->type_name = type_name;
            return;
        }
        map = map->next;
    }
    
    // Create new mapping
    map = (object_type_map_t *)malloc(sizeof(object_type_map_t));
    if (!map) return;
    
    map->obj = obj;
    map->type_name = type_name;
    map->next = type_map;
    type_map = map;
}

/* Find the trace function for an object */
static gc_trace_func find_trace_func(void *obj) {
    if (!obj) return NULL;
    
    // Find object's type
    object_type_map_t *map = type_map;
    while (map) {
        if (map->obj == obj) {
            // Find trace function for this type
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

/* Add an object to the root set */
void gc_add_root(void *root) {
    if (!root || root_set.count >= MAX_ROOTS) {
        return;
    }
    
    // Check if already in root set
    for (size_t i = 0; i < root_set.count; i++) {
        if (root_set.objects[i] == root) {
            return;
        }
    }
    
    // Add to root set
    root_set.objects[root_set.count++] = root;
}

/* Remove an object from the root set */
void gc_remove_root(void *root) {
    if (!root) return;
    
    for (size_t i = 0; i < root_set.count; i++) {
        if (root_set.objects[i] == root) {
            // Remove by shifting remaining elements
            for (size_t j = i; j < root_set.count - 1; j++) {
                root_set.objects[j] = root_set.objects[j + 1];
            }
            root_set.count--;
            return;
        }
    }
}

/* Trace an object's references */
static void trace_object(void *obj) {
    if (!obj) return;
    
    // Mark the object itself
    gc_mark(obj);
    
    // Find and call the trace function for this object
    gc_trace_func trace = find_trace_func(obj);
    if (trace) {
        trace(obj);
    }
}

/* Automatically trace all reachable objects starting from roots */
void gc_trace_all(void **roots, size_t count) {
    if (!roots) return;
    
    // Mark all roots
    for (size_t i = 0; i < count; i++) {
        if (roots[i]) {
            trace_object(roots[i]);
        }
    }
}

/* Enhanced collection that automatically traces from registered roots */
size_t gc_collect_traced(void) {
    // Trace from all registered roots
    for (size_t i = 0; i < root_set.count; i++) {
        trace_object(root_set.objects[i]);
    }
    
    // Run the standard collection
    return gc_collect();
}

/* Clean up the tracer resources */
void gc_tracer_cleanup(void) {
    // Free the type map
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