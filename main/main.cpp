#include "nvs_flash.h"
#include "esp_log.h"

#include "network/HttpGetter.hpp"
#include "network/Wifi.hpp"
#include "common/Constants.hpp"


extern "C" void app_main(void) {
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
            ESP_LOGI(TAG, "Response size: %u",
                 static_cast<unsigned>(response->size()));
        }
        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}
