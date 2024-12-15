CC = gcc
CFLAGS = -fdiagnostics-color=always -Wall -Werror -Wextra -Wcast-align -Wfloat-equal -Wformat=2 -Wnull-dereference -Wshadow -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused

SERVER = Server
CLIENT = Client

all: $(SERVER) $(CLIENT)

$(SERVER):
	$(CC) $(CFLAGS) Server.c extra.c -o $(SERVER)

$(CLIENT):
	$(CC) $(CFLAGS) Client.c extra.c -o $(CLIENT)

clean:
	rm -f $(SERVER) $(CLIENT)
	rm -rf GAMES SCORES
	rm -rf *.txt