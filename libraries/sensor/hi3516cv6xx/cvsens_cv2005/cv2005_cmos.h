/*
  Copyright (c), 2001-2025, Shenshu Tech. Co., Ltd.
 */

#ifndef CV2005_CMOS_H
#define CV2005_CMOS_H

#include "ot_common_isp.h"
#include "ot_sns_ctrl.h"
#include "securec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/****************************************************************************
 * basic info config                                                        *
 ****************************************************************************/
/* sensor ID & Resolution; chip ID 0x2004 at 0x3003/0x3002 */
#define CV2005_ID             2005
#define CV2005_WIDTH          1920
#define CV2005_HEIGHT         1080

/*
 * sensor Register Address. Every value is little-endian across consecutive
 * registers, the low byte at the lowest address. These are the registers,
 * and the order, the vendor's CV608 firmware hands the ISP to sync.
 */
#define CV2005_EXPO_L_ADDR    0x3048 /* shutter start: frame length - integration time */
#define CV2005_EXPO_M_ADDR    0x3049
#define CV2005_EXPO_H_ADDR    0x304a
#define CV2005_AGAIN_ADDR     0x3118 /* Recovered stock analog code->gain mapping matches integer formula */
#define CV2005_DGAIN_L_ADDR   0x311c /* Recovered stock digital code low byte; analog-region value 0x40 */
#define CV2005_DGAIN_H_ADDR   0x311d /* Recovered stock digital code high byte; analog-region value 0x00 */
#define CV2005_VMAX_L_ADDR    0x301c /* Stock 16-bit VMAX: low byte */
#define CV2005_VMAX_M_ADDR    0x301d /* Stock 16-bit VMAX: high byte */

/****************************************************************************
 * i2c bus config                                                           *
 ****************************************************************************/
/* sensor I2C bus config */
/* Stock runtime sensor.conf reports snr_addr=107 decimal (0x6B). */
#define CV2005_I2C_ADDR    0x6b
#define CV2005_ADDR_BYTE   2
#define CV2005_DATA_BYTE   1

/****************************************************************************
 * sensor lines configs                                                     *
 ****************************************************************************/
/* Development timing candidate: 1180 lines at 30 fps.
 * Cap 14160 reflects previously recovered stock evidence, not a verified
 * board limit. No automatic slow-shutter extension; hardware unvalidated.
 * Live stock evidence reports 1920x1080 RAW10 linear at 25 fps and VMAX 1416;
 * cmos_fps_set() derives that VMAX from the 1180-line 30 fps base timing.
 */
#define CV2005_VMAX_LINEAR            1180
#define CV2005_FULL_LINES_MAX_LINEAR  14160
#define CV2005_FPS_MAX_LINEAR 30
#define CV2005_FPS_MIN_LINEAR 2.5
#define CV2005_WIDTH_LINEAR  1920
#define CV2005_HEIGHT_LINEAR 1080
#define CV2005_MODE_LINEAR 0

/****************************************************************************
 * sensor ae configs                                                        *
 ****************************************************************************/
/* Stock analog region: codes 0..0xf8, raw gains 1024..32768.
 * 1x..32x assumes 1024-unity scaling. Fixed sensor digital gain is a
 * development restriction: the stock combined table also contains 224 digital-extension records, not enabled here.
 * 1024-unity scaling validated by stock table (249 records, zero
 * mismatches). ISP digital-gain policy
 * VERIFIED: destination majestic binary confirms sensor dgain is pinned
 * at 1x and ISP supplements via isp.aGain. Destination ABI VERIFIED:
 * soft-float EABI5 (see research notes). No hardware validation is implied.
 */
#define CV2005_AGAIN_CODE_MAX 0xf8 /* Recovered stock analog-region final code */
#define CV2005_AGAIN_MIN    1024 /* Recovered stock analog-region minimum raw gain */

/* Optional stock digital-extension rows (indices 249..472): the analog code
 * stays 0xf8 while the digital code rises 0x40..0x200 in steps of 2; gain is
 * 512 * digital code (1024 unity). Off by default: destination ISP semantics
 * are unvalidated and nothing here is board tested.
 */
#ifndef CV2005_ENABLE_DIGITAL_EXTENSION
#define CV2005_ENABLE_DIGITAL_EXTENSION 0
#endif
#if CV2005_ENABLE_DIGITAL_EXTENSION
#define CV2005_GAIN_INDEX_MAX 472
#define CV2005_AGAIN_MAX    262144
#else
#define CV2005_GAIN_INDEX_MAX CV2005_AGAIN_CODE_MAX
#define CV2005_AGAIN_MAX    32768
#endif
#define CV2005_DGAIN_1X     0x40 /* Recovered stock analog-region digital code 0x0040 */
#define CV2005_DGAIN_MIN    1024 /* Development restriction: sensor digital gain fixed */
#define CV2005_DGAIN_MAX    1024 /* Development restriction: sensor digital gain fixed */

#define DGAIN_ACCURACY         0.015625 /* Inherited AE accuracy descriptor; destination semantics unvalidated */
#define FL_OFFSET_LINEAR       5 /* Stock AE margin: frame length minus 5 */
/* Saved stock FPS path: 0x44e6ca subtracts 10; 0x44e6fa stores
 * the reported integration maximum. Other AE paths retain margin 5.
 * Static reporting evidence only, not a board-validated sensor limit.
 */
#define CV2005_FPS_INT_MARGIN 10
#define CV2005_INT_TIME_MIN    2 /* Stock AE minimum request in lines */
#define CV2005_SHS_MIN         5 /* Stock post-rounding lower clamp */
#define CV2005_SHS_FL_MARGIN   1 /* Stock upper clamp: frame length minus 1 */

/****************************************************************************
 * sensor awb calibrate configs                                             *
 ****************************************************************************/
/* Stock CV2005 AWB defaults recovered from firmware (0x44C6A0); not board validated. */
#define CALIBRATE_STATIC_TEMP       5000
#define CALIBRATE_STATIC_WB_R_GAIN  409
#define CALIBRATE_STATIC_WB_GR_GAIN 256
#define CALIBRATE_STATIC_WB_GB_GAIN 256
#define CALIBRATE_STATIC_WB_B_GAIN  452

/* Calibration results for Auto WB Planck */
#define CALIBRATE_AWB_P1 36
#define CALIBRATE_AWB_P2 220
#define CALIBRATE_AWB_Q1 0
#define CALIBRATE_AWB_A1 218409
#define CALIBRATE_AWB_B1 128
#define CALIBRATE_AWB_C1 (-167686)

/* Rgain and Bgain of the golden sample */
#define GOLDEN_RGAIN                                  0
#define GOLDEN_BGAIN                                  0

/****************************************************************************
 * sensor other configs                                                     *
 ****************************************************************************/
#define STANDARD_FPS              30
#define FLICKER_FREQ            (50 * 256)  /* light flicker freq: 50Hz, accuracy: 256 */
#define INIT_EXP_DEFAULT_LINEAR   130000
#define AE_COMENSATION_DEFAULT    0x40
/* Black level: 14-bit ISP scale, 64 DN at 10 bits, what the vendor's ISP runs */
#define BLACK_LEVEL_DEFAULT            0x400

/* DNG */
#define DNG_RAW_FORMAT_BIT_LINEAR         10 /* raw 10 bit */
#define DNG_RAW_FORMAT_WHITE_LEVEL_LINEAR 1023 /* 2^10 - 1 */

/****************************************************************************
 * assist function macros                                                   *
 ****************************************************************************/
#define high_8bits(x) (((x) & 0xff00) >> 8)
#define low_8bits(x)  ((x) & 0x00ff)

/****************************************************************************
 * sensor data type defines                                                 *
 ****************************************************************************/
/* define your sensor modes */
typedef enum {
    CV2005_1080P30_10BIT_LINEAR_MODE = 0,
    CV2005_MODE_MAX
} cv2005_res_mode;

ot_isp_sns_obj *cv2005_get_obj(td_void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
