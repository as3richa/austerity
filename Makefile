.POSIX:

CC = gcc
CFLAGS = -Iinclude -std=c99 -pedantic -Wall -Wextra -DNDEBUG -O3

TARGET = austerity.a

.PHONY: all

all: $(TARGET)

include config.in

$(TARGET): $(OBJECTS)
	ar rcs $@ $^

clean:
	rm -rf $(TARGET) $(OBJECTS)
