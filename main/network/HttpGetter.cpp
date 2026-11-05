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

std::optional<std::string> HttpGetter::fetchDepartures() {
    std::string response;

    esp_http_client_config_t config = {};
    config.url = MVG_URL;
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
        /*
        // Print in chunks to avoid excessively long log lines.
        constexpr size_t CHUNK_SIZE = 256;

        for (size_t i = 0; i < response.size(); i += CHUNK_SIZE) {
            ESP_LOGI(TAG, "%.*s",
                static_cast<int>(
                    std::min(CHUNK_SIZE, response.size() - i)
                ),
                response.data() + i
            );
        }
        */
    } else {
        ESP_LOGE(TAG, "HTTP request failed: %s",
                 esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return std::nullopt;
    }

    esp_http_client_cleanup(client);
    return response;
}
