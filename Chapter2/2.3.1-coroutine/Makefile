all: hook setjmp ucontext

hook: hook.c
	gcc hook.c -o hook -ldl

setjmp: setjmp.c
	gcc setjmp.c -o setjmp

ucontext: ucontext.c
	gcc ucontext.c -o ucontext