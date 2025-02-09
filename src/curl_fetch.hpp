#pragma once

#include <format>
#include <functional>
#include <stdexcept>
#include <string>

#include <curl/curl.h>

struct curl_fetch
{
    struct request_t
    {
        ~request_t()
        {
            curl_slist_free_all(headers);
        }
        void header(std::string const &header)
        {
            headers = curl_slist_append(headers, header.c_str());
        };
        struct curl_slist *headers{nullptr};
    };

    using header_config_t = std::function<void(request_t &)>;

    std::string operator()(std::string const &url, std::string const &method, header_config_t config, std::string const &data = {})
    {
        CURL *curl = curl_easy_init();
        if (curl)
        {
            request_t request;
            config(request);
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, request.headers);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            if (method != "GET")
            {
                curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
            }
            if (!data.empty())
            {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
            }
            else if (method != "GET")
            {
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
            }
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            std::string response;
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            try
            {
                CURLcode res = curl_easy_perform(curl);
                if (res != CURLE_OK)
                {
                    throw std::runtime_error("Failed to perform request: " + std::string(curl_easy_strerror(res)));
                }
                // obtain the response code
                long response_code;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                if (response_code != 200)
                {
                    throw std::runtime_error(std::format("Request failed with code: {} and message {}", response_code, response));
                }
            }
            catch (...)
            {
                curl_easy_cleanup(curl);
                throw;
            }
            curl_easy_cleanup(curl);
            return response;
        }
        throw std::runtime_error("Failed to initialize cURL");
    }
    static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        ((std::string *)userp)->append((char *)contents, size * nmemb);
        return size * nmemb;
    }
};