ARCH=$(shell uname -m)
CC ?= gcc
CXX ?= g++
CFLAGS += -Wmissing-declarations -Wall -Wextra -MMD -I../src/

ifeq ($(ARCH), x86_64)
CFLAGS += -mavx2 -mfma -mbmi2
else ifeq ($(ARCH), aarch64)
CFLAGS += -D__ARM_NEOM__
endif

ifdef DEBUG
    CFLAGS += -O0 -g
else
    CFLAGS += -O3 -funroll-loops -ftree-vectorize -DNDEBUG 
endif

ifdef PROFILE
    CFLAGS += -pg
endif

CFLAGS += -std=c99
CXXFLAGS += $(filter-out -std=c99, $(CFLAGS))  -std=c++17 -static-libstdc++
LDLIBS = -lpthread -ldl -lm

CFILES = basic_decimal.c conversion.c util.c

OBJS = $(CFILES:.c=.o)
EXECS = xdec xdec2

all: $(EXECS) 

xdec: xdec.c ../src/decimal/libdec128.a
	$(CC) $(CFLAGS) -o $@ $(filter-out %.hpp %.h, $^) $(LDFLAGS) $(LDLIBS)

xdec2: xdec2.cpp ../src/decimal/libdec128.a
	$(CXX) $(CXXFLAGS) -o $@ $(filter-out %.hpp %.h, $^) $(LDFLAGS) $(LDLIBS)

libdec128.a: $(OBJS)
	ar -rcs $@ $^

-include $(OBJS:%.o=%.d) $(EXECS:%=%.d)

clean:
	rm -f *.a *.o *.d $(EXECS)

format:
	clang-format -i *.c *.cpp

.PHONY: all clean format

