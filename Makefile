app: main.c
	gcc -nostdlib -static main.c -o app

run: app
	./app
