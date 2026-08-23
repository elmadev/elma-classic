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

std::string share_interface::error_message(CURLSHcode code) {
    return std::string(curl_share_strerror(code));
}

share_interface::share_interface() {
    share = curl_share_init();
    ELMA_ASSERT(share);
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
