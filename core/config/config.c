#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, char **out_text) {
  FILE *fp;
  long n;
  size_t read_count;
  char *buf;
  if (path == NULL || out_text == NULL) {
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
  buf = (char *)malloc((size_t)n + 1U);
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
  buf[n] = '\0';
  *out_text = buf;
  return 0;
}

static const char *find_key_value_start(const char *doc, const char *key) {
  char pattern[128];
  const char *p;
  size_t i;
  if (doc == NULL || key == NULL || strlen(key) + 3U >= sizeof(pattern)) {
    return NULL;
  }
  pattern[0] = '"';
  strcpy(&pattern[1], key);
  i = strlen(key) + 1U;
  pattern[i++] = '"';
  pattern[i] = '\0';

  p = strstr(doc, pattern);
  if (p == NULL) {
    return NULL;
  }
  p = strchr(p + (long)strlen(pattern), ':');
  if (p == NULL) {
    return NULL;
  }
  ++p;
  while (*p != '\0' && isspace((unsigned char)*p) != 0) {
    ++p;
  }
  return p;
}

static int parse_json_string_value(const char *start, char *out,
                                   size_t out_size) {
  size_t i = 0U;
  const char *p = start;
  if (p == NULL || out == NULL || out_size == 0U || *p != '"') {
    return 1;
  }
  ++p;
  while (*p != '\0' && *p != '"') {
    if ((unsigned char)*p < 0x20U || i + 1U >= out_size) {
      return 1;
    }
    if (*p == '\\') {
      return 1;
    }
    out[i++] = *p++;
  }
  if (*p != '"') {
    return 1;
  }
  out[i] = '\0';
  return 0;
}

static int parse_json_int_value(const char *start, long long *out_value) {
  char *end_ptr;
  long long value;
  const char *p = start;
  if (p == NULL || out_value == NULL || *p == '"') {
    return 1;
  }
  value = strtoll(p, &end_ptr, 10);
  if (end_ptr == p) {
    return 1;
  }
  while (*end_ptr != '\0' && isspace((unsigned char)*end_ptr) != 0) {
    ++end_ptr;
  }
  if (*end_ptr != ',' && *end_ptr != '}' && *end_ptr != ']') {
    return 1;
  }
  *out_value = value;
  return 0;
}

static int extract_required_string(const char *doc, const char *key, char *out,
                                   size_t out_size) {
  const char *start = find_key_value_start(doc, key);
  if (start == NULL) {
    return 1;
  }
  return parse_json_string_value(start, out, out_size);
}

static int extract_optional_string(const char *doc, const char *key, char *out,
                                   size_t out_size, int *has_value) {
  const char *start = find_key_value_start(doc, key);
  *has_value = 0;
  if (start == NULL) {
    return 0;
  }
  if (parse_json_string_value(start, out, out_size) != 0) {
    return 1;
  }
  *has_value = 1;
  return 0;
}

static int extract_required_int(const char *doc, const char *key, int *out) {
  long long value;
  const char *start = find_key_value_start(doc, key);
  if (start == NULL || parse_json_int_value(start, &value) != 0 || value < 1LL) {
    return 1;
  }
  *out = (int)value;
  return 0;
}

static int extract_optional_int(const char *doc, const char *key, int *out,
                                int *has_value) {
  long long value;
  const char *start = find_key_value_start(doc, key);
  *has_value = 0;
  if (start == NULL) {
    return 0;
  }
  if (parse_json_int_value(start, &value) != 0 || value < 1LL) {
    return 1;
  }
  *out = (int)value;
  *has_value = 1;
  return 0;
}

static int extract_required_u64(const char *doc, const char *key,
                                unsigned long long *out) {
  long long value;
  const char *start = find_key_value_start(doc, key);
  if (start == NULL || parse_json_int_value(start, &value) != 0 || value < 1LL) {
    return 1;
  }
  *out = (unsigned long long)value;
  return 0;
}

static int extract_optional_u64(const char *doc, const char *key,
                                unsigned long long *out, int *has_value) {
  long long value;
  const char *start = find_key_value_start(doc, key);
  *has_value = 0;
  if (start == NULL) {
    return 0;
  }
  if (parse_json_int_value(start, &value) != 0 || value < 1LL) {
    return 1;
  }
  *out = (unsigned long long)value;
  *has_value = 1;
  return 0;
}

static int parse_logging_mode(const char *doc, int *out_json_mode) {
  char mode[16];
  if (extract_required_string(doc, "mode", mode, sizeof(mode)) != 0) {
    return 1;
  }
  if (strcmp(mode, "text") == 0) {
    *out_json_mode = 0;
    return 0;
  }
  if (strcmp(mode, "json") == 0) {
    *out_json_mode = 1;
    return 0;
  }
  return 1;
}

static int parse_required_config(const char *doc, harness_config_t *cfg) {
  if (extract_required_string(doc, "contract", cfg->contract_path,
                              sizeof(cfg->contract_path)) != 0 ||
      extract_required_string(doc, "ceilings", cfg->ceilings_path,
                              sizeof(cfg->ceilings_path)) != 0 ||
      extract_required_string(doc, "exemptions", cfg->exemptions_path,
                              sizeof(cfg->exemptions_path)) != 0 ||
      extract_required_string(doc, "artifact_dir", cfg->artifact_dir,
                              sizeof(cfg->artifact_dir)) != 0 ||
      extract_required_string(doc, "runs_dir", cfg->runs_dir,
                              sizeof(cfg->runs_dir)) != 0 ||
      extract_required_int(doc, "max_normalization_ms",
                           &cfg->max_normalization_ms) != 0 ||
      extract_required_int(doc, "max_static_analysis_ms",
                           &cfg->max_static_analysis_ms) != 0 ||
      extract_required_int(doc, "max_freeze_ms", &cfg->max_freeze_ms) != 0 ||
      extract_required_int(doc, "max_generator_validation_ms",
                           &cfg->max_generator_validation_ms) != 0 ||
      extract_required_u64(doc, "max_bytes_normalization",
                           &cfg->max_bytes_normalization) != 0 ||
      extract_required_u64(doc, "max_bytes_static_analysis",
                           &cfg->max_bytes_static_analysis) != 0 ||
      extract_required_u64(doc, "max_bytes_freeze", &cfg->max_bytes_freeze) != 0 ||
      extract_required_int(doc, "max_open_fds", &cfg->max_open_fds) != 0 ||
      parse_logging_mode(doc, &cfg->logging_mode_json) != 0) {
    return 1;
  }
  return 0;
}

static int overlay_optional_config(const char *doc, harness_config_t *cfg) {
  int has_value;
  if (extract_optional_string(doc, "contract", cfg->contract_path,
                              sizeof(cfg->contract_path), &has_value) != 0 ||
      extract_optional_string(doc, "ceilings", cfg->ceilings_path,
                              sizeof(cfg->ceilings_path), &has_value) != 0 ||
      extract_optional_string(doc, "exemptions", cfg->exemptions_path,
                              sizeof(cfg->exemptions_path), &has_value) != 0 ||
      extract_optional_string(doc, "artifact_dir", cfg->artifact_dir,
                              sizeof(cfg->artifact_dir), &has_value) != 0 ||
      extract_optional_string(doc, "runs_dir", cfg->runs_dir,
                              sizeof(cfg->runs_dir), &has_value) != 0 ||
      extract_optional_int(doc, "max_normalization_ms",
                           &cfg->max_normalization_ms, &has_value) != 0 ||
      extract_optional_int(doc, "max_static_analysis_ms",
                           &cfg->max_static_analysis_ms, &has_value) != 0 ||
      extract_optional_int(doc, "max_freeze_ms", &cfg->max_freeze_ms,
                           &has_value) != 0 ||
      extract_optional_int(doc, "max_generator_validation_ms",
                           &cfg->max_generator_validation_ms, &has_value) != 0 ||
      extract_optional_u64(doc, "max_bytes_normalization",
                           &cfg->max_bytes_normalization, &has_value) != 0 ||
      extract_optional_u64(doc, "max_bytes_static_analysis",
                           &cfg->max_bytes_static_analysis, &has_value) != 0 ||
      extract_optional_u64(doc, "max_bytes_freeze", &cfg->max_bytes_freeze,
                           &has_value) != 0 ||
      extract_optional_int(doc, "max_open_fds", &cfg->max_open_fds,
                           &has_value) != 0) {
    return 1;
  }

  if (find_key_value_start(doc, "mode") != NULL &&
      parse_logging_mode(doc, &cfg->logging_mode_json) != 0) {
    return 1;
  }

  return 0;
}

int config_load(harness_config_t *out_cfg, const char *local_config_path,
                const char *defaults_path) {
  char *defaults = NULL;
  char *local = NULL;
  if (out_cfg == NULL || defaults_path == NULL) {
    return 1;
  }
  memset(out_cfg, 0, sizeof(*out_cfg));

  if (read_file(defaults_path, &defaults) != 0) {
    return 1;
  }

  if (parse_required_config(defaults, out_cfg) != 0) {
    free(defaults);
    return 1;
  }

  if (local_config_path != NULL) {
    if (read_file(local_config_path, &local) != 0) {
      free(defaults);
      return 1;
    }
    if (overlay_optional_config(local, out_cfg) != 0) {
      free(defaults);
      free(local);
      return 1;
    }
    free(local);
  }

  free(defaults);
  return 0;
}
