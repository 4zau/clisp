#ifndef LISP_H
#define LISP_H

struct env;
typedef struct env env;

#define MAX_CACHE_SIZE 128

typedef struct lambda_cache {
    int ref_count;
    struct val** args;
    struct val** vals;
    int count;
    int head; 
} lambda_cache;

// val

typedef enum {
    VAL_NIL,
    VAL_INT,
    VAL_SYMBOL,
    VAL_STRING,
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
        char* string;
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
            lambda_cache* cache; 
        } lambda;
    };
} val;

val* val_create_nil();
val* val_create_int(long num);
val* val_create_symbol(char* symbol);
val* val_create_string(char* string);
val* val_create_cons(val* car, val* cdr);
val* val_create_err(char* err);
val* val_create_fun(val* (*fun)(env*, val*));
val* val_create_lambda(env* e, val* formals, val* body, int is_pure);

val* val_read(char** str);

val* val_eval(env* e, val* v);

void val_print(val* v);

void val_free(val* v);

val* val_copy(val* v);

int val_eq(val* a, val* b);

// environment

typedef struct env {
    struct env* parent;
    int ref_count;
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

val* builtin_eq(env* e, val* args);
val* builtin_gt(env* e, val* args);
val* builtin_lt(env* e, val* args);

val* builtin_list(env* e, val* args);
val* builtin_car(env* e, val* args);
val* builtin_cdr(env* e, val* args);
val* builtin_cons(env* e, val* args);

val* builtin_print(env* e, val* args);

val* builtin_load_plugin(env* e, val* args);

// external

int run_file(env* e, const char* filename);

int load_plugin(env* e, const char* path);
void cleanup_plugins();


#endif