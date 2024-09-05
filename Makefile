CC = gcc
CC_FLAGS = -Wall -lncurses

SRC = main.c
TARGET = modman

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $< -o $@ $(CC_FLAGS)
