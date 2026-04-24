#ifndef HARNESS_CONFIG_H
#define HARNESS_CONFIG_H

#include <stddef.h>

#define CONFIG_PATH_MAX_LEN 512

typedef struct {
  char contract_path[CONFIG_PATH_MAX_LEN];
  char ceilings_path[CONFIG_PATH_MAX_LEN];
  char exemptions_path[CONFIG_PATH_MAX_LEN];
  char artifact_dir[CONFIG_PATH_MAX_LEN];
  char runs_dir[CONFIG_PATH_MAX_LEN];
  int max_normalization_ms;
  int max_static_analysis_ms;
  int max_freeze_ms;
  int max_generator_validation_ms;
  unsigned long long max_bytes_normalization;
  unsigned long long max_bytes_static_analysis;
  unsigned long long max_bytes_freeze;
  int max_open_fds;
  int logging_mode_json;
} harness_config_t;

int config_load(harness_config_t *out_cfg, const char *local_config_path,
                const char *defaults_path);

#endif
