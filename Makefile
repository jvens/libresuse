OUT=libresuse.a
SRC=resuse.c
INC=resuse.h
CFLAGS=-D_GNU_SOURCE
CC=gcc

OBJ := $(patsubst %.c,%.o,$(SRC))

all: $(OUT)

$(OUT): $(OBJ)
	ar -cvq $@ $^

%.o: %.c $(INC)
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f $(OUT)
	rm -f $(OBJ)

