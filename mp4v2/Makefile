TARGET = test
#LIBS = -L ../thin/x86_64/lib/libmp4v2.a

LIBS = ../thin/x86_64/lib/libmp4v2.a
CC = g++

CFLAGS= -fPIC -O0 -g -fpermissive

LDFLAGS= -L ../thin/x86_64/lib

INC = -I ../thin/x86_64/include

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)
HEADERS_2 = ../thin/x86_64/include$(wildcard *.h)

%.o: %.c $(HEADERS) 
	$(CC)  $(CFLAGS) $(INC) $(LDFLAGS) $(LIBS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(INC) $(LDFLAGS) $(LIBS) -o $@

clean:
	rm -f *.o
	rm -f $(TARGET)
