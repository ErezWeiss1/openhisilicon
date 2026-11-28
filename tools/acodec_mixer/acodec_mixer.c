/* Test helper for the Hi3516CV610 inner audio codec (/dev/acodec).
 *
 * Usage:
 *   acodec_mixer 0|1|2        select mic input (0=IN0, 1=IN1, 2=differential)
 *   acodec_mixer gain <0-31>  set analogue input gain, both channels
 *   acodec_mixer boost <0|1>  enable/disable the analogue input boost
 *   acodec_mixer vol <0-126>  set input volume (0x7F=mute)
 *   acodec_mixer adcvol <0-126> set ADC (digital input) volume, both channels
 *
 * Ioctl numbers come from kernel/include/hi3516cv6xx/ot_acodec.h.
 * Mixer/gain/boost/volume registers only; nothing is written to flash.
 * Majestic resets the mixer to IN1 when its audio pipeline restarts. */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOC_TYPE_ACODEC 'A'

typedef struct {
    unsigned int volume_ctrl;      /* 0x00~0x7e, 0x7F:mute */
    unsigned int volume_ctrl_mute; /* 1:mute, 0:unmute */
} acodec_volume_ctrl;

enum {
    IOC_NR_SOFT_RESET_CTRL = 0x0,
    IOC_NR_SET_I2S1_FS,
    IOC_NR_SET_MIXER_MIC,
    IOC_NR_SET_INPUT_VOL,
    IOC_NR_SET_OUTPUT_VOL,
    IOC_NR_GET_INPUT_VOL,
    IOC_NR_GET_OUTPUT_VOL,
    IOC_NR_BOOSTL_ENABLE,
    IOC_NR_BOOSTR_ENABLE,
    IOC_NR_SET_GAIN_MICL,
    IOC_NR_SET_GAIN_MICR,
    IOC_NR_GET_GAIN_MICL,
    IOC_NR_GET_GAIN_MICR,
    IOC_NR_SET_DACL_VOL,
    IOC_NR_SET_DACR_VOL,
    IOC_NR_SET_ADCL_VOL,
    IOC_NR_SET_ADCR_VOL,
};

#define OT_ACODEC_SET_MIXER_MIC _IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_MIXER_MIC, unsigned int)
#define OT_ACODEC_SET_INPUT_VOLUME _IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_INPUT_VOL, unsigned int)
#define OT_ACODEC_ENABLE_BOOSTL _IOWR(IOC_TYPE_ACODEC, IOC_NR_BOOSTL_ENABLE, unsigned int)
#define OT_ACODEC_ENABLE_BOOSTR _IOWR(IOC_TYPE_ACODEC, IOC_NR_BOOSTR_ENABLE, unsigned int)
#define OT_ACODEC_SET_GAIN_MICL _IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_GAIN_MICL, unsigned int)
#define OT_ACODEC_SET_GAIN_MICR _IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_GAIN_MICR, unsigned int)
#define OT_ACODEC_SET_ADCL_VOLUME _IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_ADCL_VOL, acodec_volume_ctrl)
#define OT_ACODEC_SET_ADCR_VOLUME _IOWR(IOC_TYPE_ACODEC, IOC_NR_SET_ADCR_VOL, acodec_volume_ctrl)

static int fd = -1;

static int set_u32(unsigned long req, unsigned int val, const char *what)
{
    if (ioctl(fd, req, &val) < 0) {
        printf("%s: ioctl failed", what);
        perror(" ");
        return -1;
    }
    printf("%s set to %u\n", what, val);
    return 0;
}

static int set_adc_vol(unsigned int val, const char *what)
{
    acodec_volume_ctrl c;
    c.volume_ctrl = val;
    c.volume_ctrl_mute = 0;
    if (ioctl(fd, OT_ACODEC_SET_ADCL_VOLUME, &c) < 0) {
        printf("%s left: ioctl failed", what);
        perror(" ");
        return -1;
    }
    if (ioctl(fd, OT_ACODEC_SET_ADCR_VOLUME, &c) < 0) {
        printf("%s right: ioctl failed", what);
        perror(" ");
        return -1;
    }
    printf("%s set to %u (both channels)\n", what, val);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s 0|1|2 | gain <0-31> | boost <0|1> | vol <0-126> | adcvol <0-126>\n", argv[0]);
        return 2;
    }
    fd = open("/dev/acodec", O_RDWR);
    if (fd < 0) {
        perror("open /dev/acodec");
        return 1;
    }
    int rc = 0;
    if (strcmp(argv[1], "gain") == 0 && argc == 3) {
        unsigned int g = (unsigned int)strtoul(argv[2], 0, 0);
        if (g > 0x1f) { fprintf(stderr, "gain must be 0-31\n"); rc = 2; goto out; }
        rc = set_u32(OT_ACODEC_SET_GAIN_MICL, g, "gain_left");
        rc |= set_u32(OT_ACODEC_SET_GAIN_MICR, g, "gain_right");
    } else if (strcmp(argv[1], "boost") == 0 && argc == 3) {
        unsigned int b = (unsigned int)strtoul(argv[2], 0, 0);
        if (b > 1) { fprintf(stderr, "boost must be 0 or 1\n"); rc = 2; goto out; }
        rc = set_u32(OT_ACODEC_ENABLE_BOOSTL, b, "boost_left");
        rc |= set_u32(OT_ACODEC_ENABLE_BOOSTR, b, "boost_right");
    } else if (strcmp(argv[1], "vol") == 0 && argc == 3) {
        unsigned int v = (unsigned int)strtoul(argv[2], 0, 0);
        if (v > 0x7e) { fprintf(stderr, "vol must be 0-126 (127=mute)\n"); rc = 2; goto out; }
        rc = set_u32(OT_ACODEC_SET_INPUT_VOLUME, v, "input_volume");
    } else if (strcmp(argv[1], "adcvol") == 0 && argc == 3) {
        unsigned int v = (unsigned int)strtoul(argv[2], 0, 0);
        if (v > 0x7e) { fprintf(stderr, "adcvol must be 0-126 (127=mute)\n"); rc = 2; goto out; }
        rc = set_adc_vol(v, "adc_volume");
    } else if (argc == 2) {
        unsigned int m = (unsigned int)strtoul(argv[1], 0, 0);
        if (m > 2) { fprintf(stderr, "mixer must be 0, 1 or 2\n"); rc = 2; goto out; }
        rc = set_u32(OT_ACODEC_SET_MIXER_MIC, m, "mixer input");
    } else {
        fprintf(stderr, "unknown arguments\n");
        rc = 2;
    }
out:
    close(fd);
    return rc;
}
