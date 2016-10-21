CC=gcc
CFLAGS=-Wall
LIBS=-lwiringPi
OUT = wake_up_light.app
OBJS = wake_up_light.o

all: $(OBJS)
		$(CC) -o $(OUT) $(OBJS) $(CFLAGS) $(LIBS)

clean:
		rm -f $(OBJS)
