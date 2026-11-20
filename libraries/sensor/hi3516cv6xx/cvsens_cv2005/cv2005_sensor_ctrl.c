/* Offline startup candidate, not compiled or board-validated.
 * Recovered table includes 0x3000=0 before payload writes; final write repeats it.
 * Payload uses stock's eight-entry layout (exposure x3, gain x3, frame length x2).
 * Snapshot data by index: do not depend on reg_num/address metadata being ready.
 */
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
