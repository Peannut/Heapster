#include "../../include/core/gc.h"

static gc_t gc = {NULL, 0, GC_INITIAL_THRESHOLD, 0, 0};

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

void gc_init(void) {
    gc.blocks = NULL;
    gc.bytes_allocated = 0;
    gc.collection_threshold = GC_INITIAL_THRESHOLD;
    gc.total_collections = 0;
    gc.total_freed = 0;
}

void gc_cleanup(void) {
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

void *gc_malloc(size_t size) {
    if (size == 0) return NULL;
    
    if (gc.bytes_allocated + size > gc.collection_threshold) {
        gc_collect();
    }
    
    gc_block_t *block = (gc_block_t *)malloc(sizeof(gc_block_t));
    if (!block) return NULL;
    
    void *data = malloc(size);
    if (!data) {
        free(block);
        return NULL;
    }
    
    block->size = size;
    block->marked = false;
    block->data = data;
    
    block->next = gc.blocks;
    gc.blocks = block;
    
    gc.bytes_allocated += size;
    
    return data;
}

void *gc_realloc(void *ptr, size_t size) {
    if (!ptr) return gc_malloc(size);
    if (size == 0) {
        gc_free(ptr);
        return NULL;
    }
    
    gc_block_t *block = find_block(ptr);
    if (!block) {
        return realloc(ptr, size);
    }
    
    void *new_data = realloc(block->data, size);
    if (!new_data) return NULL;
    
    gc.bytes_allocated = gc.bytes_allocated - block->size + size;
    block->size = size;
    block->data = new_data;
    
    return new_data;
}

void gc_free(void *ptr) {
    if (!ptr) return;
    
    gc_block_t *block = gc.blocks;
    gc_block_t *prev = NULL;
    
    while (block) {
        if (block->data == ptr) {
            if (prev) {
                prev->next = block->next;
            } else {
                gc.blocks = block->next;
            }
            
            gc.bytes_allocated -= block->size;
            
            free(block->data);
            free(block);
            return;
        }
        
        prev = block;
        block = block->next;
    }
}

void gc_mark(void *ptr) {
    gc_block_t *block = find_block(ptr);
    if (block) {
        block->marked = true;
    }
}

static size_t gc_sweep(void) {
    gc_block_t *block = gc.blocks;
    gc_block_t *prev = NULL;
    gc_block_t *next;
    size_t freed = 0;
    
    while (block) {
        next = block->next;
        
        if (!block->marked) {
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
            block->marked = false;
            prev = block;
            block = next;
        }
    }
    
    return freed;
}

size_t gc_collect(void) {
    size_t freed = gc_sweep();
    
    gc.total_collections++;
    gc.total_freed += freed;
    
    gc.collection_threshold = (size_t)(gc.bytes_allocated * GC_GROWTH_FACTOR);
    if (gc.collection_threshold < GC_INITIAL_THRESHOLD) {
        gc.collection_threshold = GC_INITIAL_THRESHOLD;
    }
    
    return freed;
}

void gc_stats(size_t *bytes_allocated, size_t *total_collections, size_t *total_freed) {
    if (bytes_allocated) *bytes_allocated = gc.bytes_allocated;
    if (total_collections) *total_collections = gc.total_collections;
    if (total_freed) *total_freed = gc.total_freed;
}