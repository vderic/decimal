.NOTPARALLEL:

prefix ?= /usr/local
#DIRS = src tests
DIRS = src/decimal test

BUILDDIRS = $(DIRS:%=build-%)
CLEANDIRS = $(DIRS:%=clean-%)
FORMATDIRS = $(DIRS:%=format-%)

all: $(BUILDDIRS)

$(DIRS): $(BUILDDIRS)

$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

install: all
	install -d ${prefix} ${prefix}/bin ${prefix}/include/decimal ${prefix}/lib
	install -m 0644 -t ${prefix}/include/decimal src/decimal/basic_decimal.h src/decimal/decimal_wrapper.hpp src/decimal/endian.h
	install -m 0644 -t ${prefix}/lib src/decimal/libdec128.a

format: $(FORMATDIRS)

clean: $(CLEANDIRS)

$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean

$(FORMATDIRS):
	$(MAKE) -C $(@:format-%=%) format

.PHONY: $(DIRS) $(BUILDDIRS) $(CLEANDIRS) $(FORMATDIRS)
.PHONY: all install format
