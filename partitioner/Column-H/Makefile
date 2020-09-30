.PHONY: all clean

CXXFLAGS+=-O3 -Wall

CXXFLAGS+=-Idist/include/
LDFLAGS+=-Ldist/lib/
LDLIBS+=-lmetis -lpthread

CC=$(CXX)
CFLAGS=$(CXXFLAGS)

FILES = query.o \
		tuples.o \
		graph_partitioner.o
		

DRIVERS = main

DRIVER_OBJS = $(DRIVERS)

all: dist $(DRIVERS)

clean:
	rm -f $(DRIVER_OBJS)
	rm -f $(FILES)
	rm -rf ./dist/

$(DRIVER_OBJS): $(FILES)

dist:
	./pre-init.sh
