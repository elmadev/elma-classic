#include "api/curl_wrapper.h"
#include "log.h"
#include "main.h"
#include <filesystem>
#include <format>

std::string url_encode(const std::string& text) {
    char* encoded_text = curl_easy_escape(nullptr, text.c_str(), 0);
    ELMA_ASSERT(encoded_text);
    std::string encoded_string = std::string(encoded_text);
    curl_free(encoded_text);
    return encoded_string;
}

void share_interface::lock_callback_function(CURL* /*handle*/, curl_lock_data data,
                                             curl_lock_access access, void* clientp) {
    share_interface* this_ = (share_interface*)clientp;

    ELMA_ASSERT(access == CURL_LOCK_ACCESS_SINGLE);

    switch (data) {
    case CURL_LOCK_DATA_SHARE:
        this_->locks.share.lock();
        break;
    case CURL_LOCK_DATA_COOKIE:
        this_->locks.cookie.lock();
        break;
    case CURL_LOCK_DATA_DNS:
        this_->locks.dns.lock();
        break;
    case CURL_LOCK_DATA_SSL_SESSION:
        this_->locks.ssl.lock();
        break;
    case CURL_LOCK_DATA_CONNECT:
        this_->locks.connect.lock();
        break;
    default:
        internal_error(std::format("Unsupported lock data type: {}", (int)data));
    }
}

void share_interface::unlock_callback_function(CURL* /*handle*/, curl_lock_data data,
                                               void* clientp) {
    share_interface* this_ = (share_interface*)clientp;

    switch (data) {
    case CURL_LOCK_DATA_SHARE:
        this_->locks.share.unlock();
        break;
    case CURL_LOCK_DATA_COOKIE:
        this_->locks.cookie.unlock();
        break;
    case CURL_LOCK_DATA_DNS:
        this_->locks.dns.unlock();
        break;
    case CURL_LOCK_DATA_SSL_SESSION:
        this_->locks.ssl.unlock();
        break;
    case CURL_LOCK_DATA_CONNECT:
        this_->locks.connect.unlock();
        break;
    default:
        internal_error(std::format("Unsupported unlock data type: {}", (int)data));
    }
}

std::string share_interface::error_message(CURLSHcode code) {
    return std::string(curl_share_strerror(code));
}

share_interface::share_interface() {
    share = curl_share_init();
    ELMA_ASSERT(share);

    // Pass a reference to self in lock callback functions
    setopt(CURLSHOPT_USERDATA, this);

    // Multithreaded support
    setopt(CURLSHOPT_LOCKFUNC, lock_callback_function);
    setopt(CURLSHOPT_UNLOCKFUNC, unlock_callback_function);
}

share_interface::~share_interface() {
    CURLSHcode code = curl_share_cleanup(share);
    if (code != CURLSHE_OK) {
        LOG_WARN("Failed to release curl-share resources: {}", error_message(code));
    }
    share = nullptr;
}

std::size_t easy_handle::write_callback_filesystem(char* ptr, size_t size, size_t nmemb,
                                                   void* userdata) {
    easy_handle* this_ = (easy_handle*)userdata;

    // Handle first call to function
    if (!this_->file_h) {
        // Verify that we are getting a 200 response before storing the received data
        long response_code = -1;
        this_->getinfo(CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code != 200) {
            this_->wrapper_error =
                std::format("Failed to download file: status code {}", response_code);
            return CURL_WRITEFUNC_ERROR;
        }

        // Open the file for writing
        this_->file_h = fopen(this_->file_name.c_str(), "wb");
        if (!this_->file_h) {
            this_->wrapper_error =
                std::format("Failed to download file: could not open file: {}", this_->file_name);
            return CURL_WRITEFUNC_ERROR;
        }
    }

    return fwrite(ptr, size, nmemb, this_->file_h);
}

std::size_t easy_handle::write_callback_buffer(char* ptr, size_t size, size_t nmemb,
                                               void* userdata) {
    easy_handle* this_ = (easy_handle*)userdata;

    // Handle first call to function
    if (!this_->data_first_callback) {
        this_->data_first_callback = true;

        long response_code = -1;
        this_->getinfo(CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code != 200) {
            this_->wrapper_error =
                std::format("Failed to download file: status code {}", response_code);
            return CURL_WRITEFUNC_ERROR;
        }

        curl_off_t content_length = -1;
        this_->getinfo(CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &content_length);
        if (content_length == -1) {
            // Missing/unknown Content-Length header returns a value of -1
            this_->data_buffer.reserve(1024);
        } else {
            this_->data_buffer.reserve(content_length);
        }
    }

    this_->data_buffer.insert(this_->data_buffer.end(), ptr, ptr + size * nmemb);
    return nmemb;
}

std::string easy_handle::error_message(CURLcode code) {
    // Error message from this file
    if (!wrapper_error.empty()) {
        return wrapper_error;
    }
    // Error message from libcurl (may not modify this buffer)
    if (curl_error[0]) {
        return std::string(curl_error.get());
    }
    // Error code from libcurl (curl_error may remain empty despite error occurring)
    return std::string(curl_easy_strerror(code));
}

std::optional<std::string> easy_handle::perform_to_filesystem() {
    CURLcode code = curl_easy_perform(handle);
    bool failed = code != CURLE_OK;

    if (file_h) {
        fclose(file_h);
        file_h = nullptr;

        if (failed && !file_name.empty()) {
            // Try to delete the file, but ignore any errors
            std::filesystem::remove(file_name);
        }
    }

    if (failed) {
        return error_message(code);
    }

    return std::nullopt;
}

std::pair<std::vector<unsigned char>, std::string> easy_handle::perform_to_buffer() {
    CURLcode code = curl_easy_perform(handle);

    if (code != CURLE_OK) {
        data_buffer.clear();
        data_first_callback = false;
        return {std::vector<unsigned char>(), error_message(code)};
    }
    std::vector<unsigned char> ret = std::move(data_buffer);
    data_buffer.clear();
    data_first_callback = false;
    return {std::move(ret), ""};
}

easy_handle::easy_handle(easy_handle* base, share_interface* share) {
    if (base) {
        handle = curl_easy_duphandle(base->handle);
    } else {
        handle = curl_easy_init();
    }
    ELMA_ASSERT(handle);

    curl_error = std::make_unique<char[]>(CURL_ERROR_SIZE);
    setopt(CURLOPT_ERRORBUFFER, curl_error.get());

    // Pass a reference to self in write callback function
    setopt(CURLOPT_WRITEDATA, this);

    if (share) {
        // CURLOPT_SHARE is not copied when using curl_easy_duphandle(), so set it manually
        setopt(CURLOPT_SHARE, share->share);
    }
}

easy_handle::~easy_handle() {
    curl_easy_cleanup(handle);
    handle = nullptr;

    if (file_h) {
        fclose(file_h);
        file_h = nullptr;
    }
}

void easy_handle::setopt_write_to_filesystem(std::string destination) {
    file_name = std::move(destination);
    setopt(CURLOPT_WRITEFUNCTION, write_callback_filesystem);
}

void easy_handle::setopt_write_to_buffer() { setopt(CURLOPT_WRITEFUNCTION, write_callback_buffer); }
