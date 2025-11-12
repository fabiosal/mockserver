.DEFAULT_GOAL := run

mockserver: main.c
	gcc main.c -o mockserver -lssl -Wall

clean:
	rm mockserver

run: mockserver
	./mockserver

