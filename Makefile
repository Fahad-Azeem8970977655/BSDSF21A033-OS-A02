CC = gcc
CFLAGS = -Wall -g
SRC = src/ls_main.c src/ls_utils.c
OBJ = $(SRC:.c=.o)
EXEC = ls_program

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(EXEC)
