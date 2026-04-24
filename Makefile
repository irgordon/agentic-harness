CC := cc
CFLAGS := -std=c11 -Wall -Wextra -Werror -O0 -g0 -fno-ident -fno-omit-frame-pointer -fno-common -fwrapv
AR := ar
ARFLAGS := crD

CORE_SRCS := \
core/budget_compiler/budget_compiler.c \
core/static_analysis/static_analysis.c \
core/normalization/normalization.c \
core/freeze/freeze.c \
core/harness/harness.c \
core/config/config.c \
core/generator_interface/generator_interface.c \
core/ledger/ledger.c

CORE_OBJS := $(CORE_SRCS:.c=.o)
CLI_BIN := harness_cli

.PHONY: all clean reproducible test-unit test-integration test-logging test-schema-validation test-config soak

all: core/libdeterministic_core.a $(CLI_BIN)

core/libdeterministic_core.a: $(CORE_OBJS)
	$(AR) $(ARFLAGS) $@ $(CORE_OBJS)

$(CLI_BIN): core/harness/harness.o core/normalization/normalization.o core/freeze/freeze.o core/config/config.o core/generator_interface/generator_interface.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	LC_ALL=C LANG=C TZ=UTC $(CC) $(CFLAGS) -c $< -o $@

env-normalize:
	@set -eu; \
	LC_ALL=C LANG=C TZ=UTC $(CC) --version | head -n 1 > .determinism_env.lock; \
	LC_ALL=C LANG=C TZ=UTC $(AR) --version | head -n 1 >> .determinism_env.lock; \
	printf 'CFLAGS=%s\n' "$(CFLAGS)" >> .determinism_env.lock; \
	echo "environment normalization lock updated: .determinism_env.lock"

reproducible:
	@set -eu; \
	TMP_DIR="$$(mktemp -d)"; \
	trap 'rm -rf "$$TMP_DIR"' EXIT; \
	$(MAKE) clean >/dev/null; \
	$(MAKE) all >/dev/null; \
	cp core/libdeterministic_core.a "$$TMP_DIR/first.a"; \
	cp $(CLI_BIN) "$$TMP_DIR/first_cli"; \
	$(MAKE) clean >/dev/null; \
	$(MAKE) all >/dev/null; \
	cmp -s core/libdeterministic_core.a "$$TMP_DIR/first.a"; \
	cmp -s $(CLI_BIN) "$$TMP_DIR/first_cli"; \
	echo "reproducible build check passed"

test-unit: all
	./tests/unit/run_unit_tests.sh

test-integration: all
	./tests/integration/test_replay.sh

test-logging: all
	./tests/integration/test_logging.sh

test-schema-validation:
	./tests/integration/test_schema_validation.sh

test-config: all
	./tests/integration/test_config.sh

soak: all
	@set -eu; \
	TMP_DIR="$$(mktemp -d)"; \
	trap 'rm -rf "$$TMP_DIR"' EXIT; \
	./harness_cli run --artifact fixtures/artifacts/basic.c --out-ledger "$$TMP_DIR/ledger-current.jsonl" > "$$TMP_DIR/stdout-current.txt" 2> "$$TMP_DIR/stderr-current.txt"; \
	cp "$$TMP_DIR/ledger-current.jsonl" "$$TMP_DIR/ledger-baseline.jsonl"; \
	cp "$$TMP_DIR/stdout-current.txt" "$$TMP_DIR/stdout-baseline.txt"; \
	cp "$$TMP_DIR/stderr-current.txt" "$$TMP_DIR/stderr-baseline.txt"; \
	for i in $$(seq 2 100); do \
	  ./harness_cli run --artifact fixtures/artifacts/basic.c --out-ledger "$$TMP_DIR/ledger-current.jsonl" > "$$TMP_DIR/stdout-current.txt" 2> "$$TMP_DIR/stderr-current.txt"; \
	  cmp -s "$$TMP_DIR/ledger-baseline.jsonl" "$$TMP_DIR/ledger-current.jsonl"; \
	  cmp -s "$$TMP_DIR/stdout-baseline.txt" "$$TMP_DIR/stdout-current.txt"; \
	  cmp -s "$$TMP_DIR/stderr-baseline.txt" "$$TMP_DIR/stderr-current.txt"; \
	done; \
	echo "soak check passed (100 deterministic runs)"

clean:
	rm -f $(CORE_OBJS) core/libdeterministic_core.a $(CLI_BIN)
