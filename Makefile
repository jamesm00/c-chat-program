all: server client

server: src/server/server.c
	gcc -o build/server src/server/server.c
	
client: src/client/client.c
	gcc -o build/client src/client/client.c