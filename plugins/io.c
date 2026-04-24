#include <stdio.h>
#include <string.h>
#include "lisp.h"

val* plugin_read_num(env* e, val* args) {
    long num;
    if (scanf("%ld", &num) != 1) {
        int c; while ((c = getchar()) != '\n' && c != EOF) {}
        return val_create_err("ERR: invalid input, expected a number");
    }
    return val_create_int(num);
}

val* plugin_read_string(env* e, val* args) {
    char buffer[256];
    if (scanf("%255s", buffer) != 1) {
        return val_create_err("ERR: invalid input, expected a string");
    }
    return val_create_string(buffer);
}

void lisp_plugin_init(env* e) {
    val* f1 = val_create_fun(plugin_read_num);
    env_put(e, "read-num", f1); val_free(f1);

    val* f2 = val_create_fun(plugin_read_string);
    env_put(e, "read-string", f2); val_free(f2);

    printf("successfully added functions: 'read-num', 'read-string'\n");
}