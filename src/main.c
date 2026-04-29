#include <stdio.h>
#include <stdlib.h>
#include <linenoise.h>
#include <string.h>
#include <ctype.h>
#include "lisp.h"

static env* global_env_ptr = NULL;

void bind_func(env* e, char* name, val* (*func)(env*, val*)) {
    val* f = val_create_fun(func);
    env_put(e, name, f);
    val_free(f);
}

static void print_help() {
    printf("LISP interpreter in REPL mode\n\n");
    printf("lisp [file] to run a script\n");
    printf("(try ./lisp.out examples/raylib.lisp)\n");
    printf("lisp -p [lib.so] to start with a plugin\n\n");
    printf("use 'def' to create variables or lambda functions (lambda! for no cache).\n");
    printf("use 'set!' to update existing global variables.\n");
    printf("example: (def fib (lambda (n) (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2))))))\n\n");
    printf("use ':l <filename>' (or run-script) to load and run a script file.\n");
    printf("use ':p <path to plugin>' (or load-plugin) to load a plugin.\n");
}

static env* create_global_env() {
    env* env = env_create(NULL);

    bind_func(env, "+", builtin_add);
    bind_func(env, "-", builtin_sub);
    bind_func(env, "*", builtin_mul);
    bind_func(env, "/", builtin_div);
    bind_func(env, "%", builtin_mod);

    bind_func(env, "=", builtin_eq);
    bind_func(env, ">", builtin_gt);
    bind_func(env, "<", builtin_lt);

    bind_func(env, "list", builtin_list);
    bind_func(env, "car", builtin_car);
    bind_func(env, "cdr", builtin_cdr);
    bind_func(env, "cons", builtin_cons);

    bind_func(env, "not", builtin_not);
    bind_func(env, "is_nil", builtin_is_nil);

    bind_func(env, "print", builtin_print);

    bind_func(env, "begin", builtin_begin);

    bind_func(env, "load-plugin", builtin_load_plugin);
    bind_func(env, "run-script", builtin_run_script);

    global_nil.type = VAL_NIL;

    return env;
}

static void completion(const char *buf, linenoiseCompletions *lc) {
    const char* last_word = strrchr(buf, ' ');
    if (!last_word) last_word = strrchr(buf, '('); 
    
    if (last_word) {
        last_word++;
    } else {
        last_word = buf;
    }
    
    int prefix_len = strlen(last_word);
    if (prefix_len == 0) return;

    env* e = global_env_ptr;
    while (e != NULL) {
        for (long i = 0; i < e->capacity; i++) {
            if (e->entries[i].symbol != NULL && strncmp(e->entries[i].symbol, last_word, prefix_len) == 0) {
                char completion_str[512];
                int prefix_pos = last_word - buf;
                snprintf(completion_str, sizeof(completion_str), "%.*s%s", 
                         prefix_pos, buf, e->entries[i].symbol);
                linenoiseAddCompletion(lc, completion_str);
            }
        }
        e = e->parent;
    }
}

int main(int argc, char** argv) {
    global_env_ptr = create_global_env();

    const char* file_to_run = NULL;
    int repl_mode = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_help();
            env_free(global_env_ptr);
            return 0;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--plugin") == 0) {
            if (i + 1 < argc) {
                if (load_plugin(global_env_ptr, argv[++i]) != 0) {
                    env_free(global_env_ptr);
                    return 1;
                }
            } else {
                fprintf(stderr, "ERR: -p/--plugin requires a library path\n");
                env_free(global_env_ptr);
                return 1;
            }
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "ERR: unknown argument '%s'\n", argv[i]);
            env_free(global_env_ptr);
            return 1;
        } else {
            file_to_run = argv[i];
            repl_mode = 0;
        }
    }
    
    if (repl_mode) {
        linenoiseSetCompletionCallback(completion); 
        printf("\nLISP is running. :q! to quit.\n\n");
        
        char* line;
        while ((line = linenoise("lisp> ")) != NULL) {
            if (line[0] != '\0') {
                linenoiseHistoryAdd(line);

                if (strncmp(line, ":l ", 3) == 0) {
                    const char* filename = line + 3;
                    while (isspace(*filename)) filename++;

                    if (*filename == '\0') {
                        printf("ERR: expected filename after ':l'\n");
                    } else {
                        if (run_file(global_env_ptr, filename) == 0) {
                            printf("loaded '%s' successfully.\n", filename);
                        } else {
                            printf("execution of '%s' aborted due to errors.\n", filename);
                        }
                    }
                }
                else if (strncmp(line, ":p ", 3) == 0) {
                    const char* plugin_path = line + 3;
                    while (isspace(*plugin_path)) plugin_path++;

                    if (*plugin_path == '\0') {
                        printf("ERR: expected library path after ':p'\n");
                    } else {
                        load_plugin(global_env_ptr, plugin_path);
                    }
                }
                else if (strncmp(line, ":q!", 3) == 0) {
                    linenoiseFree(line);
                    break;
                }
                else {
                    char* str = line;
                    skip_space_and_comments(&str); 

                    if (*str != '\0') {
                        val* expr = val_read(&str);

                        if (expr->type == VAL_ERR) {
                            val_print(expr);
                            val_free(expr);
                        } else {
                            val* result = val_eval(global_env_ptr, expr); 
                            val_print(result);
                            val_free(expr);
                            val_free(result);
                        }
                    }
                }
            }
            free(line);
        }
    } else {
        run_file(global_env_ptr, file_to_run);
    }

    for (long i = 0; i < global_env_ptr->capacity; i++) {
        if (global_env_ptr->entries[i].symbol != NULL) {
            free(global_env_ptr->entries[i].symbol);
            val_free(global_env_ptr->entries[i].value);
            global_env_ptr->entries[i].symbol = NULL; 
        }
    }
    global_env_ptr->count = 0;
    env_free(global_env_ptr);
    cleanup_plugins();
    return 0;
}