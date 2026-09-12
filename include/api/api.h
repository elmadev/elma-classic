#ifndef API_API
#define API_API

#include <optional>
#include <string>

namespace eol_api {

void init();
void cleanup();

// Download an lgr to your lgr/ folder
// Returns an error string if not successful
std::optional<std::string> lgr_get(const std::string& lgr_name);

} // namespace eol_api

#endif
