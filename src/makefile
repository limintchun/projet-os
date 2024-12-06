all: client server

client:
	gcc -g -fsanitize=undefined -fsanitize=address  chat/client.c -o client

server:
	gcc -g -fsanitize=undefined -fsanitize=address  serveur/server.c -o server 

clean:
	rm -rf *.dSYM client server .DS_Store

