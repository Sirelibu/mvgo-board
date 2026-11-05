#include "nvs_flash.h"
#include "esp_log.h"

#include "network/HttpGetter.hpp"
#include "network/Wifi.hpp"
#include "common/Constants.hpp"
#include "parser/Parser.hpp"

#include <chrono>

std::tm getDepartureTime(int64_t timestampMs) {
    const time_t seconds =
        static_cast<time_t>(timestampMs / 1000);

    std::tm local{};
    localtime_r(&seconds, &local);
    return local;
}


extern "C" void app_main(void) {
    /* set Munchen timezone */
    // setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    // tzset();

    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(wifi::connect());

    HttpGetter httpGetter;
    while (true) {
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
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}
