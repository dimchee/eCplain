app: main.c
	gcc -nostdlib -static main.c -Wno-builtin-declaration-mismatch -o app

run: app
	./app
