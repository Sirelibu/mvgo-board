#include "Wifi.hpp"
#include "common/Constants.hpp"

#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include <cstring>
#include <string>

namespace wifi {

namespace {

constexpr EventBits_t WIFI_CONNECTED = BIT0;

EventGroupHandle_t events = nullptr;

void eventHandler(void*, esp_event_base_t base, int32_t id, void*) {
    if (base == WIFI_EVENT) {
        if (id == WIFI_EVENT_STA_START ||
            id == WIFI_EVENT_STA_DISCONNECTED) {
            xEventGroupClearBits(events, WIFI_CONNECTED);
            esp_wifi_connect();
        }
    }

    if (base == IP_EVENT &&
        id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(events, WIFI_CONNECTED);
    }
}

std::string readLine(const char* prompt) {
    std::printf("%s", prompt);
    std::fflush(stdout);

    std::string result;

    while (true) {
        const int ch = std::getchar();

        if (ch == '\n') {
            break;
        }

        if (ch == '\r') {
            continue;
        }

        if (ch == EOF) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        result.push_back(static_cast<char>(ch));
    }

    return result;
}

esp_err_t configureWifi() {
    wifi_config_t config{};

    auto ssid = readLine("Enter Wi-Fi SSID: ");
    auto password = readLine("Enter Wi-Fi password: ");

    if (ssid.empty() ||
        ssid.size() > sizeof(config.sta.ssid) ||
        password.size() > sizeof(config.sta.password)) {
        ESP_LOGE(TAG, "Invalid SSID or password length");
        return ESP_ERR_INVALID_ARG;
    }

    std::memcpy(config.sta.ssid, ssid.data(), ssid.size());
    std::memcpy(config.sta.password, password.data(), password.size());

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config));

    ESP_LOGI(TAG, "Wi-Fi credentials saved to flash");

    return ESP_OK;
}
} // namespace

esp_err_t connect() {
    if (events != nullptr) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    events = xEventGroupCreate();
    if (events == nullptr) {
        return ESP_ERR_NO_MEM;
    }

    wifi_init_config_t init = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&init));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(esp_event_handler_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &eventHandler,
        nullptr
    ));

    ESP_ERROR_CHECK(esp_event_handler_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &eventHandler,
        nullptr
    ));

    wifi_config_t config{};

    ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &config));

    if (config.sta.ssid[0] == '\0') {
        ESP_LOGI(TAG, "No saved Wi-Fi credentials");

        esp_err_t err = configureWifi();
        if (err != ESP_OK) {
            return err;
        }
    } else {
        ESP_LOGI(TAG, "Using saved Wi-Fi configuration");
    }

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Waiting for connection...");

    xEventGroupWaitBits(
        events,
        WIFI_CONNECTED,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY
    );

    return ESP_OK;
}
} // namespace wifi
