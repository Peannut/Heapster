#ifndef GC_TRACER_H
#define GC_TRACER_H

#include "../core/gc.h"

/*
 * This extension provides automatic reference tracing capabilities
 * to the base garbage collector. It allows registering custom
 * trace functions for different object types.
 */

/* Function type for tracing references in custom objects */
typedef void (*gc_trace_func)(void *obj);

/* Object type identifier */
typedef struct {
    const char *name;       // Type name for debugging
    gc_trace_func trace;    // Function to trace references in this type
} gc_type_info_t;

/* Register a new object type with its trace function */
void gc_register_type(const char *type_name, gc_trace_func trace_func);

/* Associate an object with a type */
void gc_set_type(void *obj, const char *type_name);

/* Automatically trace all reachable objects starting from roots */
void gc_trace_all(void **roots, size_t count);

/* Enhanced collection that automatically traces from registered roots */
size_t gc_collect_traced(void);

/* Add an object to the root set */
void gc_add_root(void *root);

/* Remove an object from the root set */
void gc_remove_root(void *root);

#endif /* GC_TRACER_H */