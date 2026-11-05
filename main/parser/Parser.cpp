#include "Parser.hpp"
#include "common/Constants.hpp"

#include "cJSON.h"
#include "esp_log.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <optional>

namespace parser {

namespace {

using JsonPtr = std::unique_ptr<cJSON, decltype(&cJSON_Delete)>;

std::optional<std::string> getString(const cJSON* object, const char* key) {
    const cJSON* item = cJSON_GetObjectItemCaseSensitive(object, key);

    if (!cJSON_IsString(item) || item->valuestring == nullptr) {
        ESP_LOGE(TAG, "Missing or invalid timestamp: %s", key);
        return std::nullopt;
    }

    return item->valuestring;
}

std::optional<int64_t> getTimestamp(const cJSON* object, const char* key) {
    const cJSON* item = cJSON_GetObjectItemCaseSensitive(object, key);

    if (!cJSON_IsNumber(item)) {
        ESP_LOGE(TAG, "Missing or invalid timestamp: %s", key);
        return std::nullopt;
    }

    return static_cast<int64_t>(item->valuedouble);
}

int getOptionalIntOrFallback(const cJSON* object, const char* key, int fallback) {
    const cJSON* item = cJSON_GetObjectItemCaseSensitive(object, key);

    return cJSON_IsNumber(item) ? item->valueint : fallback;
}

bool getOptionalBoolOrFallback(const cJSON* object, const char* key, bool fallback) {
    const cJSON* item = cJSON_GetObjectItemCaseSensitive(object, key);

    return cJSON_IsBool(item) ? cJSON_IsTrue(item) : fallback;
}

Departure parseDeparture(const cJSON* object) {
    Departure result;

    if (auto label = getString(object, "label"); label) {
        result.label = std::move(*label);
    } else {
        result.label = "NO LABEL";
    }

    if (auto destination = getString(object, "destination"); destination) {
        result.destination = std::move(*destination);
    } else {
        result.destination = "NO DESTINATION";
    }

    if (auto time = getTimestamp(object, "plannedDepartureTime"); time) {
        result.plannedDepartureTime = *time;
    } else {
        result.plannedDepartureTime = -1;
    }

    if (auto time = getTimestamp(object, "realtimeDepartureTime"); time) {
        result.realtimeDepartureTime = *time;
    } else {
        result.realtimeDepartureTime = -1;
    }
    result.cancelled = getOptionalBoolOrFallback(object, "cancelled", false);
    result.delayInMinutes = getOptionalIntOrFallback(object, "delayInMinutes", 0);

    return result;
}

} // namespace

std::vector<Departure> parseHttpResp(std::string_view json) {
    JsonPtr root(cJSON_ParseWithLength(json.data(), json.size()), &cJSON_Delete);

    if (!root) {
        ESP_LOGE(TAG, "Invalid JSON response");
        return {};
    }

    if (!cJSON_IsArray(root.get())) {
        ESP_LOGE(TAG, "Expected JSON array");
        return {};
    }

    const int count = cJSON_GetArraySize(root.get());

    std::vector<Departure> departures;
    departures.reserve(count);

    const cJSON* item = nullptr;

    cJSON_ArrayForEach(item, root.get()) {
        if (!cJSON_IsObject(item)) {
            ESP_LOGE(TAG, "Invalid departure object");
            continue;
        }

        departures.push_back(parseDeparture(item));
    }

    return departures;
}

} // namespace parser