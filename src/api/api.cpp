#include "api/api.h"
#include "api/curl_wrapper.h"

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
}

void cleanup() {
    delete sequential_handle;
    sequential_handle = nullptr;

    delete share;
    share = nullptr;

    curl_global_cleanup();
}

} // namespace eol_api
