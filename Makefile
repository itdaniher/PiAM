radio: radio.c
	gcc -c -fPIC -lm -std=c99 radio.c -D _POSIX_C_SOURCE=200809L
	gcc -shared radio.o -o radio.so
clean:
	rm radio.o radio.so
