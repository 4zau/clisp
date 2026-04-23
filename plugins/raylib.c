#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include "lisp.h"

// build with gcc -shared -fPIC raylib.c -o raylib.so libraylib.a -I../include -lGL -lm -lpthread -ldl -lrt -lX11

long get_int(val** curr) {
    if (!*curr || (*curr)->type != VAL_CONS || (*curr)->car->type != VAL_INT) return 0;
    long v = (*curr)->car->num;
    *curr = (*curr)->cdr;
    return v;
}

val* rl_init(env* e, val* args) {
    long w = get_int(&args);
    long h = get_int(&args);
    char* title = "raylib on lisp!";
    if (args && args->type == VAL_CONS && args->car->type == VAL_STRING) {
        title = args->car->string;
    }
    InitWindow(w, h, title);
    SetTargetFPS(60);
    return val_create_nil();
}

val* rl_close(env* e, val* args) { CloseWindow(); return val_create_nil(); }
val* rl_begin(env* e, val* args) { BeginDrawing(); return val_create_nil(); }
val* rl_end(env* e, val* args)   { EndDrawing(); return val_create_nil(); }

val* rl_clear(env* e, val* args) {
    Color c = { get_int(&args), get_int(&args), get_int(&args), get_int(&args) };
    ClearBackground(c); 
    return val_create_nil();
}

val* rl_draw_rect(env* e, val* args) {
    long x = get_int(&args); long y = get_int(&args);
    long w = get_int(&args); long h = get_int(&args);
    Color c = { get_int(&args), get_int(&args), get_int(&args), get_int(&args) };
    DrawRectangle(x, y, w, h, c);
    return val_create_nil();
}

val* rl_is_key_down(env* e, val* args) {
    if (IsKeyDown(get_int(&args))) return val_create_symbol("T");
    return val_create_nil();
}

// call a lambda function every frame
val* rl_game_loop(env* e, val* args) {
    if (args->type != VAL_CONS) return val_create_err("ERR: expects 1 lambda function");
    
    val* lisp_update_function = args->car;
    
    while (!WindowShouldClose()) {
        // same thing as calling (lisp-update-function) in repl
        val* call_expr = val_create_cons(val_copy(lisp_update_function), val_create_nil());
        
        val* res = val_eval(e, call_expr);
        
        if (res->type == VAL_ERR) {
            val_print(res); 
            val_free(res); val_free(call_expr);
            break;
        }
        val_free(res); val_free(call_expr);
    }
    return val_create_nil();
}

void lisp_plugin_init(env* e) {
    bind_func(e, "rl-init", rl_init);
    bind_func(e, "rl-close", rl_close);
    bind_func(e, "rl-begin", rl_begin);
    bind_func(e, "rl-end", rl_end);
    bind_func(e, "rl-clear", rl_clear);
    bind_func(e, "rl-draw-rect", rl_draw_rect);
    bind_func(e, "rl-is-key-down", rl_is_key_down);
    bind_func(e, "rl-game-loop", rl_game_loop);

    printf("successfully initialized raylib!\n");
}