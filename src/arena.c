#include "arena.h"
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>

// Size is in bytes
Arena arena_new(size_t size) {
    char* buf = malloc(sizeof(char) * size);
    return (Arena) {
        .buffer = buf,
        .current = buf,
        .cap = size
    };
}
void arena_delete(Arena* arena) {
    free(arena->buffer);
}

void* arena_alloc(Arena* arena, size_t size) {
    size_t real_size = (size + 7) & ~ 7;
    assert((arena->current - arena->buffer) + real_size <= arena->cap);

    arena->current += real_size;
    return arena->current - real_size;
}
