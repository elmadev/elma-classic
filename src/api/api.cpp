#include "api/api.h"
#include "api/curl_wrapper.h"
#include "eol/settings.h"
#include <format>

namespace eol_api {

// Share connection data between all sequential curl-easy handles
// Should not be used with curl-multi
// Multithreaded support is possible but mutex has not yet been implemented
share_interface* share;

easy_handle* sequential_handle;

void init() {
    bool static initialized = false;
    ELMA_ASSERT(!initialized);
    initialized = true;

    CURLcode code = curl_global_init(CURL_GLOBAL_ALL);
    if (code != CURLE_OK) {
        internal_error(curl_easy_strerror(code));
    }

    share = new share_interface();
    share->setopt(CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    share->setopt(CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
    share->setopt(CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);

    sequential_handle = new easy_handle(nullptr);
    ELMA_ASSERT(sequential_handle);
    sequential_handle->setopt(CURLOPT_SHARE, share->get());
    sequential_handle->setopt(CURLOPT_CONNECTTIMEOUT_MS, 5000L);
    sequential_handle->setopt(CURLOPT_TIMEOUT_MS, 60000L);
    sequential_handle->setopt(CURLOPT_MAXFILESIZE, 40L * 1024L * 1024L);
    sequential_handle->setopt(CURLOPT_USERAGENT, "Eol Client/" ELMA_VERSION);
#ifdef CURL_VERBOSE
    sequential_handle->setopt(CURLOPT_VERBOSE, 1L);
#endif
}

void cleanup() {
    delete sequential_handle;
    sequential_handle = nullptr;

    delete share;
    share = nullptr;

    curl_global_cleanup();
}

#ifdef DEBUG
// Avoid incrementing the download counter at https://elma.online/lgrs
#define LGR_GET_DL ""
#else
#define LGR_GET_DL "?dl"
#endif

// https://api.elma.online/api/lgr/get/LGRNAME.lgr?dl
std::optional<std::string> lgr_get(const std::string& lgr_name) {
    easy_handle curl = easy_handle(sequential_handle);
    std::string url = std::format("https://{}/api/lgr/get/{}" LGR_GET_DL, EolSettings->api_name(),
                                  url_encode(lgr_name));
    curl.setopt(CURLOPT_URL, url.c_str());
    curl.setopt_write_to_filesystem(std::format("lgr/{}.lgr", lgr_name));
    return curl.perform_to_filesystem();
}

} // namespace eol_api
