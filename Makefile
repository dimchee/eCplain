app: main.c
	gcc -nostdlib -static -Wno-builtin-declaration-mismatch main.c  -o app

debug: main.c
	gcc -nostdlib -static -Wno-builtin-declaration-mismatch -g main.c  -o app
	gdb ./app

strace: app
	strace ./app

run: app
	./app

