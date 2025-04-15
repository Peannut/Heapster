# 🚀 Heapster: A Modern Garbage Collector for C

<div align="center">

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![Memory Management](https://img.shields.io/badge/memory-managed-orange.svg)]()

</div>

Heapster is a lightweight, efficient mark-and-sweep garbage collector implementation for C programs. It automatically manages memory allocation and deallocation, significantly reducing the risk of memory leaks and making memory management more convenient in C applications.

## ✨ Features

- **Automatic Memory Management** - Let the GC handle memory deallocation for you
- **Mark-and-Sweep Algorithm** - Efficient collection of unreachable objects
- **Simple Application Programming Interface (API)** - Intuitive wrappers around standard memory functions
- **Memory Usage Statistics** - Track allocations and collections
- **Configurable Collection Thresholds** - Control when collection occurs
- **Reference Tracing** - Optional automatic object graph traversal
- **Root Set Management** - Register important objects that should never be collected

## 📋 Table of Contents

- [Installation](#-installation)
- [Quick Start](#-quick-start)
- [API Reference](#-api-reference)
- [How It Works](#-how-it-works)
- [Advanced Usage](#-advanced-usage)
- [Performance Considerations](#-performance-considerations)
- [Best Practices](#-best-practices)

## 🔧 Installation

```bash
# Clone the repository
git clone https://github.com/yourusername/heapster.git

# Build the project
cd heapster
make

# Run the example
./heapster
```

## 🚀 Quick Start

```c
#include "gc.h"

typedef struct {
    int id;
    char *name;
} User;

int main() {
    // Initialize the garbage collector
    gc_init();
    
    // Allocate memory through the GC
    User *user = (User *)gc_malloc(sizeof(User));
    user->id = 1;
    user->name = (char *)gc_malloc(strlen("John Doe") + 1);
    strcpy(user->name, "John Doe");
    
    // Mark objects you want to keep
    gc_mark(user);
    
    // Run garbage collection
    gc_collect();
    
    // Clean up when done
    gc_cleanup();
    
    return 0;
}
```

## 📚 Application Programming Interface (API) Reference

### Core Application Programming Interface (API)

```c
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
```

### Tracer Extension Application Programming Interface (API)

```c
// Register a new object type with its trace function
void gc_register_type(const char *type_name, gc_trace_func trace_func);

// Associate an object with a type
void gc_set_type(void *obj, const char *type_name);

// Automatically trace all reachable objects starting from roots
void gc_trace_all(void **roots, size_t count);

// Enhanced collection that automatically traces from registered roots
size_t gc_collect_traced(void);

// Add an object to the root set
void gc_add_root(void *root);

// Remove an object from the root set
void gc_remove_root(void *root);
```

## 🔍 How It Works

Heapster implements a mark-and-sweep garbage collection algorithm:

1. **Allocation Phase**:
   - Objects are allocated using `gc_malloc()` or `gc_realloc()`
   - Each allocation is tracked in a linked list of memory blocks
   - Each block contains metadata about the allocation (size, mark status)

2. **Marking Phase**:
   - When collection begins, all blocks are initially unmarked
   - The user marks all directly accessible objects with `gc_mark()`
   - With the tracer extension, objects can automatically mark their references

3. **Sweeping Phase**:
   - The collector traverses the list of allocated blocks
   - Any unmarked blocks are freed, as they are no longer reachable
   - Marked blocks are unmarked, preparing for the next collection cycle

4. **Collection Triggering**:
   - Collection occurs automatically when memory usage exceeds a threshold
   - The threshold grows by a factor after each collection
   - Collection can also be triggered manually with `gc_collect()`

<div align="center">

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│  Memory Block   │     │  Memory Block   │     │  Memory Block   │
│ ┌─────────────┐ │     │ ┌─────────────┐ │     │ ┌─────────────┐ │
│ │    size     │ │     │ │    size     │ │     │ │    size     │ │
│ │   marked    │ │     │ │   marked    │ │     │ │   marked    │ │
│ │    next  ───┼─┼────►│ │    next  ───┼─┼────►│ │    next     │ │
│ │    data  ───┼─┼─┐   │ │    data  ───┼─┼─┐   │ │    data  ───┼─┼─┐
│ └─────────────┘ │ │   │ └─────────────┘ │ │   │ └─────────────┘ │ │
└─────────────────┘ │   └─────────────────┘ │   └─────────────────┘ │
                    ▼                       ▼                       ▼
              ┌──────────┐           ┌──────────┐           ┌──────────┐
              │  User    │           │  User    │           │  User    │
              │  Data    │           │  Data    │           │  Data    │
              └──────────┘           └──────────┘           └──────────┘
```

</div>

## 🔋 Advanced Usage

### Custom Object Tracing

For complex object graphs, you can implement custom tracing functions:

```c
// Define a custom object type
typedef struct Node {
    int value;
    struct Node *left;
    struct Node *right;
} Node;

// Define a trace function for the Node type
void trace_node(void *obj) {
    Node *node = (Node *)obj;
    if (node->left) gc_mark(node->left);
    if (node->right) gc_mark(node->right);
}

// Register the type and trace function
gc_register_type("Node", trace_node);

// Associate objects with the type
Node *root = gc_malloc(sizeof(Node));
gc_set_type(root, "Node");

// Later, you can use automatic tracing
gc_add_root(root);
gc_collect_traced(); // Will automatically trace through the tree
```

## ⚡ Performance Considerations

- **Collection Frequency**: More frequent collections reduce memory usage but increase CPU overhead
- **Threshold Tuning**: Adjust `GC_INITIAL_THRESHOLD` and `GC_GROWTH_FACTOR` for your workload
- **Object Marking**: Mark only necessary root objects to avoid keeping unnecessary memory
- **Manual Collection**: Call `gc_collect()` during idle periods for better responsiveness

## 🛠️ Best Practices

- Always initialize the garbage collector with `gc_init()` before use
- Mark all root objects before collection or register them with `gc_add_root()`
- For complex object graphs, implement recursive marking functions
- Clean up the garbage collector with `gc_cleanup()` at program exit
- Use `gc_stats()` to monitor memory usage and collection efficiency
- Consider using the tracer extension for complex object relationships

---

<div align="center">

**Heapster** - Making memory management in C a breeze!

</div>