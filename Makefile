.NOTPARALLEL:

prefix ?= /usr/local
#DIRS = src tests
DIRS = src test

BUILDDIRS = $(DIRS:%=build-%)
CLEANDIRS = $(DIRS:%=clean-%)
FORMATDIRS = $(DIRS:%=format-%)

all: $(BUILDDIRS)

$(DIRS): $(BUILDDIRS)

$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

install: all
	install -d ${prefix} ${prefix}/bin ${prefix}/include ${prefix}/lib
	install -m 0644 -t ${prefix}/include src/basic_decimal.h src/decimal_wrapper.hpp
	install -m 0644 -t ${prefix}/lib src/libdec128.a

format: $(FORMATDIRS)

clean: $(CLEANDIRS)

$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean

$(FORMATDIRS):
	$(MAKE) -C $(@:format-%=%) format

.PHONY: $(DIRS) $(BUILDDIRS) $(CLEANDIRS) $(FORMATDIRS)
.PHONY: all install format
