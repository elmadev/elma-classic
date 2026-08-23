#include "api/curl_wrapper.h"
#include "main.h"
#include <format>

std::string url_encode(const std::string& text) {
    char* encoded_text = curl_easy_escape(nullptr, text.c_str(), 0);
    std::string encoded_string = std::string(encoded_text);
    curl_free(encoded_text);
    return encoded_string;
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

    if (file_h) {
        fclose(file_h);
        file_h = nullptr;
    }

    if (code != CURLE_OK) {
        return error_message(code);
    }
    return std::nullopt;
}

easy_handle::easy_handle(easy_handle* base) {
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
