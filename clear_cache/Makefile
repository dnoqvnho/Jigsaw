.PHONY: all clean

CXXFLAGS+=-O3 -Wall -lpthread
#CXXFLAGS+=-g -O0 -Wall -lpthread

CC=$(CXX)
CFLAGS=$(CXXFLAGS)

FILES = 


DRIVERS = main

DRIVER_OBJS = $(DRIVERS)

all: $(DRIVERS)

clean:
	rm -f $(DRIVER_OBJS)
	rm -f $(FILES)

$(DRIVER_OBJS): $(FILES)
