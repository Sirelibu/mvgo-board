#include "HttpGetter.hpp"
#include "common/Constants.hpp"

#include "esp_log.h"
#include "esp_crt_bundle.h"

namespace {
esp_err_t httpEventHandler(esp_http_client_event_t* event) {
    if (event->event_id == HTTP_EVENT_ON_DATA) {
        auto* response =
            static_cast<std::string*>(event->user_data);

        response->append(
            static_cast<const char*>(event->data),
            event->data_len
        );
    }

    return ESP_OK;
}
}

std::string HttpGetter::getUrl() const {
    return std::string(MVG_URL) + "?globalId=" + globalId + "&limit=" + std::to_string(limit);
}

std::optional<std::string> HttpGetter::fetchDepartures() const {
    std::string response;

    esp_http_client_config_t config = {};
    std::string url = getUrl();
    config.url = url.c_str();
    config.method = HTTP_METHOD_GET;
    config.event_handler = httpEventHandler;
    config.user_data = &response;
    config.crt_bundle_attach = esp_crt_bundle_attach;
    config.timeout_ms = 15000;

    esp_http_client_handle_t client =
        esp_http_client_init(&config);

    if (client == nullptr) {
        ESP_LOGE(TAG, "HTTP client initialization failed");
        return std::nullopt;
    }

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP status: %d", status);
    } else {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return std::nullopt;
    }

    esp_http_client_cleanup(client);
    return response;
}
