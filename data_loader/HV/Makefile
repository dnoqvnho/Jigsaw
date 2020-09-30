.PHONY: all clean

CXXFLAGS+=-O3 -Wall -lpthread
#CXXFLAGS+=-g -O0 -Wall -lpthread

CC=$(CXX)
CFLAGS=$(CXXFLAGS)

FILES =         storage/segment.o \
                storage/partition.o \
                index/index.o \
                reorganizer/reorganizer.o


DRIVERS = main

DRIVER_OBJS = $(DRIVERS)

all: $(DRIVERS)

clean:
	rm -f $(DRIVER_OBJS)
	rm -f $(FILES)

$(DRIVER_OBJS): $(FILES)
