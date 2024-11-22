CC = gcc
CFLAGS = -Wall -Wextra -Werror

SERVER = Server
CLIENT = Client

all: $(SERVER) $(CLIENT)

$(SERVER):
	$(CC) $(CFLAGS) Server.c extra.c -o $(SERVER)

$(CLIENT):
	$(CC) $(CFLAGS) Client.c extra.c -o $(CLIENT)

clean:
	rm -f $(SERVER) $(CLIENT)