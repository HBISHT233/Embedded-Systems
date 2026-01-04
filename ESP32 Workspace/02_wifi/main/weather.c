#include "weather.h"
static const char *TAG_WEATHER = "WEATHER";

// Global variables to store your weather data
float current_temp = 0.0;
float current_humidity = 0.0;
float current_wind_speed = 0.0;
int current_weather_code = 0;

// Buffer to store the incoming JSON string
#define MAX_HTTP_OUTPUT_BUFFER 2048
char response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};

// HTTP Event Handler
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static int output_len;
    switch(evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            // Append received data to our buffer
            if (!esp_http_client_is_chunked_response(evt->client)) {
                int copy_len = evt->data_len;
                if (output_len + copy_len < MAX_HTTP_OUTPUT_BUFFER) {
                    memcpy(response_buffer + output_len, evt->data, copy_len);
                    output_len += copy_len;
                    response_buffer[output_len] = '\0'; // Null terminate
                }
            }
            break;
        case HTTP_EVENT_ON_FINISH:
            output_len = 0;
            break;
        default:
            break;
    }
    return ESP_OK;
}

void fetch_weather_data()
{
    // 1. Configure the HTTP Request
    // Note: I switched "hourly" to "current" to make it easier for the ESP32 to display "Now"
    esp_http_client_config_t config = {
        .url = "https://api.open-meteo.com/v1/forecast?latitude=35.2281&longitude=128.6811&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m&timezone=auto",
        .event_handler = _http_event_handler,
        .disable_auto_redirect = true,
        .crt_bundle_attach = esp_crt_bundle_attach, // Attach SSL capabilities
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    // 2. Perform the Request
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG_WEATHER, "HTTP GET Status = %d, content_length = %lld",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));

        // 3. Parse JSON Response
        ESP_LOGI(TAG_WEATHER, "Parsing JSON...");
        
        cJSON *root = cJSON_Parse(response_buffer);
        if (root == NULL) {
            ESP_LOGE(TAG_WEATHER, "JSON Parse Error");
        } else {
            // Navigate to "current" object
            cJSON *current = cJSON_GetObjectItem(root, "current");
            if (current) {
                // Extract values
                current_temp = cJSON_GetObjectItem(current, "temperature_2m")->valuedouble;
                current_humidity = cJSON_GetObjectItem(current, "relative_humidity_2m")->valuedouble;
                current_wind_speed = cJSON_GetObjectItem(current, "wind_speed_10m")->valuedouble;
                current_weather_code = cJSON_GetObjectItem(current, "weather_code")->valueint;

                ESP_LOGI(TAG_WEATHER, "Temp: %.1f C, Hum: %.1f %%, Wind: %.1f km/h, Code: %d", 
                         current_temp, current_humidity, current_wind_speed, current_weather_code);
            }
            cJSON_Delete(root); // Free memory!
        }
    } else {
        ESP_LOGE(TAG_WEATHER, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
}

/*
// Inside your main loop or display task:

// 1. Fetch new data (Do not do this too often! Open-Meteo limit is ~10k calls/day)
// Maybe run this once every 15 minutes.
fetch_weather_data(); 

// 2. Update Display with LIVE variables
char buf[64];

// --- Temperature ---
snprintf(buf, sizeof(buf), "Temp = %.1f \xB0" "C", current_temp); // %.1f for float
lcd_draw_string(0, 170, buf, COLOR_WHITE, COLOR_BLACK, 2);

// --- Humidity ---
snprintf(buf, sizeof(buf), "Humidity = %.0f %%", current_humidity); // %.0f for no decimals
lcd_draw_string(0, 200, buf, COLOR_WHITE, COLOR_BLACK, 2);

// --- Weather Icon Logic (Bonus) ---
// You can switch icons based on "current_weather_code"
// WMO Codes: 0=Clear, 1-3=Cloudy, 61-65=Rain, 71=Snow
if (current_weather_code == 0) {
     lcd_draw_image(..., sun_img);
} else if (current_weather_code >= 61 && current_weather_code <= 65) {
     lcd_draw_image(..., rain_img);
}
// etc...


*/