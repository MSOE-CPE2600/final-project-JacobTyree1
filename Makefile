# Compiler and flags
CC = gcc
CFLAGS = -Wall -pthread

# Source files
SERVER_SRC = finalMain.c
CLIENT_SRC = client1.c

# Output executables
SERVER_BIN = server
CLIENT_BIN = client

# Default target
all: $(SERVER_BIN) $(CLIENT_BIN)

# Compile the server
$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Compile the client
$(CLIENT_BIN): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Clean up build files
clean:
	rm -f $(SERVER_BIN) $(CLIENT_BIN)