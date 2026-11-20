/* File-backed candidate from reviewed ELF c69cf9aa87f6ace806d0254f4b03b57a41803244bfcb4d170dab7f51e82f00f8.
 * VA 0x006f9898, file 0x006e8898, 30 address/value records.
 * Region SHA256: 9021a27c3cb80339574a721d8db0bf41ee1fa3bd1c4cd903d06c4446bb2f1b02.
 * Runtime immutability and board behavior are NOT established.
 * Preserve ordering, duplicates, and the stream-control write at index 26.
 */
#ifndef CV2005_CFG_H
#define CV2005_CFG_H
#include "sensor_common.h"
#define CV2005_RESET_ADDR 0x3004
#define CV2005_STREAM_ADDR 0x3000
static const cis_reg_cfg cv2005_linear_1080p30_10bit[] = {
    {0x301c, 0x9c},
    {0x301d, 0x04},
    {0x301e, 0x00},
    {0x3020, 0x4e},
    {0x3021, 0x04},
    {0x3808, 0x41},
    {0x3031, 0x00},
    {0x3204, 0x40},
    {0x3109, 0x01},
    {0x3499, 0x01},
    {0x356f, 0x0f},
    {0x310c, 0x64},
    {0x359d, 0x00},
    {0x389d, 0x0a},
    {0x389c, 0x6a},
    {0x38a0, 0x2b},
    {0x3878, 0x01},
    {0x3879, 0x15},
    {0x366c, 0x10},
    {0x359f, 0x15},
    {0x356f, 0x0f},
    {0x3420, 0x2f},
    {0x3422, 0x87},
    {0x3424, 0x3f},
    {0x3426, 0x57},
    {0x3428, 0x2f},
    {0x3000, 0x00},
    {0x3a0d, 0x01},
    {0x3a07, 0x01},
    {0x3a07, 0x00},
};
#define CV2005_INIT_REG_COUNT (sizeof(cv2005_linear_1080p30_10bit) / sizeof(cv2005_linear_1080p30_10bit[0]))
#endif
