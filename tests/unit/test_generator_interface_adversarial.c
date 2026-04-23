#include "../../core/generator_interface/generator_interface.h"

#include <stdio.h>

int main(void) {
  const char *request_id = "abcd1234efef5678";
  const char *wrong_id = "ffff1234efef5678";
  const char artifact_json[] = "{}";
  const char err_code[] = "GEN_E_PROTOCOL";

  gi_generator_response_t r = {{request_id, 16}, GI_GENERATOR_STATUS_SUCCESS,
                               {true, {{(const uint8_t *)artifact_json, 2}}},
                               {false, {NULL, 0}},
                               {false, {{NULL, 0}}}};

  if (gi_validate_response_for_request(&r, (gi_string_t){request_id, 16}) != 0) {
    fprintf(stderr, "expected valid success response\n");
    return 1;
  }

  if (gi_validate_response_for_request(&r, (gi_string_t){wrong_id, 16}) !=
      GEN_E_PROTOCOL) {
    fprintf(stderr, "expected wrong request_id rejection\n");
    return 1;
  }

  r.status = GI_GENERATOR_STATUS_FAILURE;
  r.candidate_artifact.has_value = true;
  r.error_code = (gi_optional_string_t){true, {err_code, 14}};
  if (gi_validate_response(&r) != GEN_E_PROTOCOL) {
    fprintf(stderr, "expected failure candidate payload rejection\n");
    return 1;
  }

  r.status = GI_GENERATOR_STATUS_SUCCESS;
  r.candidate_artifact.has_value = false;
  r.error_code = (gi_optional_string_t){false, {NULL, 0}};
  if (gi_validate_response(&r) != GEN_E_PROTOCOL) {
    fprintf(stderr, "expected missing artifact rejection\n");
    return 1;
  }

  return 0;
}
