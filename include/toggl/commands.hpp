#pragma once

#include <format>
#include <print>
#include <sstream>
#include <string>

#include "client.hpp"

namespace ignacionr::toggl {
    template <typename client_t>
    struct commands {
        commands(client_t &client): client_{client} {}
        void register(auto registry) {
            registry.register_command("list", [this](auto const &args) -> std::string {
                auto entries = client_.getTimeEntries();
                std::stringstream result;
                for(auto const &entry: entries) {
                    result << std::format("{}\n", entry.description);
                }
                return result.str();
            });
        }
    private:
        client_t &client_;
    };
}