#ifndef __IMAGES_H__
#define __IMAGES_H__
#include <stdint.h>

#define IMAGE_W 240
#define IMAGE_H 240

extern const uint16_t *om_img;
uint32_t om_size(void);
extern const uint16_t om[IMAGE_W * IMAGE_H];
#endif