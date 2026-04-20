#include <stdio.h>
#include <stdlib.h>
#include <linenoise.h>
#include <string.h>
#include <ctype.h>
#include "lisp.h"

static env* global_env_ptr = NULL;

static void print_help() {
    printf("LISP interpreter in REPL mode\n");
    printf("lisp [file] to run a script\n");
    printf("lisp -p [lib.so] to start with a plugin\n");
    printf("use 'def' to create variables or lambda functions (lambda! for no cache).\n");
    printf("example: (def fib (lambda (n) (if (< n 2) n (+ (fib (- n 1)) (fib (- n 2))))))\n");
    printf("use ':l <filename>' to load and run a script file.\n");
    printf("use ':p <path to plugin>' (or load-plugin) to load a plugin.\n");
}

static env* create_global_env() {
    env* env = env_create(NULL);

    val* fun_add = val_create_fun(builtin_add); env_put(env, "+", fun_add); val_free(fun_add);
    val* fun_sub = val_create_fun(builtin_sub); env_put(env, "-", fun_sub); val_free(fun_sub);
    val* fun_mul = val_create_fun(builtin_mul); env_put(env, "*", fun_mul); val_free(fun_mul);
    val* fun_div = val_create_fun(builtin_div); env_put(env, "/", fun_div); val_free(fun_div);
    val* fun_mod = val_create_fun(builtin_mod); env_put(env, "%", fun_mod); val_free(fun_mod);

    val* fun_eq = val_create_fun(builtin_eq); env_put(env, "=", fun_eq); val_free(fun_eq);
    val* fun_gt = val_create_fun(builtin_gt); env_put(env, ">", fun_gt); val_free(fun_gt);
    val* fun_lt = val_create_fun(builtin_lt); env_put(env, "<", fun_lt); val_free(fun_lt);

    val* fun_list = val_create_fun(builtin_list); env_put(env, "list", fun_list); val_free(fun_list);
    val* fun_car = val_create_fun(builtin_car); env_put(env, "car", fun_car); val_free(fun_car);
    val* fun_cdr = val_create_fun(builtin_cdr); env_put(env, "cdr", fun_cdr); val_free(fun_cdr);
    val* fun_cons = val_create_fun(builtin_cons); env_put(env, "cons", fun_cons); val_free(fun_cons);

    val* fun_not = val_create_fun(builtin_not); env_put(env, "not", fun_not); val_free(fun_not);
    val* fun_is_nil = val_create_fun(builtin_is_nil); env_put(env, "is_nil", fun_is_nil); val_free(fun_is_nil);
    val* fun_return_nil = val_create_fun(builtin_return_nil); env_put(env, "return_nil", fun_return_nil); val_free(fun_return_nil);

    val* fun_print = val_create_fun(builtin_print); env_put(env, "print", fun_print); val_free(fun_print);

    val* fun_load = val_create_fun(builtin_load_plugin); env_put(env, "load-plugin", fun_load); val_free(fun_load);

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
        for (int i = 0; i < e->count; i++) {
            if (strncmp(e->symbols[i], last_word, prefix_len) == 0) {
                char completion_str[512];
                int prefix_pos = last_word - buf;
                snprintf(completion_str, sizeof(completion_str), "%.*s%s", 
                         prefix_pos, buf, e->symbols[i]);
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
                return 1;
            }
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "ERR: unknown argument '%s'\n", argv[i]);
            return 1;
        } else {
            file_to_run = argv[i];
            repl_mode = 0;
        }
    }
    
    if (repl_mode) {
        linenoiseSetCompletionCallback(completion); 
        printf("\nLISP is running. CTRL+C to quit.\n\n");
        
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
                else {
                    char* str = line;
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
            free(line);
        }
    } else {
        run_file(global_env_ptr, file_to_run);
    }

    env_free(global_env_ptr);
    cleanup_plugins();
    return 0;
}