#include "nvs_flash.h"
#include "esp_log.h"

#include "network/Wifi.hpp"
#include "common/Constants.hpp"
#include "board/Board.hpp"


extern "C" void app_main(void) {
    /* set Munchen timezone */
    // setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    // tzset();

    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(wifi::connect());

    Board board;
    board.run();
}
