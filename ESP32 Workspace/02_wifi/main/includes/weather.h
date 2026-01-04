#ifndef __WEATHER_H__
#define __WEATHER_H__

#include <stdio.h>
#include <string.h>
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"


esp_err_t _http_event_handler(esp_http_client_event_t *evt);
void fetch_weather_data();

#endif