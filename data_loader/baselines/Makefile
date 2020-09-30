.PHONY: all clean

CXXFLAGS+=-g -O0 -Wall

CC=$(CXX)
CFLAGS=$(CXXFLAGS)

FILES = storage/index.o \
		storage/partition.o \
		load/partition_schema.o \
		load/data_load.o
		

DRIVERS = load/main

DRIVER_OBJS = $(DRIVERS)

all: $(DRIVERS)

clean:
	rm -f $(DRIVER_OBJS)
	rm -f $(FILES)

$(DRIVER_OBJS): $(FILES)

