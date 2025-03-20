CC = gcc
CC_FLAGS = -Wall -g
LD_FLAGS = -lncurses

SRC = main.c
TARGET = modman

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $< -o $@ $(CC_FLAGS) $(LD_FLAGS)
