#ifndef API_CURL_WRAPPER
#define API_CURL_WRAPPER

#include "main.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

#define NOMINMAX
#include <curl/curl.h>

// libcurl strictly follows RFC 3986 or else the url will be rejected
// Percent-encode all characters that are not in the following set: A-Za-z0-9-._~
// For example, ' ' needs to be encoded as "%20"
std::string url_encode(const std::string& text);

// Thin wrapper around libcurl-easy with error-checking and memory management
class easy_handle {
    CURL* handle = nullptr;

    std::string wrapper_error = {};
    std::unique_ptr<char[]> curl_error;

    // Download to file
    std::string file_name = {};
    FILE* file_h = nullptr;

    static std::size_t write_callback_filesystem(char* ptr, size_t size, size_t nmemb,
                                                 void* userdata);

    std::string error_message(CURLcode code);

  public:
    template <typename T> void setopt(CURLoption option, T value) {
        CURLcode code = curl_easy_setopt(handle, option, value);
        if (code != CURLE_OK) {
            internal_error("curl_easy_setopt() failed: " + error_message(code));
        }
    }

    template <typename T> void getinfo(CURLINFO info, T ptr) {
        CURLcode code = curl_easy_getinfo(handle, info, ptr);
        if (code != CURLE_OK) {
            internal_error("curl_easy_getinfo() failed: " + error_message(code));
        }
    }

    // Save target to file. Return an error message if fails
    std::optional<std::string> perform_to_filesystem();

    // Copy the settings of `base`, or create a new default handle if nullptr
    easy_handle(easy_handle* base);
    ~easy_handle();

    // Set file destination target
    void setopt_write_to_filesystem(std::string destination);
};

#endif
