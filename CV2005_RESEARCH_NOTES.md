
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
- "Highest sensor digital gain... Unlike the other two gains this one
  is a ceiling and never a value. These sensors carry a single combined
  gain register, so there is no separate digital gain to hold at a
  figure... Use the analog gain when the exposure mode is manual."

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

### Build blocker — recommendation to remove
Evidence: ABI verified (soft-float EABI5), ISP digital-gain policy
verified (majestic), stock init table + ISP tables committed.
Conclusion: Remaining question (board MIPI/ISP behavior) requires
build + hardware test. Static analysis is exhausted. Recommend removing
the $(error ...) blocker to allow a build attempt, pending user
authorization.
Status: RECOMMENDATION — remove blocker to enable build attempt.
