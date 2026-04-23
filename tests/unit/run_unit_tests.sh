#!/usr/bin/env bash
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$ROOT_DIR"

cc -std=c11 -Wall -Wextra -Werror -O0 -g0 -fno-ident \
  tests/unit/test_budget.c core/budget_compiler/budget_compiler.c \
  -o tests/unit/test_budget

cc -std=c11 -Wall -Wextra -Werror -O0 -g0 -fno-ident \
  tests/unit/test_static_analysis.c core/static_analysis/static_analysis.c \
  -o tests/unit/test_static_analysis

cc -std=c11 -Wall -Wextra -Werror -O0 -g0 -fno-ident \
  tests/unit/test_normalization.c core/normalization/normalization.c \
  -o tests/unit/test_normalization


cc -std=c11 -Wall -Wextra -Werror -O0 -g0 -fno-ident   tests/unit/test_generator_interface_adversarial.c core/generator_interface/generator_interface.c   -o tests/unit/test_generator_interface_adversarial

tests/unit/test_budget
tests/unit/test_static_analysis
tests/unit/test_normalization
tests/unit/test_generator_interface_adversarial

echo "unit tests passed"
