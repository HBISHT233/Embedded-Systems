#include <string.h>
#include <stdbool.h>
#include <stddef.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_http_server.h"

#include "wifi_setup.h" // Your header file

// Define the TAG here, NOT in the header file
static const char *TAG = "WIFI_MGR";

#define AP_SSID "ESP32_Setup" // Name of the config Wi-Fi

// HTML Form for the user to enter credentials
const char* form_html = 
    "<html><body>"
    "<h2>ESP32 Config</h2>"
    "<form action=\"/save\" method=\"post\">"
    "SSID: <input type=\"text\" name=\"ssid\"><br>"
    "Pass: <input type=\"text\" name=\"pass\"><br>"
    "<input type=\"submit\" value=\"Connect\">"
    "</form></body></html>";

// --- Function to Write to NVS ---
void save_wifi_creds(const char* ssid, const char* pass) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error opening NVS handle!");
        return;
    }
    nvs_set_str(my_handle, "ssid", ssid);
    nvs_set_str(my_handle, "pass", pass);
    nvs_commit(my_handle);
    nvs_close(my_handle);
}

// --- Web Server Handler: GET / (Show Form) ---
esp_err_t root_get_handler(httpd_req_t *req) {
    httpd_resp_send(req, form_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// --- Web Server Handler: POST /save (Save Data) ---
esp_err_t save_post_handler(httpd_req_t *req) {
    char buf[100];
    int remaining = req->content_len;
    
    // Read the data sent by the form
    if (remaining >= sizeof(buf)) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    httpd_req_recv(req, buf, remaining);
    buf[remaining] = '\0'; // Null terminate

    // Parse simple URL encoded string (ssid=...&pass=...)
    char ssid[32] = {0}, pass[64] = {0};
    char *ssid_ptr = strstr(buf, "ssid=") + 5;
    char *pass_ptr = strstr(buf, "pass=") + 5;
    char *sep = strstr(ssid_ptr, "&");
    
    if (ssid_ptr && pass_ptr && sep) {
        // Extract SSID
        memcpy(ssid, ssid_ptr, sep - ssid_ptr);
        // Extract Password (rest of string)
        strcpy(pass, pass_ptr);

        ESP_LOGI(TAG, "Saving Credentials: %s / %s", ssid, pass);
        save_wifi_creds(ssid, pass);

        httpd_resp_send(req, "Saved! Rebooting...", HTTPD_RESP_USE_STRLEN);
        
        // Wait 2 seconds then restart
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        esp_restart();
    }
    return ESP_OK;
}

// --- Start the Web Server ---
void start_webserver(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t root_uri = { .uri = "/", .method = HTTP_GET, .handler = root_get_handler, .user_ctx = NULL };
        httpd_register_uri_handler(server, &root_uri);

        httpd_uri_t save_uri = { .uri = "/save", .method = HTTP_POST, .handler = save_post_handler, .user_ctx = NULL };
        httpd_register_uri_handler(server, &save_uri);
    }
}

// --- Main Manager Logic ---
// ... keep includes at the top ...

void wifi_init_manager(void) {
    // 1. Initialize NVS
    esp_wifi_set_max_tx_power(78);
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Init Network Interface
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // 3. Check if SSID exists in NVS
    nvs_handle_t my_handle;
    char ssid[32] = {0};
    char pass[64] = {0};
    size_t len = sizeof(ssid);
    
    bool has_creds = false;
    if (nvs_open("storage", NVS_READONLY, &my_handle) == ESP_OK) {
        if (nvs_get_str(my_handle, "ssid", ssid, &len) == ESP_OK) {
            len = sizeof(pass);
            nvs_get_str(my_handle, "pass", pass, &len);
            has_creds = true;
        }
        nvs_close(my_handle);
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    if (has_creds) {
        // --- MODE: STATION (Connect to Router) ---
        ESP_LOGI(TAG, "Credentials found! Connecting to %s...", ssid);
        esp_netif_create_default_wifi_sta();
        
        wifi_config_t wifi_config = {0};
        strcpy((char*)wifi_config.sta.ssid, ssid);
        strcpy((char*)wifi_config.sta.password, pass);

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_ERROR_CHECK(esp_wifi_connect());
    } else {
        // --- MODE: ACCESS POINT (Configuration) ---
        ESP_LOGI(TAG, "No credentials. Starting AP Mode (WPA2)...");
        esp_netif_create_default_wifi_ap();

        wifi_config_t wifi_config = {
            .ap = {
                .ssid = AP_SSID,
                .ssid_len = strlen(AP_SSID),
                .channel = 1,
                // ADDING PASSWORD HERE
                .password = "12345678", 
                .max_connection = 4,
                // CHANGING SECURITY TYPE
                .authmode = WIFI_AUTH_WPA2_PSK, 
                .pmf_cfg = {
                    .required = false,
                },
            },
        };
        
        // Safety check: Password must be at least 8 chars for WPA2
        if (strlen((char *)wifi_config.ap.password) == 0) {
            wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        }

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
        
        start_webserver();
    }
}