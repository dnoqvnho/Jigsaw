.PHONY: all clean

CXXFLAGS+=-O3 -Wall

CC=$(CXX)
CFLAGS=$(CXXFLAGS)

FILES = query.o \
		segment.o \
		partitioner.o
		

DRIVERS = main 

DRIVER_OBJS = $(DRIVERS)

all: $(DRIVERS)

clean:
	rm -f $(DRIVER_OBJS)
	rm -f $(FILES)

$(DRIVER_OBJS): $(FILES)

