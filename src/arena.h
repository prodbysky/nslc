#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

typedef struct {
    char* buffer;
    char* current;
    size_t cap;
} Arena;

Arena arena_new(size_t size);
void arena_delete(Arena* arena);
void* arena_alloc(Arena* arena, size_t size);

#endif
