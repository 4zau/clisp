# clisp
simple lisp interpreter written in c

uses linenoise (https://github.com/antirez/linenoise), and one plugin uses raylib (https://github.com/raysan5/raylib)
everything needed to built everything is already included

other plugins can be built with simple gcc -I../include -fPIC -shared io.c -o io.so
