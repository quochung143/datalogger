/**
 * @file fonts.h
 *
 * @brief Font and Image Definitions for LCD Display
 */

#ifndef FONT_H
#define FONT_H

/* INCLUDES ------------------------------------------------------------------*/

#include "stdint.h"

/* TYPEDEFS ------------------------------------------------------------------*/

typedef struct
{
    const uint8_t width;
    uint8_t height;
    const uint16_t *data;
} FontDef;

/* EXTERNAL VARIABLES ---------------------------------------------------------*/

// Font lib. (OPTIMIZED: Removed Font_16x26 to save FLASH)
extern FontDef Font_7x10;
extern FontDef Font_11x18;

// 16-bit(RGB565) Image lib.
/*******************************************
 *             CAUTION:
 *   If the MCU onchip flash cannot
 *  store such huge image data,please
 *           do not use it.
 * These pics are for test purpose only.
 *******************************************/

/* 128x128 pixel RGB565 image
extern const uint16_t saber[][128];
*/

/* 240x240 pixel RGB565 image
extern const uint16_t knky[][240];
extern const uint16_t tek[][240];
extern const uint16_t adi1[][240];
*/
#endif /* FONT_H */
