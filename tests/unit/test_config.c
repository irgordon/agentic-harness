#include "../../core/config/config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int write_text(const char *path, const char *text) {
  FILE *fp = fopen(path, "wb");
  size_t n = strlen(text);
  if (fp == NULL) {
    return 1;
  }
  if (fwrite(text, 1U, n, fp) != n) {
    fclose(fp);
    return 1;
  }
  fclose(fp);
  return 0;
}

int main(void) {
  harness_config_t cfg;
  if (config_load(&cfg, NULL, "spec/v1/config_defaults.json") != 0) {
    fprintf(stderr, "expected defaults config load success\n");
    return 1;
  }
  if (strcmp(cfg.contract_path, "fixtures/contracts/basic.json") != 0) {
    fprintf(stderr, "unexpected defaults contract path\n");
    return 1;
  }

  if (write_text("/tmp/harness_test_cfg.json",
                 "{\"logging\":{\"mode\":\"json\"},\"paths\":{\"contract\":\"fixtures/contracts/basic.json\"}}") != 0) {
    fprintf(stderr, "failed to write override config\n");
    return 1;
  }
  if (config_load(&cfg, "/tmp/harness_test_cfg.json", "spec/v1/config_defaults.json") != 0) {
    fprintf(stderr, "expected override config load success\n");
    return 1;
  }
  if (cfg.logging_mode_json != 1) {
    fprintf(stderr, "expected json logging mode from override\n");
    return 1;
  }

  if (write_text("/tmp/harness_test_cfg_bad.json",
                 "{\"limits\":{\"performance\":{\"max_freeze_ms\":\"oops\"}}}") != 0) {
    fprintf(stderr, "failed to write bad config\n");
    return 1;
  }
  if (config_load(&cfg, "/tmp/harness_test_cfg_bad.json", "spec/v1/config_defaults.json") == 0) {
    fprintf(stderr, "expected bad config load failure\n");
    return 1;
  }

  return 0;
}
