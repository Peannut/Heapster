#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/core/gc.h"

// Example struct to demonstrate GC usage
typedef struct {
    int id;
    char *name;
    void *next;  // For creating linked structures
} Object;

// Helper function to create an object with GC
Object *create_object(int id, const char *name) {
    Object *obj = (Object *)gc_malloc(sizeof(Object));
    if (!obj) return NULL;
    
    obj->id = id;
    obj->name = (char *)gc_malloc(strlen(name) + 1);
    if (!obj->name) {
        gc_free(obj);
        return NULL;
    }
    
    strcpy(obj->name, name);
    obj->next = NULL;
    return obj;
}

// Function to demonstrate memory allocation and collection
void demo_gc() {
    printf("\n=== Garbage Collector Demo ===\n\n");
    
    // Initialize the garbage collector
    gc_init();
    
    // Allocate some objects
    printf("Creating objects...\n");
    Object *root = create_object(1, "Root Object");
    
    // Create a chain of objects
    Object *current = root;
    for (int i = 2; i <= 5; i++) {
        char name[32];
        sprintf(name, "Object %d", i);
        
        Object *next = create_object(i, name);
        current->next = next;
        current = next;
    }
    
    // Print GC stats before collection
    size_t bytes_allocated, total_collections, total_freed;
    gc_stats(&bytes_allocated, &total_collections, &total_freed);
    printf("Before collection: %zu bytes allocated, %zu collections, %zu bytes freed\n", 
           bytes_allocated, total_collections, total_freed);
    
    // Mark objects we want to keep (just the root in this example)
    printf("Marking root object...\n");
    gc_mark(root);
    
    // Run garbage collection
    printf("Running garbage collection...\n");
    size_t freed = gc_collect();
    printf("Garbage collection freed %zu bytes\n", freed);
    
    // Print GC stats after collection
    gc_stats(&bytes_allocated, &total_collections, &total_freed);
    printf("After collection: %zu bytes allocated, %zu collections, %zu bytes freed\n", 
           bytes_allocated, total_collections, total_freed);
    
    // Clean up the garbage collector
    gc_cleanup();
    printf("\nGarbage collector cleaned up\n");
}

int main(int argc, char **argv) {
    printf("Heapster - A Custom Garbage Collector for C\n");
    
    // Run the garbage collector demo
    demo_gc();
    
    return 0;
}