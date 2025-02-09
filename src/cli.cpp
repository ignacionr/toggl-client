#include <print>
#include <cstdlib>

#include "../include/toggl/client.hpp"
#include "curl_fetch.hpp"

int main() {
    ignacionr::toggl::client client{
        [](std::string_view message) {
            std::println("{}", message);
    }, 
    curl_fetch{}, 
    []() -> std::string {
        char* token = nullptr;
        size_t len = 0;
        _dupenv_s(&token, &len, "TOGGL_API_TOKEN");
        std::string result = token ? token : "";
        free(token);
        return result;
    }};

    try {
        auto entries = client.getTimeEntries();
        for(auto const &entry: entries) {
            std::println("{}", entry.description);
        }
    }
    catch(std::exception const &e) {
        std::println("{}", e.what());
    }

    return 0;
}