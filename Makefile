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
CLI_BIN := harness_cli

.PHONY: all clean

all: core/libdeterministic_core.a $(CLI_BIN)

core/libdeterministic_core.a: $(CORE_OBJS)
	$(AR) $(ARFLAGS) $@ $(CORE_OBJS)

$(CLI_BIN): core/harness/harness.o core/normalization/normalization.o core/freeze/freeze.o core/generator_interface/generator_interface.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CORE_OBJS) core/libdeterministic_core.a $(CLI_BIN)
