/* Test helper: select the acodec microphone input (0=IN0, 1=IN1, 2=differential).
 * Issues OT_ACODEC_SET_MIXER_MIC (_IOWR('A', 2, u32)) on /dev/acodec.
 * Mixer register only; nothing is written to flash. Majestic resets it to IN1
 * when its audio pipeline restarts. */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define OT_ACODEC_SET_MIXER_MIC _IOWR('A', 2, unsigned int)

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s 0|1|2\n", argv[0]);
        return 2;
    }
    unsigned int mode = (unsigned int)strtoul(argv[1], 0, 0);
    if (mode > 2) {
        fprintf(stderr, "mode must be 0, 1 or 2\n");
        return 2;
    }
    int fd = open("/dev/acodec", O_RDWR);
    if (fd < 0) {
        perror("open /dev/acodec");
        return 1;
    }
    if (ioctl(fd, OT_ACODEC_SET_MIXER_MIC, &mode) < 0) {
        perror("ioctl");
        close(fd);
        return 1;
    }
    printf("mixer input set to %u\n", mode);
    close(fd);
    return 0;
}
