#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lisp.h"

val* val_eval(env* e, val* v);

static val* eval_args(env* e, val* args) {
    if (args->type == VAL_NIL) return val_create_nil();

    val* evaled_car = val_eval(e, args->car);
    if (evaled_car->type == VAL_ERR) return evaled_car;

    val* evaled_cdr = eval_args(e, args->cdr);
    if (evaled_cdr->type == VAL_ERR) {
        val_free(evaled_car);
        return evaled_cdr;
    }

    return val_create_cons(evaled_car, evaled_cdr);
}

val* val_eval(env* e, val* v) {
    if (v->type == VAL_INT) return val_create_int(v->num);
    if (v->type == VAL_STRING) return val_create_string(v->string);
    if (v->type == VAL_ERR) return val_create_err(v->err);
    if (v->type == VAL_NIL) return val_create_nil();
    
    if (v->type == VAL_SYMBOL) {
        return env_get(e, v->symbol);
    }

    if (v->type == VAL_CONS) {
        val* first = v->car;
        
        if (first->type == VAL_SYMBOL && strcmp(first->symbol, "def") == 0) {
            val* args = v->cdr; 
            
            if (args->type != VAL_CONS || args->cdr->type != VAL_CONS) {
                return val_create_err("ERR: 'def' expects exactly 2 arguments");
            }

            if (args->car->type != VAL_SYMBOL) {
                return val_create_err("ERR: 'def' expects a symbol as its first argument");
            }

            char* var_name = args->car->symbol;
            
            val* var_value = val_eval(e, args->cdr->car);
            
            if (var_value->type == VAL_ERR) {
                return var_value;
            }
            
            env_put(e, var_name, var_value);
            
            val_free(var_value);
            return val_create_nil();
        }

        // with cache
        if (first->type == VAL_SYMBOL && strcmp(first->symbol, "lambda") == 0) {
            val* args = v->cdr;
            
            if (args->type != VAL_CONS || args->cdr->type != VAL_CONS) {
                return val_create_err("ERR: 'lambda' expects arguments list and a body");
            }
            
            val* formals = args->car;
            val* body = args->cdr->car;
            
            return val_create_lambda(e, args->car, args->cdr->car, 1); 
        }

        // no cache
        if (first->type == VAL_SYMBOL && strcmp(first->symbol, "lambda!") == 0) {
            val* args = v->cdr;
            if (args->type != VAL_CONS || args->cdr->type != VAL_CONS) {
                return val_create_err("ERR: 'lambda!' expects arguments and a body");
            }
            
            return val_create_lambda(e, args->car, args->cdr->car, 0);
        }

        if (first->type == VAL_SYMBOL && strcmp(first->symbol, "if") == 0) {
            val* args = v->cdr;
            
            if (args->type != VAL_CONS || args->cdr->type != VAL_CONS || 
                args->cdr->cdr->type != VAL_CONS || args->cdr->cdr->cdr->type != VAL_NIL) {
                return val_create_err("ERR: 'if' expects 3 arguments: condition, true-branch, false-branch");
            }
            
            val* cond = args->car;
            val* true_expr = args->cdr->car;
            val* false_expr = args->cdr->cdr->car;
            
            val* cond_res = val_eval(e, cond);
            if (cond_res->type == VAL_ERR) return cond_res;
            
            int is_true = (cond_res->type != VAL_NIL);
            val_free(cond_res);
            
            if (is_true) {
                return val_eval(e, true_expr);
            } else {
                return val_eval(e, false_expr);
            }
        }

        if (first->type == VAL_SYMBOL && strcmp(first->symbol, "exists") == 0) {
            val* args = v->cdr;
            if (args->type != VAL_CONS || args->cdr->type != VAL_NIL) {
                return val_create_err("ERR: 'exists' expects exactly 1 arguments");
            }
            
            if (args->car->type != VAL_SYMBOL) {
                return val_create_err("ERR: 'exists' expects a symbol");
            }

            val* car = args->car;
            
            if (env_get(e, car->symbol)->type == VAL_ERR) {
                return val_create_nil();
            }
            return val_create_symbol("T");
        }

        val* f = val_eval(e, first);
        if (f->type == VAL_ERR) return f;

        if (f->type == VAL_FUN) {
            val* args = eval_args(e, v->cdr);
            if (args->type == VAL_ERR) {
                val_free(f);
                return args;
            }
            
            val* result = f->fun(e, args);
            
            val_free(f);
            val_free(args);
            return result;
        }

        if (f->type == VAL_LAMBDA) {
            val* given_args = eval_args(e, v->cdr);
            if (given_args->type == VAL_ERR) {
                val_free(f);
                return given_args;
            }

            lambda_cache* cache = f->lambda.cache;
            if (cache != NULL) { 
                for (int i = 0; i < cache->count; i++) {
                    if (val_eq(given_args, cache->args[i])) {
                        val* cached_res = val_copy(cache->vals[i]);
                        
                        val_free(given_args);
                        val_free(f);
                        return cached_res;
                    }
                }
            }

            env* call_env = env_create(f->lambda.env);
            val* symbol_node = f->lambda.formals;
            val* val_node = given_args;

            while (symbol_node->type == VAL_CONS && val_node->type == VAL_CONS) {
                env_put(call_env, symbol_node->car->symbol, val_node->car);
                symbol_node = symbol_node->cdr;
                val_node = val_node->cdr;
            }

            if (symbol_node->type != VAL_NIL) {
                env_free(call_env);
                val_free(f);
                val_free(given_args);
                return val_create_err("ERR: too few arguments provided to lambda");
            }
            if (val_node->type != VAL_NIL) {
                env_free(call_env);
                val_free(f);
                val_free(given_args);
                return val_create_err("ERR: too many arguments provided to lambda");
            }

            val* result = val_eval(call_env, f->lambda.body);

            if (result->type != VAL_ERR && cache != NULL) {
                int idx = cache->head;

                if (cache->count == MAX_CACHE_SIZE) {
                    val_free(cache->args[idx]);
                    val_free(cache->vals[idx]);
                } else {
                    cache->count++;
                }

                cache->args[idx] = val_copy(given_args);
                cache->vals[idx] = val_copy(result);
                
                cache->head = (cache->head + 1) % MAX_CACHE_SIZE;
            }

            val_free(given_args);
            env_free(call_env);
            val_free(f);
            
            return result;
        }

        val_free(f);
        return val_create_err("ERR: first element is not a function");
    }

    return val_copy(v);
}