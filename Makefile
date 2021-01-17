all:
	gcc -pthread -o server server.c
	gcc client.c -o client -lncurses
clean:
	rm -f *~client server
