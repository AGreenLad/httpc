CC = cc
CFLAGS = -Wall -Wextra -Werror -Iinclude -g
SRC_DIR = ./src
OBJ_DIR = ./obj
TARGET = bin/debug/http
# how can i do this ↓ but better... hmmmm....
SRC_NAMES = request.c map.c http.c socket.c vec.c response.c parse_request.c tpool.c
SRCS = $(SRC_NAMES:%.c=$(SRC_DIR)/%.c)
OBJS = $(SRC_NAMES:%.c=$(OBJ_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@


clean:
	rm -f obj/*; rm bin/debug/http