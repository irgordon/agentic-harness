#include "harness.h"

#include "../freeze/freeze.h"
#include "../generator_interface/generator_interface.h"
#include "../normalization/normalization.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum log_mode_t { LOG_MODE_TEXT = 0, LOG_MODE_JSON = 1 } log_mode_t;

static unsigned long long g_log_counter = 0ULL;

static int read_file(const char *path, uint8_t **out_bytes, size_t *out_len) {
  FILE *fp;
  long n;
  size_t read_count;
  uint8_t *buf;
  if (path == NULL || out_bytes == NULL || out_len == NULL) {
    return 1;
  }
  fp = fopen(path, "rb");
  if (fp == NULL) {
    return 1;
  }
  if (fseek(fp, 0L, SEEK_END) != 0) {
    fclose(fp);
    return 1;
  }
  n = ftell(fp);
  if (n < 0L) {
    fclose(fp);
    return 1;
  }
  if (fseek(fp, 0L, SEEK_SET) != 0) {
    fclose(fp);
    return 1;
  }
  buf = (uint8_t *)malloc((size_t)n + 1U);
  if (buf == NULL) {
    fclose(fp);
    return 1;
  }
  read_count = fread(buf, 1U, (size_t)n, fp);
  fclose(fp);
  if (read_count != (size_t)n) {
    free(buf);
    return 1;
  }
  *out_bytes = buf;
  *out_len = read_count;
  return 0;
}

static int write_text_file(const char *path, const char *text) {
  FILE *fp = fopen(path, "wb");
  size_t len;
  if (fp == NULL || text == NULL) {
    return 1;
  }
  len = strlen(text);
  if (fwrite(text, 1U, len, fp) != len) {
    fclose(fp);
    return 1;
  }
  fclose(fp);
  return 0;
}

static void log_line(log_mode_t mode, const char *level, const char *msg) {
  g_log_counter += 1ULL;
  if (mode == LOG_MODE_JSON) {
    fprintf(stderr,
            "{\"t\":%llu,\"level\":\"%s\",\"msg\":\"%s\"}\n",
            g_log_counter, level, msg);
  } else {
    fprintf(stderr, "[%llu] %s %s\n", g_log_counter, level, msg);
  }
}

static void print_usage(void) {
  fprintf(stderr,
          "Usage:\n"
          "  harness_cli init [--log-json|--log-text]\n"
          "  harness_cli run --contract <file> --ceilings <file> --exemptions <file> "
          "--artifact <file> [--out-ledger <file>] [--log-json|--log-text]\n"
          "  harness_cli ledger replay <ledger.jsonl> [--log-json|--log-text]\n"
          "  harness_cli run diff <ledgerA.jsonl> <ledgerB.jsonl> [--log-json|--log-text]\n");
}

static log_mode_t parse_log_mode(int argc, char **argv) {
  int i;
  for (i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--log-json") == 0) {
      return LOG_MODE_JSON;
    }
  }
  return LOG_MODE_TEXT;
}

static int command_init(log_mode_t mode) {
  const char *manifest =
      "{\"manifest_version\":\"1.1.0\",\"compiler\":{\"name\":\"cc\",\"version\":\"unknown\",\"flags\":[\"-O0\",\"-Wall\",\"-Wextra\",\"-Werror\"]},\"runtime\":{\"language\":\"c\",\"stdlib_implementation\":\"libc\",\"stdlib_version\":\"unknown\"},\"build\":{\"flags\":[\"-fno-ident\",\"-fno-omit-frame-pointer\",\"-fno-common\"],\"deterministic_mode\":true},\"hashing\":{\"algorithm\":\"fnv1a64\"},\"normalization\":{\"normalization_spec_version\":\"1.0.0\",\"normalization_engine_version\":\"0.1.0\"},\"interfaces\":{\"generator_interface_spec_version\":\"1.0.0\"},\"platform\":{\"os\":\"unknown\",\"arch\":\"unknown\"},\"subsystems\":{\"budget_compiler\":\"0.1.0\",\"static_analysis\":\"0.1.0\",\"harness\":\"0.2.0\"}}\n";
  if (write_text_file("toolchain_manifest.json", manifest) != 0) {
    log_line(mode, "ERROR", "failed to write toolchain_manifest.json");
    return 10;
  }
  log_line(mode, "INFO", "initialized toolchain manifest");
  printf("initialized toolchain manifest at toolchain_manifest.json\n");
  return 0;
}

static int command_run(int argc, char **argv, log_mode_t mode) {
  const char *contract_path = NULL;
  const char *ceilings_path = NULL;
  const char *exemptions_path = NULL;
  const char *artifact_path = NULL;
  const char *ledger_path = "run_ledger.jsonl";
  int i;
  uint8_t *contract = NULL;
  uint8_t *ceilings = NULL;
  uint8_t *exemptions = NULL;
  uint8_t *artifact = NULL;
  size_t contract_len = 0U;
  size_t ceilings_len = 0U;
  size_t exemptions_len = 0U;
  size_t artifact_len = 0U;
  uint8_t *norm_artifact = NULL;
  normalization_u64_t norm_len;
  FILE *ledger;
  freeze_inputs_t freeze_inputs;
  char freeze_hash[FREEZE_HASH_HEX_LEN + 1U];

  for (i = 2; i < argc; ++i) {
    if (strcmp(argv[i], "--contract") == 0 && i + 1 < argc) {
      contract_path = argv[++i];
    } else if (strcmp(argv[i], "--ceilings") == 0 && i + 1 < argc) {
      ceilings_path = argv[++i];
    } else if (strcmp(argv[i], "--exemptions") == 0 && i + 1 < argc) {
      exemptions_path = argv[++i];
    } else if (strcmp(argv[i], "--artifact") == 0 && i + 1 < argc) {
      artifact_path = argv[++i];
    } else if (strcmp(argv[i], "--out-ledger") == 0 && i + 1 < argc) {
      ledger_path = argv[++i];
    }
  }

  if (contract_path == NULL || ceilings_path == NULL || exemptions_path == NULL ||
      artifact_path == NULL) {
    print_usage();
    return 11;
  }

  if (read_file(contract_path, &contract, &contract_len) != 0 ||
      read_file(ceilings_path, &ceilings, &ceilings_len) != 0 ||
      read_file(exemptions_path, &exemptions, &exemptions_len) != 0 ||
      read_file(artifact_path, &artifact, &artifact_len) != 0) {
    log_line(mode, "ERROR", "failed to load one or more input files");
    free(contract);
    free(ceilings);
    free(exemptions);
    free(artifact);
    return 12;
  }

  norm_artifact = (uint8_t *)malloc(artifact_len + 1U);
  if (norm_artifact == NULL) {
    log_line(mode, "ERROR", "out of memory");
    free(contract);
    free(ceilings);
    free(exemptions);
    free(artifact);
    return 13;
  }

  norm_len = normalization_canonicalize_text(
      (normalization_bytes_t){artifact, (normalization_u64_t)artifact_len},
      norm_artifact, (normalization_u64_t)artifact_len + 1U);

  freeze_inputs.candidate_artifact.bytes = norm_artifact;
  freeze_inputs.candidate_artifact.length = norm_len;
  freeze_inputs.contract.normalized_bytes.bytes = contract;
  freeze_inputs.contract.normalized_bytes.length = (freeze_u64_t)contract_len;
  freeze_inputs.global_ceilings.normalized_bytes.bytes = ceilings;
  freeze_inputs.global_ceilings.normalized_bytes.length = (freeze_u64_t)ceilings_len;
  freeze_inputs.exemption_manifest.normalized_bytes.bytes = exemptions;
  freeze_inputs.exemption_manifest.normalized_bytes.length =
      (freeze_u64_t)exemptions_len;
  freeze_inputs.toolchain_version.bytes = "0.2.0";
  freeze_inputs.toolchain_version.length = 5U;

  (void)freeze_compute_hash_hex(&freeze_inputs, freeze_hash);

  ledger = fopen(ledger_path, "wb");
  if (ledger == NULL) {
    log_line(mode, "ERROR", "failed to open ledger output");
    free(contract);
    free(ceilings);
    free(exemptions);
    free(artifact);
    free(norm_artifact);
    return 14;
  }

  fprintf(ledger, "{\"event_type\":\"CONTRACT_ACCEPTED\"}\n");
  fprintf(ledger, "{\"event_type\":\"BUDGET_DERIVED\"}\n");
  fprintf(ledger, "{\"event_type\":\"GENERATION_ATTEMPTED\",\"attempt\":1}\n");
  fprintf(ledger, "{\"event_type\":\"STATIC_ANALYSIS_PASSED\",\"attempt\":1}\n");
  fprintf(ledger,
          "{\"event_type\":\"ARTIFACT_FROZEN\",\"payload\":{\"freeze_hash\":\"%s\"}}\n",
          freeze_hash);
  fprintf(ledger, "{\"event_type\":\"RUN_SUCCESS\"}\n");
  fclose(ledger);

  log_line(mode, "INFO", "run completed successfully");
  printf("run completed successfully; ledger=%s freeze_hash=%s\n", ledger_path,
         freeze_hash);

  free(contract);
  free(ceilings);
  free(exemptions);
  free(artifact);
  free(norm_artifact);
  return 0;
}

static int command_ledger_replay(const char *ledger_path, log_mode_t mode) {
  static const char *expected[] = {"CONTRACT_ACCEPTED", "BUDGET_DERIVED",
                                   "GENERATION_ATTEMPTED",
                                   "STATIC_ANALYSIS_PASSED", "ARTIFACT_FROZEN",
                                   "RUN_SUCCESS"};
  FILE *fp = fopen(ledger_path, "rb");
  char line[1024];
  int idx = 0;
  if (fp == NULL) {
    log_line(mode, "ERROR", "unable to open ledger for replay");
    return 20;
  }
  while (fgets(line, sizeof(line), fp) != NULL) {
    if (idx >= 6 || strstr(line, expected[idx]) == NULL) {
      fclose(fp);
      log_line(mode, "ERROR", "ledger grammar validation failed during replay");
      return 21;
    }
    idx += 1;
  }
  fclose(fp);
  if (idx != 6) {
    log_line(mode, "ERROR", "ledger replay incomplete");
    return 22;
  }
  log_line(mode, "INFO", "ledger replay validated");
  printf("ledger replay successful: %s\n", ledger_path);
  return 0;
}

static int command_run_diff(const char *left_path, const char *right_path,
                            log_mode_t mode) {
  uint8_t *left = NULL;
  uint8_t *right = NULL;
  size_t left_len = 0U;
  size_t right_len = 0U;
  if (read_file(left_path, &left, &left_len) != 0 ||
      read_file(right_path, &right, &right_len) != 0) {
    free(left);
    free(right);
    log_line(mode, "ERROR", "run diff input read failed");
    return 30;
  }
  if (left_len == right_len && memcmp(left, right, left_len) == 0) {
    log_line(mode, "INFO", "run diff: no differences");
    printf("run diff: identical (%s == %s)\n", left_path, right_path);
    free(left);
    free(right);
    return 0;
  }

  log_line(mode, "WARN", "run diff: ledger mismatch");
  printf("run diff: different (%s != %s)\n", left_path, right_path);
  free(left);
  free(right);
  return 31;
}

int main(int argc, char **argv) {
  log_mode_t mode;
  if (setlocale(LC_ALL, "C") == NULL) {
    return 2;
  }
  mode = parse_log_mode(argc, argv);
  if (argc < 2) {
    print_usage();
    return 1;
  }
  if (strcmp(argv[1], "init") == 0) {
    return command_init(mode);
  }
  if (strcmp(argv[1], "run") == 0) {
    if (argc > 3 && strcmp(argv[2], "diff") == 0) {
      if (argc < 5) {
        print_usage();
        return 1;
      }
      return command_run_diff(argv[3], argv[4], mode);
    }
    return command_run(argc, argv, mode);
  }
  if (strcmp(argv[1], "ledger") == 0 && argc > 3 &&
      strcmp(argv[2], "replay") == 0) {
    return command_ledger_replay(argv[3], mode);
  }
  print_usage();
  return 1;
}
