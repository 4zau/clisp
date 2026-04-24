#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

static val* builtin_math(env* e, val* args, char* op) {
    val* curr = args;
    
    while (curr->type == VAL_CONS) {
        if (curr->car->type != VAL_INT) {
            return val_create_err("ERR: math operation expected numbers");
        }
        curr = curr->cdr;
    }

    if (args->type == VAL_NIL) {
        if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) return val_create_int(0);
        if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0) return val_create_int(1);
    }

    curr = args;
    long result = curr->car->num;
    curr = curr->cdr;

    if (curr->type == VAL_NIL && strcmp(op, "-") == 0) {
        return val_create_int(-result);
    }

    while (curr->type == VAL_CONS) {
        long next_val = curr->car->num;
        if (strcmp(op, "+") == 0) result += next_val;
        else if (strcmp(op, "-") == 0) result -= next_val;
        else if (strcmp(op, "*") == 0) result *= next_val;
        else if (strcmp(op, "/") == 0) {
            if (next_val == 0) return val_create_err("ERR: division by zero");
            result /= next_val;
        }
        else if (strcmp(op, "%") == 0) result = result % next_val;
        curr = curr->cdr;
    }
    return val_create_int(result);
}

val* builtin_add(env* e, val* args) { return builtin_math(e, args, "+"); }
val* builtin_sub(env* e, val* args) { return builtin_math(e, args, "-"); }
val* builtin_mul(env* e, val* args) { return builtin_math(e, args, "*"); }
val* builtin_div(env* e, val* args) { return builtin_math(e, args, "/"); }
val* builtin_mod(env* e, val* args) { return builtin_math(e, args, "%"); }


static val* builtin_cmp(env* e, val* args, char* op) {
    if (args->type != VAL_CONS || args->cdr->type != VAL_CONS || args->cdr->cdr->type != VAL_NIL) {
        return val_create_err("ERR: comparison expects exactly 2 arguments");
    }
    
    val* a = args->car;
    val* b = args->cdr->car;

    if (strcmp(op, "=") == 0 && a->type == VAL_STRING && b->type == VAL_STRING) {
        if (strcmp(a->string, b->string) == 0) return val_create_symbol("T");
        else return val_create_nil();
    }
    
    if (a->type != VAL_INT || b->type != VAL_INT) {
        return val_create_err("ERR: comparison expects numbers");
    }

    int res = 0;
    if (strcmp(op, "=") == 0) res = (a->num == b->num);
    else if (strcmp(op, ">") == 0) res = (a->num > b->num);
    else if (strcmp(op, "<") == 0) res = (a->num < b->num);

    if (res) return val_create_symbol("T");
    else return val_create_nil();
}

val* builtin_eq(env* e, val* args) { return builtin_cmp(e, args, "="); }
val* builtin_gt(env* e, val* args) { return builtin_cmp(e, args, ">"); }
val* builtin_lt(env* e, val* args) { return builtin_cmp(e, args, "<"); }


val* builtin_list(env* e, val* args) {
    return val_copy(args);
}

val* builtin_car(env* e, val* args) {
    if (args->type != VAL_CONS || args->cdr->type != VAL_NIL) 
        return val_create_err("ERR: 'car' expects exactly 1 argument");
    
    val* lst = args->car;
    if (lst->type == VAL_NIL) return val_create_nil(); 
    if (lst->type != VAL_CONS) return val_create_err("ERR: 'car' expects a list");
    
    return val_copy(lst->car);
}

val* builtin_cdr(env* e, val* args) {
    if (args->type != VAL_CONS || args->cdr->type != VAL_NIL) 
        return val_create_err("ERR: 'cdr' expects exactly 1 argument");
    
    val* lst = args->car;
    if (lst->type == VAL_NIL) return val_create_nil();
    if (lst->type != VAL_CONS) return val_create_err("ERR: 'cdr' expects a list");
    
    return val_copy(lst->cdr);
}

val* builtin_cons(env* e, val* args) {
    if (args->type != VAL_CONS || args->cdr->type != VAL_CONS || args->cdr->cdr->type != VAL_NIL) 
        return val_create_err("ERR: 'cons' expects exactly 2 arguments");
    
    return val_create_cons(val_copy(args->car), val_copy(args->cdr->car));
}

val* builtin_print(env* e, val* args) {
    val* curr = args;
    while (curr->type == VAL_CONS) {
        if (curr->car->type == VAL_STRING) {
            printf("%s\n", curr->car->string);
        } else {
            val_print(curr->car);
        }
        curr = curr->cdr;
    }
    return val_create_nil();
}

val* builtin_load_plugin(env* e, val* args) {
    if (args->type != VAL_CONS || args->cdr->type != VAL_NIL) {
        return val_create_err("ERR: 'load-plugin' expects exactly 1 argument");
    }
    if (args->car->type != VAL_STRING) {
        return val_create_err("ERR: 'load-plugin' expects a string (path to .so)");
    }
    
    if (load_plugin(e, args->car->string) != 0) {
        char err_buf[512];
        snprintf(err_buf, sizeof(err_buf), "ERR: failed to load plugin from path '%s'", args->car->string);
        return val_create_err(err_buf);
    }
    
    return val_create_nil();
}

val* builtin_run_script(env* e, val* args) {
    if (args->type != VAL_CONS || args->cdr->type != VAL_NIL) {
        return val_create_err("ERR: 'run-script' expects exactly 1 argument");
    }
    if (args->car->type != VAL_STRING) {
        return val_create_err("ERR: 'run-script' expects a string (path to script)");
    }
    
    if (run_file(e, args->car->string) != 0) {
        char err_buf[512];
        snprintf(err_buf, sizeof(err_buf), "ERR: failed to run script from path '%s'", args->car->string);
        return val_create_err(err_buf);
    }
    
    return val_create_nil();
}

val* builtin_not(env* e, val* args) {
    if (args->type != VAL_CONS || args->cdr->type != VAL_NIL) {
        return val_create_err("ERR: 'not' expects exactly 1 argument");
    }

    if (args->car->type != VAL_NIL) {
        return val_create_nil();
    }

    return val_create_symbol("T");
}

val* builtin_is_nil(env* e, val* args) {
    if (args->type != VAL_CONS || args->cdr->type != VAL_NIL) {
        return val_create_err("ERR: 'is_nil' expects exactly 1 argument");
    }

    if (args->car->type == VAL_NIL) {
        return val_create_symbol("T");
    }

    return val_create_nil();
}

val* builtin_begin(env* e, val* args) {
    if (args->type == VAL_NIL) return val_create_nil();
    val* curr = args;
    while (curr->cdr->type == VAL_CONS) {
        curr = curr->cdr;
    }
    return val_copy(curr->car);
}