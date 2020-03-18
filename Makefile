all: client

client: client.c
				 clang -g -o client client.c
