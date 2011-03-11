CC         = gcc
CFLAGS    ?= -O2 -pipe -Wall -Wextra -Wno-variadic-macros -Wno-strict-aliasing
PKGCONFIG  = pkg-config
STRIP      = strip
INSTALL    = install

CFLAGS    += $(shell $(PKGCONFIG) --cflags lem)
LUA_PATH   = $(shell $(PKGCONFIG) --variable=path lem)
LUA_CPATH  = $(shell $(PKGCONFIG) --variable=cpath lem)

programs = evdev.so

ifdef NDEBUG
CFLAGS += -DNDEBUG
endif

.PHONY: all strip install clean
.PRECIOUS: %.o

all: $(programs)

%.o: %.c
	@echo '  CC $@'
	@$(CC) $(CFLAGS) -fPIC -nostartfiles -c $< -o $@

%.so: %.o
	@echo '  LD $@'
	@$(CC) -shared $(LDFLAGS) $^ -o $@

%-strip: %
	@echo '  STRIP $<'
	@$(STRIP) $<

strip: $(programs:%=%-strip)

cpath-install:
	@echo "  INSTALL -d $(LUA_CPATH)/lem"
	@$(INSTALL) -d $(DESTDIR)$(LUA_CPATH)/lem

%.so-install: %.so cpath-install
	@echo "  INSTALL $<"
	@$(INSTALL) $< $(DESTDIR)$(LUA_CPATH)/lem/$<

install: $(programs:%=%-install)

clean:
	rm -f $(programs) *.o *.c~ *.h~
