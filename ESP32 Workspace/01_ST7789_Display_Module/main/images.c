#include "images.h"
#include <stdint.h>

// --- Existing Externs ---
extern const uint8_t _binary_temp_bin_start[];
extern const uint8_t _binary_temp_bin_end[];

extern const uint8_t _binary_humidity_bin_start[];
extern const uint8_t _binary_humidity_bin_end[];

extern const uint8_t _binary_w_hot_bin_start[];
extern const uint8_t _binary_w_hot_bin_end[];

extern const uint8_t _binary_korea_bin_start[];
extern const uint8_t _binary_korea_bin_end[];

extern const uint8_t _binary_sun_bin_start[];
extern const uint8_t _binary_sun_bin_end[];

extern const uint8_t _binary_w_rain_bin_start[];
extern const uint8_t _binary_w_rain_bin_end[];

extern const uint8_t _binary_w_snow_bin_start[];
extern const uint8_t _binary_w_snow_bin_end[];

extern const uint8_t _binary_w_wind_bin_start[];
extern const uint8_t _binary_w_wind_bin_end[];

extern const uint8_t _binary_snow_bin_start[];
extern const uint8_t _binary_snow_bin_end[];

extern const uint8_t _binary_rain_bin_start[];
extern const uint8_t _binary_rain_bin_end[];

extern const uint8_t _binary_hot_wind_bin_start[];
extern const uint8_t _binary_hot_wind_bin_end[];

const uint16_t *temp_img = (const uint16_t *)_binary_temp_bin_start;
const uint16_t *humidity_img = (const uint16_t *)_binary_humidity_bin_start;
const uint16_t *w_hot_img = (const uint16_t *)_binary_w_hot_bin_start;
const uint16_t *korea_img = (const uint16_t *)_binary_korea_bin_start;
const uint16_t *sun_img = (const uint16_t *)_binary_sun_bin_start;
const uint16_t *w_rain_img = (const uint16_t *)_binary_w_rain_bin_start;
const uint16_t *w_snow_img = (const uint16_t *)_binary_w_snow_bin_start;
const uint16_t *w_wind_img = (const uint16_t *)_binary_w_wind_bin_start;
const uint16_t *snow_img = (const uint16_t *)_binary_snow_bin_start;
const uint16_t *rain_img = (const uint16_t *)_binary_rain_bin_start;
const uint16_t *hot_wind_img = (const uint16_t *)_binary_hot_wind_bin_start;