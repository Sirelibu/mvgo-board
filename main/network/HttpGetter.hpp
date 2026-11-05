#pragma once

#include "esp_http_client.h"

#include <string>
#include <optional>


class HttpGetter {
public:
    HttpGetter() {}

    std::optional<std::string> fetchDepartures();

private:
    static constexpr char MVG_URL[] =
    "https://www.mvg.de/api/bgw-pt/v3/departures"
    "?globalId=de:09162:1740&limit=5";
};