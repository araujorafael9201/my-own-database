CC = gcc
CFLAGS = -Wall

main: main.c query.o table.o user.o
	$(CC) $(CFLAGS) -o main main.c table.o query.o user.o

query: query.c
	$(CC) $(CFLAGS) -c query.c

user: user.c
	$(CC) $(CFLAGS) -c user.c

table: table.c
	$(CC) $(CFLAGS) -c table.c
