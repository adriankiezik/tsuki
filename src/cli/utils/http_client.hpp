#pragma once

#include <string>
#include <functional>
#include <memory>
#include <curl/curl.h>

namespace tsuki::cli {

// Progress callback: (bytes_downloaded, total_bytes, percentage)
using ProgressCallback = std::function<void(size_t, size_t, int)>;

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    // Download file to path
    bool downloadFile(const std::string& url, const std::string& output_path,
                     ProgressCallback progress_cb = nullptr);

    // Get HTTP headers only
    bool getHeaders(const std::string& url);

    // Get last error message
    std::string getLastError() const { return last_error_; }

    // Get HTTP response code
    long getResponseCode() const { return response_code_; }

private:
    std::string last_error_;
    long response_code_;
    void* curl_handle_; // CURL*

    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp);
    static int progressCallback(void* clientp, curl_off_t dltotal, curl_off_t dlnow,
                               curl_off_t ultotal, curl_off_t ulnow);
};

} // namespace tsuki::cli