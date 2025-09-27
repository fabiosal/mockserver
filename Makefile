.DEFAULT_GOAL := run

netdesk: main.c
	gcc main.c -o mockserver

clean:
	rm mockserver

run: netdesk
	./mockserver

