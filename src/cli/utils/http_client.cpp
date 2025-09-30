#include "http_client.hpp"
#include <curl/curl.h>
#include <fstream>
#include <spdlog/spdlog.h>

namespace tsuki::cli {

HttpClient::HttpClient() : response_code_(0), curl_handle_(nullptr) {
    curl_handle_ = curl_easy_init();
    if (!curl_handle_) {
        last_error_ = "Failed to initialize libcurl";
        spdlog::error("Failed to initialize libcurl");
    }
}

HttpClient::~HttpClient() {
    if (curl_handle_) {
        curl_easy_cleanup(static_cast<CURL*>(curl_handle_));
    }
}

size_t HttpClient::writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    auto* stream = static_cast<std::ofstream*>(userp);
    stream->write(static_cast<char*>(contents), total_size);
    return total_size;
}

int HttpClient::progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                                 curl_off_t ultotal, curl_off_t ulnow) {
    (void)ultotal;
    (void)ulnow;

    if (dltotal > 0) {
        auto* cb = static_cast<ProgressCallback*>(clientp);
        if (cb && *cb) {
            int percentage = static_cast<int>((dlnow * 100) / dltotal);
            (*cb)(dlnow, dltotal, percentage);
        }
    }
    return 0;
}

bool HttpClient::downloadFile(const std::string& url, const std::string& output_path,
                              ProgressCallback progress_cb) {
    if (!curl_handle_) {
        last_error_ = "CURL not initialized";
        return false;
    }

    CURL* curl = static_cast<CURL*>(curl_handle_);

    // Validate URL
    if (url.find("http://") != 0 && url.find("https://") != 0) {
        last_error_ = "Invalid URL protocol (must be http:// or https://)";
        spdlog::error("Invalid URL: {}", url);
        return false;
    }

    std::ofstream output_file(output_path, std::ios::binary);
    if (!output_file.is_open()) {
        last_error_ = "Failed to open output file: " + output_path;
        spdlog::error("Failed to open output file: {}", output_path);
        return false;
    }

    // Reset curl options
    curl_easy_reset(curl);

    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Follow redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);

    // Set write callback
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output_file);

    // Set progress callback if provided
    if (progress_cb) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progress_cb);
    }

    // Set timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L); // 5 minutes
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);

    // Set user agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Tsuki-Engine/0.1.4");

    // Perform the request
    spdlog::info("Downloading from: {}", url);
    CURLcode res = curl_easy_perform(curl);

    output_file.close();

    if (res != CURLE_OK) {
        last_error_ = std::string("curl_easy_perform() failed: ") + curl_easy_strerror(res);
        spdlog::error("Download failed: {}", last_error_);
        return false;
    }

    // Get response code
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code_);

    if (response_code_ >= 400) {
        last_error_ = "HTTP error: " + std::to_string(response_code_);
        spdlog::error("HTTP error {}", response_code_);
        return false;
    }

    spdlog::info("Download completed successfully");
    return true;
}

bool HttpClient::getHeaders(const std::string& url) {
    if (!curl_handle_) {
        last_error_ = "CURL not initialized";
        return false;
    }

    CURL* curl = static_cast<CURL*>(curl_handle_);

    // Reset curl options
    curl_easy_reset(curl);

    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Get headers only
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // Set timeout
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    // Perform the request
    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        last_error_ = std::string("curl_easy_perform() failed: ") + curl_easy_strerror(res);
        return false;
    }

    // Get response code
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code_);

    return response_code_ < 400;
}

} // namespace tsuki::cli