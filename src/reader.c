#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lisp.h"

void skip_space_and_comments(char** str) {
    while (**str != '\0') {
        if (isspace(**str)) {
            (*str)++;
        } else if (**str == ';') {
            while (**str != '\n' && **str != '\0') {
                (*str)++;
            }
        } else {
            break;
        }
    }
}

static val* read_string(char** str) {
    (*str)++;
    
    char buffer[2048];
    int i = 0;
    
    while (**str != '\0' && **str != '"') {
        if (i < 2047) {
            buffer[i++] = **str;
        }
        (*str)++;
    }
    
    if (**str == '"') {
        (*str)++;
    } else {
        return val_create_err("ERR: unclosed string");
    }
    
    buffer[i] = '\0';
    return val_create_string(buffer);
}

static val* read_list(char** str) {
    skip_space_and_comments(str);

    if (**str == '\0') {
        return val_create_err("ERR: unclosed list (missing ')')");
    }

    if (**str == ')') {
        (*str)++;
        return val_create_nil();
    }

    val* car = val_read(str);

    if (car->type == VAL_ERR) {
        return car;
    }

    val* cdr = read_list(str);

    if (cdr->type == VAL_ERR) {
        val_free(car);
        return cdr;
    }

    return val_create_cons(car, cdr);
}

static val* read_atom(char** str) {
    char buffer[256];
    int i = 0;

    while (**str != '\0' && !isspace(**str) && **str != '(' && **str != ')' && **str != ';') {
        if (i < 255) {
            buffer[i++] = **str;
        }
        (*str)++;
    }
    buffer[i] = '\0';

    char* endptr;
    long num = strtol(buffer, &endptr, 10);

    if (*endptr == '\0') {
        return val_create_int(num);
    } else {
        return val_create_symbol(buffer);
    }
}

val* val_read(char** str) {
    skip_space_and_comments(str);

    if (**str == '\0') {
        return val_create_err("ERR: unexpected EOL");
    }

    if (**str == '(') {
        (*str)++;
        return read_list(str);
    }

    if (**str == ')') {
        (*str)++;
        return val_create_err("ERR: unexpected ')'");
    }

    if (**str == '"') {
        return read_string(str);
    }

    return read_atom(str);
}