CC := cc
CFLAGS := -std=c11 -Wall -Wextra -Werror -O0 -g0 -fno-ident
AR := ar
ARFLAGS := rcsD

CORE_SRCS := \
core/budget_compiler/budget_compiler.c \
core/static_analysis/static_analysis.c \
core/normalization/normalization.c \
core/freeze/freeze.c \
core/harness/harness.c \
core/generator_interface/generator_interface.c \
core/ledger/ledger.c

CORE_OBJS := $(CORE_SRCS:.c=.o)

.PHONY: all clean

all: core/libdeterministic_core.a

core/libdeterministic_core.a: $(CORE_OBJS)
	$(AR) $(ARFLAGS) $@ $(CORE_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CORE_OBJS) core/libdeterministic_core.a
