#pragma once

#include "esp_http_client.h"

#include <string>
#include <optional>


class HttpGetter {
public:
    HttpGetter(const std::string& globalId, unsigned limit = 15) : globalId(globalId), limit(limit) {}

    std::optional<std::string> fetchDepartures() const;
private:
    std::string getUrl() const;

    static constexpr char MVG_URL[] =
    "https://www.mvg.de/api/bgw-pt/v3/departures";
    std::string globalId;
    unsigned limit;
};