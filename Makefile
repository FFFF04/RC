CC = gcc
CFLAGS = -fdiagnostics-color=always -Wall -Werror -Wextra -Wcast-align -Wfloat-equal -Wformat=2 -Wnull-dereference -Wshadow -Wswitch-default -Wswitch-enum -Wundef -Wunreachable-code -Wunused

SERVER = GS
CLIENT = player

all: $(SERVER) $(CLIENT)

$(SERVER):
	$(CC) $(CFLAGS) extra_server.c Server.c -o $(SERVER)

$(CLIENT):
	$(CC) $(CFLAGS) extra_player.c Client.c -o $(CLIENT)

clean:
	rm -f $(SERVER) $(CLIENT)
	rm -rf GAMES SCORES
	rm -rf *.html