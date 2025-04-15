#ifndef GC_H
#define GC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Garbage Collector Configuration */
#define GC_INITIAL_THRESHOLD 1024 * 1024  // 1MB initial threshold before collection
#define GC_GROWTH_FACTOR 1.5             // How much to increase threshold after collection

/* Memory Block Structure */
typedef struct gc_block {
    size_t size;                // Size of the allocated memory
    bool marked;                // Mark flag for mark-sweep collection
    struct gc_block *next;      // Next block in the linked list
    void *data;                 // Pointer to the actual data
} gc_block_t;

/* Garbage Collector Context */
typedef struct {
    gc_block_t *blocks;         // Linked list of allocated blocks
    size_t bytes_allocated;     // Current bytes allocated
    size_t collection_threshold; // Threshold to trigger garbage collection
    size_t total_collections;   // Statistics: total collections performed
    size_t total_freed;         // Statistics: total bytes freed
} gc_t;

/* Garbage Collector API */

// Initialize the garbage collector
void gc_init(void);

// Cleanup and destroy the garbage collector
void gc_cleanup(void);

// Allocate memory through the garbage collector
void *gc_malloc(size_t size);

// Reallocate memory through the garbage collector
void *gc_realloc(void *ptr, size_t size);

// Free a specific pointer (manual free)
void gc_free(void *ptr);

// Mark an object as in-use (prevent collection)
void gc_mark(void *ptr);

// Force a garbage collection cycle
size_t gc_collect(void);

// Get statistics about the garbage collector
void gc_stats(size_t *bytes_allocated, size_t *total_collections, size_t *total_freed);

#endif /* GC_H */