.PHONY: all clean

CXXFLAGS+=-O3 -Wall
#CXXFLAGS+=-g -O0 -Wall

CC=$(CXX)
CFLAGS=$(CXXFLAGS)

FILES = query.o 

DRIVERS = main 

DRIVER_OBJS = $(DRIVERS)

all: $(DRIVERS)

clean:
	rm -f $(DRIVER_OBJS)
	rm -f $(FILES)

$(DRIVER_OBJS): $(FILES)

