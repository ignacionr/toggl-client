#pragma once

#include <chrono>
#include <format>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <glaze/glaze.hpp>
#include <glaze/glaze_exceptions.hpp>
#include <cppcodec/base64_default_rfc4648.hpp>

namespace ignacionr::toggl
{
    template<typename fetch_t>
    class client
    {
    public:

        using token_getter_t = std::function<std::string()>;
        using notifier_t = std::function<void(std::string_view)>;

        struct error: public std::runtime_error {
            error(std::string const &what): std::runtime_error{what} {}
        };

        struct config_error: error {
            config_error(std::string const &what): error{what} {}
        };

        struct no_login_set: error {
            no_login_set(): error{"No login set"} {}
        };

        struct time_entry {
            long long id;
            std::string created_with {"Ops2"};
            std::string description;
            int duration {-1};
            std::string start;
            std::string stop;
            std::string at;
            std::optional<std::string> server_deleted_at;
            long long user_id;
            long long uid;
            long wid;
            void set_start(std::chrono::system_clock::time_point const &tp) {
                start = std::format("{:%FT%T}Z", tp);
            }
            std::vector<std::string> tags {"adhoc"};
            std::vector<long long> tag_ids;
            long long workspace_id;
            std::optional<long long> project_id;
            std::optional<long long> task_id;
            bool billable;
            bool duronly;
        };

        client(notifier_t notifier, fetch_t fetch, token_getter_t login)
            : baseUrl("https://api.track.toggl.com/api/v9/"), notifier_{notifier}, fetch_{fetch}, login_{login}
        {
        }

        std::vector<time_entry> getTimeEntries()
        {
            auto str = performRequest(std::format("{}me/time_entries", baseUrl), "GET");
            std::vector<time_entry> entries;
            glz::ex::read_json(entries, str);
            return entries;
        }

        std::string startTimeEntry(long long workspace_id, const std::string_view description_text)
        {
            time_entry entry;
            entry.description = description_text;
            entry.workspace_id = workspace_id;
            entry.set_start(std::chrono::system_clock::now());
            entry.tags = {"adhoc"};

            return performRequest(
                    std::format("{}workspaces/{}/time_entries", baseUrl, workspace_id),
                    "POST",
                    glz::ex::to_json(entry));
        }

        // std::string stopTimeEntry(auto &entry)
        // {
        //     auto url = std::format("{}workspaces/{}/time_entries/{}",
        //                            baseUrl,
        //                            entry["workspace_id"].get<long long>(),
        //                            entry["id"].get<long long>());
        //     auto data = std::format("{{\"stop\":\"{}\"}}",
        //                             std::format("{:%FT%T}Z", std::chrono::system_clock::now()));
        //     return performRequest(url, "PUT", data);
        // }

        // void deleteTimeEntry(auto &entry)
        // {
        //     auto url = std::format("{}workspaces/{}/time_entries/{}",
        //                            baseUrl,
        //                            entry["workspace_id"].get<long long>(),
        //                            entry["id"].get<long long>());
        //     performRequest(url, "DELETE");
        // }

    private:
        std::string baseUrl;

        std::string login_header() const
        {
            if (!login_) throw no_login_set{};

            std::string auth_token = login_() + ":api_token";
            return std::format("Authorization: Basic {}", cppcodec::base64_rfc4648::encode(auth_token));
        }

        std::string performRequest(const std::string &url, const std::string &method, const std::string &data = {})
        {
            auto string_response = fetch_(url, method, [this, has_data = data.empty()](auto &request){
                request.header(login_header());
                request.header("Accept: application/json");
                if (has_data)
                {
                    request.header("Content-Type: application/json");
                }
            }, data);
            return string_response;
        }
        static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
        {
            ((std::string *)userp)->append((char *)contents, size * nmemb);
            return size * nmemb;
        }
        notifier_t notifier_;
        fetch_t fetch_;
        token_getter_t login_;
    };
}