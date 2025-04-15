#include "../../include/core/gc.h"

/* Global garbage collector state */
static gc_t gc = {NULL, 0, GC_INITIAL_THRESHOLD, 0, 0};

/* Helper function to find a block from a user pointer */
static gc_block_t *find_block(void *ptr) {
    if (!ptr) return NULL;
    
    gc_block_t *block = gc.blocks;
    while (block) {
        if (block->data == ptr) {
            return block;
        }
        block = block->next;
    }
    return NULL;
}

/* Initialize the garbage collector */
void gc_init(void) {
    gc.blocks = NULL;
    gc.bytes_allocated = 0;
    gc.collection_threshold = GC_INITIAL_THRESHOLD;
    gc.total_collections = 0;
    gc.total_freed = 0;
}

/* Cleanup and destroy the garbage collector */
void gc_cleanup(void) {
    // Free all remaining blocks
    gc_block_t *block = gc.blocks;
    gc_block_t *next;
    
    while (block) {
        next = block->next;
        free(block->data);
        free(block);
        block = next;
    }
    
    gc.blocks = NULL;
    gc.bytes_allocated = 0;
}

/* Allocate memory through the garbage collector */
void *gc_malloc(size_t size) {
    if (size == 0) return NULL;
    
    // Check if we need to collect garbage
    if (gc.bytes_allocated + size > gc.collection_threshold) {
        gc_collect();
    }
    
    // Allocate the block metadata
    gc_block_t *block = (gc_block_t *)malloc(sizeof(gc_block_t));
    if (!block) return NULL;
    
    // Allocate the actual data
    void *data = malloc(size);
    if (!data) {
        free(block);
        return NULL;
    }
    
    // Initialize the block
    block->size = size;
    block->marked = false;
    block->data = data;
    
    // Add to the front of the list
    block->next = gc.blocks;
    gc.blocks = block;
    
    // Update statistics
    gc.bytes_allocated += size;
    
    return data;
}

/* Reallocate memory through the garbage collector */
void *gc_realloc(void *ptr, size_t size) {
    if (!ptr) return gc_malloc(size);
    if (size == 0) {
        gc_free(ptr);
        return NULL;
    }
    
    gc_block_t *block = find_block(ptr);
    if (!block) {
        // Not managed by our GC, fallback to regular realloc
        return realloc(ptr, size);
    }
    
    // Reallocate the memory
    void *new_data = realloc(block->data, size);
    if (!new_data) return NULL;
    
    // Update the block
    gc.bytes_allocated = gc.bytes_allocated - block->size + size;
    block->size = size;
    block->data = new_data;
    
    return new_data;
}

/* Free a specific pointer (manual free) */
void gc_free(void *ptr) {
    if (!ptr) return;
    
    gc_block_t *block = gc.blocks;
    gc_block_t *prev = NULL;
    
    while (block) {
        if (block->data == ptr) {
            // Remove from the list
            if (prev) {
                prev->next = block->next;
            } else {
                gc.blocks = block->next;
            }
            
            // Update statistics
            gc.bytes_allocated -= block->size;
            
            // Free the memory
            free(block->data);
            free(block);
            return;
        }
        
        prev = block;
        block = block->next;
    }
}

/* Mark phase of mark-sweep collection */
void gc_mark(void *ptr) {
    gc_block_t *block = find_block(ptr);
    if (block) {
        block->marked = true;
    }
}

/* Sweep phase of mark-sweep collection */
static size_t gc_sweep(void) {
    gc_block_t *block = gc.blocks;
    gc_block_t *prev = NULL;
    gc_block_t *next;
    size_t freed = 0;
    
    while (block) {
        next = block->next;
        
        if (!block->marked) {
            // This block is not marked, free it
            if (prev) {
                prev->next = next;
            } else {
                gc.blocks = next;
            }
            
            freed += block->size;
            gc.bytes_allocated -= block->size;
            
            free(block->data);
            free(block);
            
            block = next;
        } else {
            // This block is marked, unmark it for the next collection
            block->marked = false;
            prev = block;
            block = next;
        }
    }
    
    return freed;
}

/* Force a garbage collection cycle */
size_t gc_collect(void) {
    // Mark phase would be done by the application
    // by calling gc_mark() on all reachable objects
    
    // Sweep phase
    size_t freed = gc_sweep();
    
    // Update statistics
    gc.total_collections++;
    gc.total_freed += freed;
    
    // Adjust the collection threshold
    gc.collection_threshold = (size_t)(gc.bytes_allocated * GC_GROWTH_FACTOR);
    if (gc.collection_threshold < GC_INITIAL_THRESHOLD) {
        gc.collection_threshold = GC_INITIAL_THRESHOLD;
    }
    
    return freed;
}

/* Get statistics about the garbage collector */
void gc_stats(size_t *bytes_allocated, size_t *total_collections, size_t *total_freed) {
    if (bytes_allocated) *bytes_allocated = gc.bytes_allocated;
    if (total_collections) *total_collections = gc.total_collections;
    if (total_freed) *total_freed = gc.total_freed;
}