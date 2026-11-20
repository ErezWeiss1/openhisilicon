## CV2005 stage 2 incorporated and musl build verified (2026-11-20)

Stage 2 timing/layout policy is incorporated in commit ce4c261 and present
at current HEAD dbace77. The obsolete cv2005_stage2_patch.py must not be
run or applied again.

Verified source values:
- CV2005_VMAX_LINEAR = 1180
- CV2005_FULL_LINES_MAX_LINEAR = 14160
- CV2005_FPS_MIN_LINEAR = 2.5
- CV2005_FPS_MAX_LINEAR = 30
- VMAX registers = 0x301C, 0x301D, 0x301E
- CV2005_SNS_REGS = 9
- Frame extension disabled; full_lines follows selected frame length
- Invalid FPS rejected before timing calculation

GitHub Actions run 44792107411 completed successfully at source commit
a6c8bc0500a8d8020223f673b3e996c9dc474632. That commit changed only
.github/workflows/build-cv2005.yml; its CV2005 source is identical to
local commit dbace77.

Preserved artifacts:
- libsns_cv2005.so SHA256 bb300ed510358da774161ce5fa2da12190bfac740de713141e4bdd2fce7f5e64
- libsns_cv2005.a SHA256 990148375a08a2ca34d926864be0bb17c82239ca5fa7bbbedcd0956405f76ed0

Both hashes match evidence/artifacts.sha256, and sha256sum -c reports OK.
The build used the OpenIPC Hi3516CV100 musl toolchain and linked against libc.so.

Scope: compile, link, and static ARM/libc checks only. This does not
validate runtime initialization, gain or exposure behavior, MIPI transfer,
ISP compatibility, calibration transfer, or camera operation.

## ABI verified: soft-float EABI5 (2026-11-20)

Evidence: pi_saved/openipc_sensor_check/usr/lib/sensors/libsns_cv2003.so
Command: readelf -h usr/lib/sensors/libsns_cv2003.so
Output: Flags: 0x5000200, Version5 EABI, soft-float ABI
Machine: ARM, 32-bit, little-endian
Type: DYN (shared object), dynamically linked, stripped

Conclusion: The OpenIPC Hi3516CV6xx destination toolchain produces
soft-float EABI5 ARM 32-bit little-endian binaries. This is primary
evidence from a .so that ran on the actual target, not an inference
from H44. CV2005 driver must be compiled with -mfloat-abi=soft
(or equivalent) to link correctly.

Status: VERIFIED — supersedes earlier "destination ABI unverified" note.

## ISP digital-gain policy verified (2026-11-20)

Evidence: analysis/cv2005_destination_abi_20261118_224107_168420/rootfs/usr/bin/majestic
Command: strings majestic | grep -i "dgain\|digital.gain\|isp.dgain\|sensor.dgain"
Key strings:
- "isp.dGain cannot be set while isp.aeMode is manual: this sensor has
  no separate digital gain to hold, so it is pinned at 1x. Use
  isp.aGain, which does reach it"
- "Highest sensor digital gain... Unlike the other two gains this one is a
  ceiling and never a value. These sensors carry a single combined
  gain register, so there is no separate digital gain to hold at a figure...
  Use the analog gain when the exposure mode is manual."

Conclusion: The destination ISP (majestic) expects sensor digital gain
to be pinned at 1x and supplements via ISP gain (isp.aGain). The 224
digital-extension records in the stock table are NOT used by the
destination ISP path. The current cv2005_cmos.c approach (sensor dgain
fixed at 1x, ISP supplements) is correct for the destination.

Status: VERIFIED — supersedes earlier "ISP digital-gain policy
unvalidated" note in cv2005_cmos.c.

## 1024-unity scaling, MIPI/ISP defaults, build blocker (2026-11-20)

### 1024-unity scaling — VERIFIED
Evidence: stock_analog_gain_249.csv (249 records, zero mismatches against
(256*1024)/(256-code)); majestic binary confirms ISP supplements beyond
sensor analog max.
Conclusion: 1/1024 is the correct unity for the combined gain register,
validated by stock table, not merely an interpretation.
Status: VERIFIED — supersedes "1024-unity scaling is an interpretation"
in cv2005_cmos.c.

### MIPI/ISP defaults — stock-verified, board-transfer pending
Evidence: stock boot log (ran on destination) confirmed 465 Mbps MIPI
and successful init; cv2005_cfg.h carries the 30-pair stock init table;
cv2005_stock_isp_tables.h carries stock ISP tables.
Conclusion: MIPI rate and ISP defaults are stock-verified for the
destination. Board-behavior transfer (whether destination MIPI PHY/ISP
accepts these at runtime) requires build + hardware test; cannot be
resolved by more static analysis.
Status: STOCK-VERIFIED, HARDWARE-TRANSFER PENDING.

### Build blocker — removed
Evidence: ABI verified (soft-float EABI5), ISP digital-gain policy
verified (majestic), stock init table + ISP tables committed, and GitHub
Actions run 44792107411 successfully built the current CV2005 source.
Conclusion: The $(error ...) blocker was removed in dbace77 to enable
the build attempt. The preserved musl-linked build is the current build
baseline; runtime and hardware validation remain pending.
Status: REMOVED — build attempt enabled; no hardware validation claimed.
