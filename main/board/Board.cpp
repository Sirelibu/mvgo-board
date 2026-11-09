#include "Board.hpp"
#include "common/Constants.hpp"
#include "parser/Parser.hpp"

#include "esp_log.h"

#include <chrono>

namespace {
std::tm getDepartureTime(int64_t timestampMs) {
    const time_t seconds =
        static_cast<time_t>(timestampMs / 1000);

    std::tm local{};
    localtime_r(&seconds, &local);
    return local;
}
} // namespace

void Board::run() {
    while(true) {
        auto response = httpGetter.fetchDepartures();
        if (not response) {
            ESP_LOGE(TAG, "HTTP request failed.");
        } else {
            const auto departures = parser::parseHttpResp(*response);

            for (const auto& departure : departures) {
                ESP_LOGI(
                    TAG,
                    "%s -> %s, depart: %02d:%02d",
                    departure.label.c_str(),
                    departure.destination.c_str(),
                    getDepartureTime(departure.realtimeDepartureTime).tm_hour,
                    getDepartureTime(departure.realtimeDepartureTime).tm_min
                );
            }
        }
        /* 
            updateImgOnDisplay(response);
        */
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}
