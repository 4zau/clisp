#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

env* env_create(env* parent) {
    env* new_env = malloc(sizeof(env));
    new_env->parent = parent;
    new_env->capacity = 10;
    new_env->count = 0;
    new_env->symbols = malloc(sizeof(char*) * new_env->capacity);
    new_env->vals = malloc(sizeof(val*) * new_env->capacity);
    return new_env;
}

env* env_copy(env* e) {
    if (e == NULL) return NULL;
    
    env* new_env = malloc(sizeof(env));
    new_env->parent = env_copy(e->parent);
    new_env->capacity = e->capacity;
    new_env->count = e->count;
    new_env->symbols = malloc(sizeof(char*) * new_env->capacity);
    new_env->vals = malloc(sizeof(val*) * new_env->capacity);
    
    for (int i = 0; i < e->count; i++) {
        new_env->symbols[i] = strdup(e->symbols[i]);
        new_env->vals[i] = val_copy(e->vals[i]);
    }
    return new_env;
}

void env_free(env* e) {
    for (int i = 0; i < e->count; i++) {
        free(e->symbols[i]); val_free(e->vals[i]);
    }
    free(e->symbols); free(e->vals);
    free(e);
}

void env_put(env* e, char* symbol, val* v) {
    for (int i = 0; i < e->count; i++) {
        if (strcmp(symbol, e->symbols[i]) == 0) {
            val_free(e->vals[i]);
            e->vals[i] = val_copy(v);
            return;
        }
    }
    if (e->count >= e->capacity) {
        e->capacity *= 2;
        e->symbols = realloc(e->symbols, e->capacity * sizeof(char*));
        e->vals = realloc(e->vals, e->capacity * sizeof(val*));
    }
    e->symbols[e->count] = strdup(symbol);
    e->vals[e->count] = val_copy(v);
    e->count++;
}

val* env_get(env* e, char* symbol) {
    if (e->count == -1) return val_create_err("ERR: environment is empty");
    for (int i = 0; i < e->count; i++) {
        if (strcmp(symbol, e->symbols[i]) == 0) {
            return val_copy(e->vals[i]);
        }
    }
    if (e->parent != NULL) {
        return env_get(e->parent, symbol);
    }
    char buffer[256];
    snprintf(buffer, 256, "ERR: '%s' not found", symbol);
    return val_create_err(buffer);
}