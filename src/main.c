#include <stdio.h>
#include <stdlib.h>
#include <linenoise.h>
#include "lisp.h"

int main(int argc, char** argv) {
    char* line;

    env* global_env = env_create(NULL);

    val* fun_add = val_create_fun(builtin_add); env_put(global_env, "+", fun_add); val_free(fun_add);
    val* fun_sub = val_create_fun(builtin_sub); env_put(global_env, "-", fun_sub); val_free(fun_sub);
    val* fun_mul = val_create_fun(builtin_mul); env_put(global_env, "*", fun_mul); val_free(fun_mul);
    val* fun_div = val_create_fun(builtin_div); env_put(global_env, "/", fun_div); val_free(fun_div);

    printf("LISP is running. ctrl+c to quit.\n\n");
    while ((line = linenoise("lisp> ")) != NULL) {
        if (line[0] != '\0') {
            linenoiseHistoryAdd(line);
            
            const char* str = line;

            val* expr = val_read(&str);

            if (expr->type == VAL_ERR) {
                val_print(expr);
                val_free(expr);
            } else {
                val* result = val_eval(global_env, expr); 

                val_print(result);

                val_free(expr);
                val_free(result);
            }
        }
        free(line);
    }

    return 0;
}