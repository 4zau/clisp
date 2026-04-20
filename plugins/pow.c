#include <stdio.h>
#include "lisp.h"

val* plugin_pow(env* e, val* args) {
    if (args->type != VAL_CONS || args->cdr->type != VAL_CONS || args->cdr->cdr->type != VAL_NIL) {
        return val_create_err("ERR: 'pow' expects exactly 2 arguments");
    }

    val* base_val = args->car;
    val* exp_val = args->cdr->car;

    if (base_val->type != VAL_INT || exp_val->type != VAL_INT) {
        return val_create_err("ERR: 'pow' expects integer arguments");
    }

    long base = base_val->num;
    long exp = exp_val->num;

    if (exp < 0) {
        return val_create_err("ERR: 'pow' does not support negative exponents");
    }

    long result = 1;
    for (long i = 0; i < exp; i++) {
        result *= base;
    }

    return val_create_int(result);
}

void lisp_plugin_init(env* e) {
    val* fun_pow = val_create_fun(plugin_pow);
    env_put(e, "pow", fun_pow);
    val_free(fun_pow);
    
    printf("successfully added function: 'pow'\n");
}