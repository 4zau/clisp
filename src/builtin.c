#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

static val* builtin_math(env* e, val* args, const char* op) {
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
        curr = curr->cdr;
    }
    return val_create_int(result);
}

val* builtin_add(env* e, val* args) { return builtin_math(e, args, "+"); }
val* builtin_sub(env* e, val* args) { return builtin_math(e, args, "-"); }
val* builtin_mul(env* e, val* args) { return builtin_math(e, args, "*"); }
val* builtin_div(env* e, val* args) { return builtin_math(e, args, "/"); }