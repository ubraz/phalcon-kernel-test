CC        = gcc
PHPCONFIG = $(shell phpenv which php-config)

CFLAGS  += -I. $(shell $(PHPCONFIG) --includes) -O0 -g3
LDFLAGS += -lcunit -L$(shell $(PHPCONFIG) --prefix)/lib -lphp5 -Wl,-rpath,$(shell $(PHPCONFIG) --prefix)/lib

KERNEL_SOURCES = \
	kernel/memory.c \
	kernel/array.c \
	kernel/assert.c \
	kernel/backtrace.c \
	kernel/concat.c \
	kernel/debug.c \
	kernel/exception.c \
	kernel/fcall.c \
	kernel/file.c \
	kernel/filter.c \
	kernel/hash.c \
	kernel/main.c \
	kernel/memory.c \
	kernel/object.c \
	kernel/operators.c \
	kernel/output.c \
	kernel/persistent.c \
	kernel/require.c \
	kernel/string.c \
	kernel/variables.c \
	kernel/alternative/fcall.c \
	kernel/framework/orm.c \
	phalcon_exception.c

KERNEL_OBJS = $(patsubst %.c,%.o,$(KERNEL_SOURCES))

SOURCES = \
	main.c \
	extension.c \
	embed.c \
	test_memory.c

OBJS = $(patsubst %.c,%.o,$(SOURCES))

all: test-phalcon-kernel

test-phalcon-kernel: $(OBJS) $(KERNEL_OBJS)
	$(CC) -o "$@" $^ $(LDFLAGS)

clean:
	rm -f $(OBJS) $(KERNEL_OBJS) test-phalcon-kernel

distclean: clean
