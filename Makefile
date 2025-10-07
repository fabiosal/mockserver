.DEFAULT_GOAL := run

mockserver: main.c
	gcc main.c -o mockserver

clean:
	rm mockserver

run: mockserver
	./mockserver

