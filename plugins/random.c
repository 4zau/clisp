#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "lisp.h"

val* plugin_get_random(env* e, val* args) {
    if (args->type != VAL_NIL) {
        return val_create_err("ERR: 'get_random' expects exactly no arguments");
    }

    return val_create_int(rand());
}

void lisp_plugin_init(env* e) {
    srand(time(NULL));
    val* fun_pow = val_create_fun(plugin_get_random);
    env_put(e, "get_random", fun_pow);
    val_free(fun_pow);
    
    printf("successfully added function: 'get_random'\n");
}