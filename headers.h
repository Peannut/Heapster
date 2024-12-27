typedef struct header {
    unsigned int    size;
    struct header   *next;
} header_t;