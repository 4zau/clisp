#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

static lambda_cache* cache_create() {
    lambda_cache* c = malloc(sizeof(lambda_cache));
    c->ref_count = 1;
    c->count = 0;
    c->capacity = 10;
    c->args = malloc(sizeof(val*) * c->capacity);
    c->vals = malloc(sizeof(val*) * c->capacity);
    return c;
}

static void cache_free(lambda_cache* c) {
    c->ref_count--;
    if (c->ref_count > 0) return;
    
    for (int i = 0; i < c->count; i++) {
        val_free(c->args[i]);
        val_free(c->vals[i]);
    }
    free(c->args);
    free(c->vals);
    free(c);
}

val* val_create_nil() {
    val* v = malloc(sizeof(val));
    v->type = VAL_NIL;
    return v;
}

val* val_create_int(long num) {
    val* v = malloc(sizeof(val));
    v->type = VAL_INT;
    v->num = num;
    return v;
}
val* val_create_symbol(char* symbol) {
    val* v = malloc(sizeof(val));
    v->type = VAL_SYMBOL;
    v->symbol = strdup(symbol);
    return v;
}

val* val_create_cons(val* car, val* cdr) {
    val* v = malloc(sizeof(val));
    v->type = VAL_CONS;
    v->car = car;
    v->cdr = cdr;
    return v;
}

val* val_create_fun(val* (*fun)(env*, val*)) {
    val* v = malloc(sizeof(val));
    v->type = VAL_FUN;
    v->fun = fun;
    return v;
}

val* val_create_err(char* err) {
    val* v = malloc(sizeof(val));
    v->type = VAL_ERR;
    v->err = strdup(err);
    return v;
}

val* val_create_lambda(env* e, val* formals, val* body) {
    val* v = malloc(sizeof(val));
    v->type = VAL_LAMBDA;
    v->lambda.env = env_copy(e);
    v->lambda.formals = val_copy(formals);
    v->lambda.body = val_copy(body);
    v->lambda.cache = cache_create();
    return v;
}

static void val_print_expr(val* v);

static void val_print_cons(val* v) {
    val_print_expr(v->car);

    if (v->cdr->type == VAL_CONS) {
        printf(" ");
        val_print_cons(v->cdr);
    } else if (v->cdr->type == VAL_NIL) {
        // empty
    } else {
        printf(" . ");
        val_print_expr(v->cdr);
    }
}

static void val_print_expr(val* v) {
    switch (v->type) {
        case VAL_NIL: printf("NIL"); break;
        case VAL_INT: printf("%ld", v->num); break;
        case VAL_SYMBOL: printf("%s", v->symbol); break;
        case VAL_CONS:
            printf("(");
            val_print_cons(v);
            printf(")");
            break;
        case VAL_FUN: printf("<builtin function>"); break;
        case VAL_LAMBDA: printf("<lambda function>"); break;
        case VAL_ERR: printf("%s", v->err); break;
    }
}

void val_print(val* v) {   
    val_print_expr(v);
    printf("\n");
}

void val_free(val* v) {
    switch (v->type) {
        case VAL_SYMBOL: free(v->symbol); break;
        case VAL_CONS: val_free(v->car); val_free(v->cdr); break;
        case VAL_LAMBDA:
            val_free(v->lambda.formals);
            val_free(v->lambda.body);
            env_free(v->lambda.env);
            cache_free(v->lambda.cache);
            break;
        case VAL_ERR: free(v->err); break;
    }
    free(v);
}

val* val_copy(val* v) {
    switch (v->type) {
        case VAL_NIL: return val_create_nil(); break;
        case VAL_INT: return val_create_int(v->num); break;
        case VAL_SYMBOL: return val_create_symbol(v->symbol); break;
        case VAL_CONS: return val_create_cons(val_copy(v->car), val_copy(v->cdr)); break;
        case VAL_FUN: return val_create_fun(v->fun); break;
        case VAL_LAMBDA:
            val* copy = malloc(sizeof(val));
            copy->type = VAL_LAMBDA;
            copy->lambda.env = env_copy(v->lambda.env);
            copy->lambda.formals = val_copy(v->lambda.formals);
            copy->lambda.body = val_copy(v->lambda.body);
            copy->lambda.cache = v->lambda.cache;
            copy->lambda.cache->ref_count++;
            return copy; break;
        case VAL_ERR: return val_create_err(v->err); break;
    }
    return val_create_err("ERR: couldnt copy val");
}

int val_eq(val* a, val* b) {
    if (a->type != b->type) return 0;
    switch (a->type) {
        case VAL_NIL: return 1;
        case VAL_INT: return a->num == b->num;
        case VAL_SYMBOL:
        case VAL_ERR: return strcmp(a->symbol, b->symbol) == 0;
        case VAL_CONS: return val_eq(a->car, b->car) && val_eq(a->cdr, b->cdr);
        default: return 0;
    }
}