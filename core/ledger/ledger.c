#include "ledger.h"
#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>

enum {
  LEDGER_SHA256_BLOCK_SIZE = 64U,
  LEDGER_SHA256_MESSAGE_WORDS = 64U,
  LEDGER_U64_MAX_DECIMAL_DIGITS = 20U
};

typedef struct ledger_json_writer_t {
  uint8_t *bytes;
  ledger_u64_t capacity;
  /* Tracks required output size even when buffer capacity is insufficient. */
  ledger_u64_t length;
} ledger_json_writer_t;

static uint32_t ledger_sha256_rotr(uint32_t value, uint32_t shift) {
  return (value >> shift) | (value << (32U - shift));
}

static uint32_t ledger_sha256_ch(uint32_t x, uint32_t y, uint32_t z) {
  return (x & y) ^ ((~x) & z);
}

static uint32_t ledger_sha256_maj(uint32_t x, uint32_t y, uint32_t z) {
  return (x & y) ^ (x & z) ^ (y & z);
}

static uint32_t ledger_sha256_big_sigma0(uint32_t x) {
  return ledger_sha256_rotr(x, 2U) ^ ledger_sha256_rotr(x, 13U) ^
         ledger_sha256_rotr(x, 22U);
}

static uint32_t ledger_sha256_big_sigma1(uint32_t x) {
  return ledger_sha256_rotr(x, 6U) ^ ledger_sha256_rotr(x, 11U) ^
         ledger_sha256_rotr(x, 25U);
}

static uint32_t ledger_sha256_small_sigma0(uint32_t x) {
  return ledger_sha256_rotr(x, 7U) ^ ledger_sha256_rotr(x, 18U) ^ (x >> 3U);
}

static uint32_t ledger_sha256_small_sigma1(uint32_t x) {
  return ledger_sha256_rotr(x, 17U) ^ ledger_sha256_rotr(x, 19U) ^ (x >> 10U);
}

static char ledger_sha256_hex_digit(uint8_t nibble) {
  static const char digits[] = "0123456789abcdef";
  return digits[nibble & 0x0FU];
}

static void ledger_json_write_byte(ledger_json_writer_t *writer, uint8_t value) {
  if (writer == NULL) {
    return;
  }
  if (writer->length < writer->capacity && writer->bytes != NULL) {
    writer->bytes[writer->length] = value;
  }
  writer->length += 1U;
}

static void ledger_json_write_bytes(ledger_json_writer_t *writer,
                                    const uint8_t *bytes,
                                    ledger_u64_t length) {
  ledger_u64_t i;
  if (writer == NULL) {
    return;
  }
  if (bytes == NULL) {
    /* Preserve required-length accounting when writing into a null sink. */
    writer->length += length;
    return;
  }
  for (i = 0U; i < length; ++i) {
    ledger_json_write_byte(writer, bytes[i]);
  }
}

static void ledger_json_write_cstr(ledger_json_writer_t *writer,
                                   const char *cstr) {
  const char *it = cstr;
  if (writer == NULL || cstr == NULL) {
    return;
  }
  while (*it != '\0') {
    ledger_json_write_byte(writer, (uint8_t)*it);
    ++it;
  }
}

static void ledger_json_write_escaped_string(ledger_json_writer_t *writer,
                                             ledger_string_t value) {
  ledger_u64_t i;
  ledger_json_write_byte(writer, (uint8_t)'"');
  for (i = 0U; i < value.length; ++i) {
    const uint8_t byte = (uint8_t)value.bytes[i];
    switch (byte) {
      case '"':
        ledger_json_write_cstr(writer, "\\\"");
        break;
      case '\\':
        ledger_json_write_cstr(writer, "\\\\");
        break;
      case '\b':
        ledger_json_write_cstr(writer, "\\b");
        break;
      case '\f':
        ledger_json_write_cstr(writer, "\\f");
        break;
      case '\n':
        ledger_json_write_cstr(writer, "\\n");
        break;
      case '\r':
        ledger_json_write_cstr(writer, "\\r");
        break;
      case '\t':
        ledger_json_write_cstr(writer, "\\t");
        break;
      default:
        if (byte < 0x20U) {
          ledger_json_write_cstr(writer, "\\u00");
          ledger_json_write_byte(writer,
                                 (uint8_t)ledger_sha256_hex_digit(byte >> 4U));
          ledger_json_write_byte(writer,
                                 (uint8_t)ledger_sha256_hex_digit(byte & 0x0FU));
        } else {
          ledger_json_write_byte(writer, byte);
        }
        break;
    }
  }
  ledger_json_write_byte(writer, (uint8_t)'"');
}

static void ledger_json_write_u64(ledger_json_writer_t *writer, ledger_u64_t value) {
  uint8_t digits[LEDGER_U64_MAX_DECIMAL_DIGITS];
  ledger_u64_t digit_count = 0U;
  ledger_u64_t i;

  do {
    digits[digit_count] = (uint8_t)('0' + (value % 10U));
    value /= 10U;
    digit_count += 1U;
  } while (value != 0U);

  for (i = digit_count; i > 0U; --i) {
    ledger_json_write_byte(writer, digits[i - 1U]);
  }
}

static void ledger_json_write_null(ledger_json_writer_t *writer) {
  ledger_json_write_cstr(writer, "null");
}

static void ledger_json_write_key(ledger_json_writer_t *writer, const char *key) {
  ledger_json_write_byte(writer, (uint8_t)'"');
  ledger_json_write_cstr(writer, key);
  ledger_json_write_cstr(writer, "\":");
}

static void ledger_json_write_optional_u64(ledger_json_writer_t *writer,
                                           ledger_optional_u64_t value) {
  if (value.has_value) {
    ledger_json_write_u64(writer, value.value);
  } else {
    ledger_json_write_null(writer);
  }
}

static void ledger_json_write_optional_string(ledger_json_writer_t *writer,
                                              ledger_optional_string_t value) {
  if (value.has_value) {
    ledger_json_write_escaped_string(writer, value.value);
  } else {
    ledger_json_write_null(writer);
  }
}

static void ledger_event_serialize_json_into(ledger_json_writer_t *writer,
                                             const ledger_event_t *envelope) {
  /*
   * docs/LEDGER.md section 8.1:
   * * keys in exact section 6 schema order
   * * no extra whitespace
   * * explicit nulls
   * * no trailing commas
   */
  ledger_json_write_byte(writer, (uint8_t)'{');

  ledger_json_write_key(writer, "event_type");
  ledger_json_write_escaped_string(writer, envelope->event_type);
  ledger_json_write_byte(writer, (uint8_t)',');

  ledger_json_write_key(writer, "run_id");
  ledger_json_write_escaped_string(writer, envelope->run_id);
  ledger_json_write_byte(writer, (uint8_t)',');

  ledger_json_write_key(writer, "attempt");
  ledger_json_write_optional_u64(writer, envelope->attempt);
  ledger_json_write_byte(writer, (uint8_t)',');

  ledger_json_write_key(writer, "artifact_id");
  ledger_json_write_optional_string(writer, envelope->artifact_id);
  ledger_json_write_byte(writer, (uint8_t)',');

  ledger_json_write_key(writer, "contract_hash");
  ledger_json_write_optional_string(writer, envelope->contract_hash);
  ledger_json_write_byte(writer, (uint8_t)',');

  ledger_json_write_key(writer, "global_ceilings_hash");
  ledger_json_write_optional_string(writer, envelope->global_ceilings_hash);
  ledger_json_write_byte(writer, (uint8_t)',');

  ledger_json_write_key(writer, "exemption_manifest_hash");
  ledger_json_write_optional_string(writer, envelope->exemption_manifest_hash);
  ledger_json_write_byte(writer, (uint8_t)',');

  ledger_json_write_key(writer, "toolchain_hash");
  ledger_json_write_optional_string(writer, envelope->toolchain_hash);
  ledger_json_write_byte(writer, (uint8_t)',');

  ledger_json_write_key(writer, "payload");
  /*
   * Payload is opaque to ledger serialization and is emitted verbatim.
   * Callers provide pre-serialized JSON bytes for the payload value.
   */
  ledger_json_write_bytes(writer, envelope->payload.opaque_payload,
                          envelope->payload.length);

  ledger_json_write_byte(writer, (uint8_t)'}');
}

static void ledger_sha256_digest_to_hex(const ledger_sha256_digest_t *digest,
                                        char out_hex[LEDGER_SHA256_HEX_STORAGE_LENGTH]) {
  ledger_u64_t i;

  for (i = 0U; i < LEDGER_SHA256_DIGEST_SIZE; ++i) {
    const uint8_t byte = digest->bytes[i];
    out_hex[i * 2U] = ledger_sha256_hex_digit((uint8_t)(byte >> 4U));
    out_hex[(i * 2U) + 1U] = ledger_sha256_hex_digit(byte);
  }

  out_hex[LEDGER_SHA256_HEX_LENGTH] = '\0';
}

static void ledger_sha256_process_block(uint32_t state[8],
                                        const uint8_t block[64]) {
  uint32_t schedule[LEDGER_SHA256_MESSAGE_WORDS];
  uint32_t a;
  uint32_t b;
  uint32_t c;
  uint32_t d;
  uint32_t e;
  uint32_t f;
  uint32_t g;
  uint32_t h;
  uint32_t i;
  const uint32_t constants[LEDGER_SHA256_MESSAGE_WORDS] = {
      0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, 0x3956c25bU,
      0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U, 0xd807aa98U, 0x12835b01U,
      0x243185beU, 0x550c7dc3U, 0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U,
      0xc19bf174U, 0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
      0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU, 0x983e5152U,
      0xa831c66dU, 0xb00327c8U, 0xbf597fc7U, 0xc6e00bf3U, 0xd5a79147U,
      0x06ca6351U, 0x14292967U, 0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU,
      0x53380d13U, 0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
      0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U, 0xd192e819U,
      0xd6990624U, 0xf40e3585U, 0x106aa070U, 0x19a4c116U, 0x1e376c08U,
      0x2748774cU, 0x34b0bcb5U, 0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU,
      0x682e6ff3U, 0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
      0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U};

  for (i = 0U; i < 16U; ++i) {
    const uint32_t offset = i * 4U;
    schedule[i] = ((uint32_t)block[offset] << 24U) |
                  ((uint32_t)block[offset + 1U] << 16U) |
                  ((uint32_t)block[offset + 2U] << 8U) |
                  (uint32_t)block[offset + 3U];
  }

  for (i = 16U; i < LEDGER_SHA256_MESSAGE_WORDS; ++i) {
    schedule[i] = ledger_sha256_small_sigma1(schedule[i - 2U]) +
                  schedule[i - 7U] + ledger_sha256_small_sigma0(schedule[i - 15U]) +
                  schedule[i - 16U];
  }

  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];
  f = state[5];
  g = state[6];
  h = state[7];

  for (i = 0U; i < LEDGER_SHA256_MESSAGE_WORDS; ++i) {
    const uint32_t t1 = h + ledger_sha256_big_sigma1(e) + ledger_sha256_ch(e, f, g) +
                        constants[i] + schedule[i];
    const uint32_t t2 = ledger_sha256_big_sigma0(a) + ledger_sha256_maj(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + t1;
    d = c;
    c = b;
    b = a;
    a = t1 + t2;
  }

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
  state[5] += f;
  state[6] += g;
  state[7] += h;
}

void ledger_sha256_digest(const uint8_t *input_bytes,
                          ledger_u64_t input_length,
                          ledger_sha256_digest_t *out_digest) {
  uint32_t state[8] = {0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
                       0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U};
  uint8_t block[LEDGER_SHA256_BLOCK_SIZE];
  ledger_u64_t block_count = input_length / LEDGER_SHA256_BLOCK_SIZE;
  ledger_u64_t remaining = input_length % LEDGER_SHA256_BLOCK_SIZE;
  ledger_u64_t i;
  uint64_t bit_length = (uint64_t)(input_length * 8U);
  uint32_t output_index;

  for (i = 0U; i < block_count; ++i) {
    ledger_sha256_process_block(state, input_bytes + (i * LEDGER_SHA256_BLOCK_SIZE));
  }

  for (i = 0U; i < remaining; ++i) {
    block[i] = input_bytes[(block_count * LEDGER_SHA256_BLOCK_SIZE) + i];
  }

  block[remaining] = 0x80U;
  for (i = remaining + 1U; i < LEDGER_SHA256_BLOCK_SIZE; ++i) {
    block[i] = 0U;
  }

  if (remaining >= 56U) {
    ledger_sha256_process_block(state, block);
    for (i = 0U; i < LEDGER_SHA256_BLOCK_SIZE; ++i) {
      block[i] = 0U;
    }
  }

  block[56] = (uint8_t)(bit_length >> 56U);
  block[57] = (uint8_t)(bit_length >> 48U);
  block[58] = (uint8_t)(bit_length >> 40U);
  block[59] = (uint8_t)(bit_length >> 32U);
  block[60] = (uint8_t)(bit_length >> 24U);
  block[61] = (uint8_t)(bit_length >> 16U);
  block[62] = (uint8_t)(bit_length >> 8U);
  block[63] = (uint8_t)bit_length;
  ledger_sha256_process_block(state, block);

  for (output_index = 0U; output_index < 8U; ++output_index) {
    out_digest->bytes[(output_index * 4U)] = (uint8_t)(state[output_index] >> 24U);
    out_digest->bytes[(output_index * 4U) + 1U] =
        (uint8_t)(state[output_index] >> 16U);
    out_digest->bytes[(output_index * 4U) + 2U] =
        (uint8_t)(state[output_index] >> 8U);
    out_digest->bytes[(output_index * 4U) + 3U] = (uint8_t)state[output_index];
  }
}

void ledger_event_populate_envelope_hashes(
    ledger_event_t *event,
    const ledger_event_hash_inputs_t *hash_inputs,
    ledger_event_hash_storage_t *hash_storage) {
  ledger_sha256_digest_t contract_digest;
  ledger_sha256_digest_t global_ceilings_digest;
  ledger_sha256_digest_t exemption_manifest_digest;
  ledger_sha256_digest_t toolchain_digest;

  /* docs/LEDGER.md section 6.1: hash exact normalized JSON UTF-8 bytes. */
  ledger_sha256_digest(hash_inputs->contract.normalized_bytes.bytes,
                       hash_inputs->contract.normalized_bytes.length,
                       &contract_digest);
  ledger_sha256_digest(hash_inputs->global_ceilings.normalized_bytes.bytes,
                       hash_inputs->global_ceilings.normalized_bytes.length,
                       &global_ceilings_digest);
  ledger_sha256_digest(hash_inputs->exemption_manifest.normalized_bytes.bytes,
                       hash_inputs->exemption_manifest.normalized_bytes.length,
                       &exemption_manifest_digest);
  ledger_sha256_digest(hash_inputs->toolchain.normalized_bytes.bytes,
                       hash_inputs->toolchain.normalized_bytes.length,
                       &toolchain_digest);

  /* docs/LEDGER.md section 6.1: lowercase hex output, exactly 64 chars. */
  ledger_sha256_digest_to_hex(&contract_digest, hash_storage->contract_hash);
  ledger_sha256_digest_to_hex(&global_ceilings_digest,
                              hash_storage->global_ceilings_hash);
  ledger_sha256_digest_to_hex(&exemption_manifest_digest,
                              hash_storage->exemption_manifest_hash);
  ledger_sha256_digest_to_hex(&toolchain_digest, hash_storage->toolchain_hash);

  /* docs/LEDGER.md section 6: hash fields are explicit envelope members. */
  event->contract_hash.has_value = true;
  event->contract_hash.value.bytes = hash_storage->contract_hash;
  event->contract_hash.value.length = LEDGER_SHA256_HEX_LENGTH;

  event->global_ceilings_hash.has_value = true;
  event->global_ceilings_hash.value.bytes = hash_storage->global_ceilings_hash;
  event->global_ceilings_hash.value.length = LEDGER_SHA256_HEX_LENGTH;

  event->exemption_manifest_hash.has_value = true;
  event->exemption_manifest_hash.value.bytes =
      hash_storage->exemption_manifest_hash;
  event->exemption_manifest_hash.value.length = LEDGER_SHA256_HEX_LENGTH;

  event->toolchain_hash.has_value = true;
  event->toolchain_hash.value.bytes = hash_storage->toolchain_hash;
  event->toolchain_hash.value.length = LEDGER_SHA256_HEX_LENGTH;
}

void ledger_event_construct_envelope(
    ledger_event_t *out_event,
    const ledger_event_envelope_inputs_t *inputs) {
  /*
   * docs/LEDGER.md section 6:
   * * Fields MUST appear in the exact order listed above.
   * * Nulls MUST be explicit.
   */
  out_event->event_type = inputs->event_type;
  out_event->run_id = inputs->run_id;
  out_event->attempt = inputs->attempt;
  out_event->artifact_id = inputs->artifact_id;
  out_event->contract_hash = inputs->contract_hash;
  out_event->global_ceilings_hash = inputs->global_ceilings_hash;
  out_event->exemption_manifest_hash = inputs->exemption_manifest_hash;
  out_event->toolchain_hash = inputs->toolchain_hash;
  out_event->payload = inputs->payload;
}

void ledger_event_serialize_json(const ledger_event_t *envelope,
                                 uint8_t *out_bytes,
                                 ledger_u64_t *in_out_length) {
  ledger_json_writer_t writer;

  if (envelope == NULL || in_out_length == NULL) {
    return;
  }

  writer.bytes = out_bytes;
  writer.capacity = *in_out_length;
  writer.length = 0U;

  ledger_event_serialize_json_into(&writer, envelope);

  *in_out_length = writer.length;
}

ledger_error_code_t ledger_append_bytes(int fd,
                                        const uint8_t *bytes,
                                        ledger_u64_t length) {
  ledger_u64_t written = 0U;
  const ledger_u64_t max_write_size = (ledger_u64_t)SIZE_MAX;

  /*
   * docs/LEDGER.md section 9:
   * return only ledger-defined error identifiers.
   */
  if (fd < 0 || bytes == NULL || length == 0U) {
    return LEDGER_E_SERIALIZATION;
  }

  /*
   * docs/LEDGER.md section 2 + 8.2 + 10:
   * append-only + serial single-writer model.
   * Seek to EOF before writing so this call only appends bytes.
   */
  if (lseek(fd, 0, SEEK_END) == (off_t)-1) {
    return LEDGER_E_APPEND_FAILURE;
  }

  while (written < length) {
    const ledger_u64_t remaining = length - written;
    const size_t write_size =
        (size_t)((remaining > max_write_size) ? max_write_size : remaining);
    const ssize_t rc = write(fd, bytes + written, write_size);

    if (rc > 0) {
      written += (ledger_u64_t)rc;
      continue;
    }

    if (rc == -1) {
      const int saved_errno = errno;
      if (saved_errno == EINTR) {
        continue;
      }
    }
    return LEDGER_E_APPEND_FAILURE;
  }

  return (ledger_error_code_t)0;
}
