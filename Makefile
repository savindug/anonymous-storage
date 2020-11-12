server:
	gcc server.c -o server -lcrypto -lssl 

client:
	gcc client.c -o client

clean:
	rm server
	rm client

all: server client
