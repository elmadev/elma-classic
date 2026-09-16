#ifndef API_API
#define API_API

#include "api/types.h"
#include <optional>
#include <string>

namespace eol_api {

void init();
void cleanup();

// Download an LGR to your lgr/ folder.
// Returns an error string if not successful.
std::optional<std::string> lgr_get(const std::string& lgr_name);

// Returns an error string if not successful
std::pair<api_type::lgr_info, std::string> lgr_info();

} // namespace eol_api

#endif
