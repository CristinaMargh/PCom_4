all: client

client: client.c parson.c helpers.c buffer.c requests.c
		gcc -g -Wall -o client client.c parson.c buffer.c helpers.c requests.c

clean:
		rm -rf client                       
