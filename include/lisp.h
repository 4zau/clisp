#ifndef LISP_H
#define LISP_H

struct env;
typedef struct env env;

// val

typedef enum {
    VAL_NIL,
    VAL_INT,
    VAL_SYMBOL,
    VAL_CONS,
    VAL_ERR,
    VAL_FUN,
    VAL_LAMBDA,
} val_type;

typedef struct val {
    val_type type;
    union {
        long num;
        char* symbol;
        char* err;
        struct {
            struct val* car;
            struct val* cdr;
        };
        struct val* (*fun)(env*, struct val*);
        struct {
            struct env* env;
            struct val* formals;
            struct val* body;
        } lambda;
    };
} val;

val* val_create_nil();
val* val_create_int(long num);
val* val_create_symbol(const char* symbol);
val* val_create_cons(val* car, val* cdr);
val* val_create_err(const char* err);
val* val_create_fun(val* (*fun)(env*, val*));
val* val_create_lambda(env* e, val* formals, val* body);

val* val_read(const char** str);

val* val_eval(env* e, val* v);

void val_print(val* v);

void val_free(val* v);

val* val_copy(val* v);

// environment

typedef struct env {
    struct env* parent;
    long count;
    long capacity;
    char** symbols;
    val** vals;
} env;

env* env_create(env* parent);
env* env_copy(env* e);

void env_free(env* e);

val* env_get(env* e, char* symbol);

void env_put(env* e, char* symbol, val* v);

// builtin

val* builtin_add(env* e, val* args);
val* builtin_sub(env* e, val* args);
val* builtin_mul(env* e, val* args);
val* builtin_div(env* e, val* args);

#endif