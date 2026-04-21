#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <ctype.h>
#include "lisp.h"

static void** dl_handles = NULL;
static int dl_count = 0;

int load_plugin(env* e, const char* path) {
    void* handle = dlopen(path, RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "ERR: could not load plugin: %s\n", dlerror());
        return 1;
    }

    typedef void (*init_func_t)(env*);
    init_func_t init = (init_func_t)dlsym(handle, "lisp_plugin_init");

    if (!init) {
        fprintf(stderr, "ERR: plugin '%s' lacks 'lisp_plugin_init' function\n", path);
        dlclose(handle);
        return 1;
    }

    init(e);
    
    dl_count++;
    dl_handles = realloc(dl_handles, sizeof(void*) * dl_count);
    dl_handles[dl_count - 1] = handle;
    
    printf("loaded plugin: %s\n", path);
    return 0;
}

void cleanup_plugins() {
    for (int i = 0; i < dl_count; i++) {
        dlclose(dl_handles[i]);
    }
    free(dl_handles);
    dl_handles = NULL;
    dl_count = 0;
}

int run_file(env* e, const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "ERR: could not open file '%s'\n", filename);
        return 1; 
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buffer = malloc(length + 1);
    fread(buffer, 1, length, f);
    buffer[length] = '\0';
    fclose(f);

    char* str = buffer;
    int exit_code = 0;
    
    while (*str != '\0') {
        skip_space_and_comments(&str); 
        if (*str == '\0') break;

        val* expr = val_read(&str);
        if (expr->type == VAL_ERR) {
            val_print(expr);
            val_free(expr);
            exit_code = 1;
            break;
        }

        val* result = val_eval(e, expr);
        
        if (result->type == VAL_ERR) {
            val_print(result);
            val_free(expr);
            val_free(result);
            exit_code = 1;
            break;
        }

        val_free(expr);
        val_free(result);
    }

    free(buffer);
    return exit_code;
}