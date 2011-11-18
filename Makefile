CFLAGS += -O3 -ansi -DNDEBUG -Wall

all: bff
	strip bff

bff:

install: bff
	cp bff /usr/bin

clean:
	rm bff
