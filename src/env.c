#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

// djb2 hash function
static unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

static void env_resize(env* e) {
    long old_capacity = e->capacity;
    env_entry* old_entries = e->entries;

    e->capacity *= 2;
    e->entries = calloc(e->capacity, sizeof(env_entry));
    e->count = 0; 

    for (long i = 0; i < old_capacity; i++) {
        if (old_entries[i].symbol != NULL) {
            unsigned long h = hash_string(old_entries[i].symbol) % e->capacity;
            while (e->entries[h].symbol != NULL) {
                h = (h + 1) % e->capacity;
            }
            e->entries[h].symbol = old_entries[i].symbol;
            e->entries[h].value = old_entries[i].value;
            e->count++;
        }
    }
    free(old_entries);
}

env* env_create(env* parent) {
    env* new_env = malloc(sizeof(env));
    new_env->parent = parent;

    new_env->ref_count = 1;
    if (parent != NULL) {
        parent->ref_count++;
    }

    new_env->capacity = 16;
    new_env->count = 0;
    new_env->entries = calloc(new_env->capacity, sizeof(env_entry));
    return new_env;
}

env* env_copy(env* e) {
    if (e != NULL) {
        e->ref_count++;
    }
    return e;
}

void env_free(env* e) {
    if (e == NULL) return;

    e->ref_count--;
    if (e->ref_count > 0) return; 

    for (long i = 0; i < e->capacity; i++) {
        if (e->entries[i].symbol != NULL) {
            free(e->entries[i].symbol);
            val_free(e->entries[i].value);
        }
    }
    free(e->entries);
    
    if (e->parent != NULL) {
        env_free(e->parent); 
    }
    
    free(e);
}

void env_put(env* e, char* symbol, val* v) {
    // resize if were over ~70%
    if (e->count * 10 >= e->capacity * 7) {
        env_resize(e);
    }

    unsigned long h = hash_string(symbol) % e->capacity;
    
    while (e->entries[h].symbol != NULL) {
        if (strcmp(symbol, e->entries[h].symbol) == 0) {
            val_free(e->entries[h].value);
            e->entries[h].value = val_copy(v);
            return;
        }
        h = (h + 1) % e->capacity;
    }
    
    e->entries[h].symbol = strdup(symbol);
    e->entries[h].value = val_copy(v);
    e->count++;
}

val* env_set(env* e, char* symbol, val* v) {
    unsigned long h = hash_string(symbol) % e->capacity;
    unsigned long start_h = h;

    while (e->entries[h].symbol != NULL) {
        if (strcmp(symbol, e->entries[h].symbol) == 0) {
            val_free(e->entries[h].value);
            e->entries[h].value = val_copy(v);
            return val_create_nil();
        }
        h = (h + 1) % e->capacity;
        if (h == start_h) break;
    }
    
    if (e->parent != NULL) {
        return env_set(e->parent, symbol, v);
    }
    
    char buffer[256];
    snprintf(buffer, 256, "ERR: cannot set! undefined variable '%s'", symbol);
    return val_create_err(buffer);
}

val* env_get(env* e, char* symbol) {
    unsigned long h = hash_string(symbol) % e->capacity;
    unsigned long start_h = h;

    while (e->entries[h].symbol != NULL) {
        if (strcmp(symbol, e->entries[h].symbol) == 0) {
            return val_copy(e->entries[h].value);
        }
        h = (h + 1) % e->capacity;
        if (h == start_h) break;
    }
    
    if (e->parent != NULL) {
        return env_get(e->parent, symbol);
    }
    
    char buffer[256];
    snprintf(buffer, 256, "ERR: '%s' not found", symbol);
    return val_create_err(buffer);
}