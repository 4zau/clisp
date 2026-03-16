#include <stdio.h>
#include <stdlib.h>
#include <linenoise.h>
#include <string.h>
#include "lisp.h"

static env* global_env_ptr = NULL;

static void print_help() {
    printf("LISP interpreter in REPL mode\n\n");
    printf("use 'def' to create variables or lambda functions.\n");
}

static env* create_global_env() {
    env* env = env_create(NULL);

    val* fun_add = val_create_fun(builtin_add); env_put(env, "+", fun_add); val_free(fun_add);
    val* fun_sub = val_create_fun(builtin_sub); env_put(env, "-", fun_sub); val_free(fun_sub);
    val* fun_mul = val_create_fun(builtin_mul); env_put(env, "*", fun_mul); val_free(fun_mul);
    val* fun_div = val_create_fun(builtin_div); env_put(env, "/", fun_div); val_free(fun_div);

    val* fun_eq = val_create_fun(builtin_eq); env_put(env, "=", fun_eq); val_free(fun_eq);
    val* fun_gt = val_create_fun(builtin_gt); env_put(env, ">", fun_gt); val_free(fun_gt);
    val* fun_lt = val_create_fun(builtin_lt); env_put(env, "<", fun_lt); val_free(fun_lt);

    val* fun_list = val_create_fun(builtin_list); env_put(env, "list", fun_list); val_free(fun_list);
    val* fun_car = val_create_fun(builtin_car); env_put(env, "car", fun_car); val_free(fun_car);
    val* fun_cdr = val_create_fun(builtin_cdr); env_put(env, "cdr", fun_cdr); val_free(fun_cdr);
    val* fun_cons = val_create_fun(builtin_cons); env_put(env, "cons", fun_cons); val_free(fun_cons);

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
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            print_help();
            return 0;
        } else {
            printf("ERR: unknown argument '%s'\n", argv[1]);
            printf("use lisp --help to get help\n");
            return 1;
        }
    }


    char* line;

    global_env_ptr = create_global_env();
    linenoiseSetCompletionCallback(completion); 
    
    printf("\nLISP is running. CTRL+C to quit.\n\n");
    while ((line = linenoise("lisp> ")) != NULL) {
        if (line[0] != '\0') {
            linenoiseHistoryAdd(line);
            
            const char* str = line;

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
        free(line);
    }

    env_free(global_env_ptr);
    return 0;
}