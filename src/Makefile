CC ?= gcc
CXX ?= g++
CFLAGS += -Wmissing-declarations -Wall -Wextra -mavx2 -mfma -mbmi2 -MMD -DDEC128_LITTLE_ENDIAN=1

ifdef DEBUG
    CFLAGS += -O0 -g
else
    CFLAGS += -O3 -funroll-loops -ftree-vectorize -DNDEBUG 
endif

ifdef PROFILE
    CFLAGS += -pg
endif

CFLAGS += -std=c99
LDLIBS = -lpthread -ldl -lm

CFILES = basic_decimal.c conversion.c

OBJS = $(CFILES:.c=.o)
EXECS = xdec

all: libdec128.a $(EXECS) 

xdec: xdec.c libdec128.a
	$(CC) $(CFLAGS) -o $@ $(filter-out %.hpp %.h, $^) $(LDFLAGS) $(LDLIBS)

libdec128.a: $(OBJS)
	ar -rcs $@ $^

-include $(OBJS:%.o=%.d) $(EXECS:%=%.d)

clean:
	rm -f *.a *.o *.d $(EXECS)

format:
	clang-format -i *.c *.h

.PHONY: all clean format

