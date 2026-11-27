#include <unistd.h>
#include "sensor_common.h"
#include "cv2005_cmos.h"
#include "cv2005_cfg.h"
#include "cv2005_sensor_ctrl.h"

td_s32 cv2005_linear_1080p30_10bit_init(cis_info *cis)
{
    static const td_u16 startup_addr[8] = {
        CV2005_EXPO_L_ADDR, CV2005_EXPO_M_ADDR, CV2005_EXPO_H_ADDR,
        CV2005_AGAIN_ADDR, CV2005_DGAIN_L_ADDR, CV2005_DGAIN_H_ADDR,
        CV2005_VMAX_L_ADDR, CV2005_VMAX_M_ADDR
    };
    td_u32 data[8];
    td_u32 i, fl, shs, vmax;
    ot_isp_sns_state *st;
    sns_check_pointer_return(cis);
    st = cis->sns_state;
    sns_check_pointer_return(st);
    fl = st->fl[0];
    if (fl <= CV2005_SHS_MIN || fl > CV2005_FULL_LINES_MAX_LINEAR) {
        return TD_FAILURE;
    }
    /* Board-validation fix (2026-11-27): at ISP start, before the first AE
     * update, the queued sync snapshot is at reset values (digital gain 0,
     * VMAX 0, exposure 0). The validation guard below requires a consistent
     * snapshot and silently returned TD_FAILURE, so the 30-entry init table
     * never executed and the sensor was never programmed. Seed a reset-state
     * snapshot with the defaults the AE would queue (1x digital gain, VMAX
     * = frame length, longest in-frame exposure), which is what the stock
     * routine's queued sync writes carry at startup. Indices follow the
     * sync payload order: 0-2 exposure, 3 again, 4-5 dgain, 6-7 VMAX. */
    if (st->regs_info[0].i2c_data[4].data == 0 &&
        st->regs_info[0].i2c_data[5].data == 0) {
        st->regs_info[0].i2c_data[4].data = CV2005_DGAIN_1X & 0xff;
        st->regs_info[0].i2c_data[5].data = (CV2005_DGAIN_1X >> 8) & 0xff;
    }
    if (st->regs_info[0].i2c_data[6].data == 0 &&
        st->regs_info[0].i2c_data[7].data == 0) {
        st->regs_info[0].i2c_data[6].data = fl & 0xff;
        st->regs_info[0].i2c_data[7].data = (fl >> 8) & 0xff;
    }
    if (st->regs_info[0].i2c_data[0].data == 0 &&
        st->regs_info[0].i2c_data[1].data == 0 &&
        st->regs_info[0].i2c_data[2].data == 0) {
        td_u32 shs_dft = fl - CV2005_SHS_FL_MARGIN;
        st->regs_info[0].i2c_data[0].data = shs_dft & 0xff;
        st->regs_info[0].i2c_data[1].data = (shs_dft >> 8) & 0xff;
        st->regs_info[0].i2c_data[2].data = (shs_dft >> 16) & 0xff;
    }
    for (i = 0; i < 8; ++i) {
        data[i] = st->regs_info[0].i2c_data[i].data;
        if (data[i] > 0xffU) {
            return TD_FAILURE;
        }
    }
    shs = data[0] | (data[1] << 8) | (data[2] << 16);
    vmax = data[6] | (data[7] << 8);
    if (shs < CV2005_SHS_MIN || shs > fl - CV2005_SHS_FL_MARGIN ||
        vmax != fl ||
        data[3] > CV2005_AGAIN_CODE_MAX ||
        (data[4] | (data[5] << 8)) != CV2005_DGAIN_1X) {
        return TD_FAILURE;
    }
    sns_check_return(cis_write_reg(&cis->i2c, CV2005_RESET_ADDR, 0x01));
    sns_check_return(cis_write_reg(&cis->i2c, CV2005_RESET_ADDR, 0x00));
    usleep(1000);
    for (i = 0; i < (td_u32)CV2005_INIT_REG_COUNT; ++i) {
        sns_check_return(cis_write_reg(&cis->i2c,
            cv2005_linear_1080p30_10bit[i].addr,
            cv2005_linear_1080p30_10bit[i].data));
    }
    for (i = 0; i < 8; ++i) {
        sns_check_return(cis_write_reg(&cis->i2c, startup_addr[i], data[i]));
    }
    sns_check_return(cis_write_reg(&cis->i2c, CV2005_STREAM_ADDR, 0x00));
    printf("CV2005 development startup sequence completed; board validation pending.\n");
    return TD_SUCCESS;
}
