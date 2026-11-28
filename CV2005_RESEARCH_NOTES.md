## Vendor sensor.conf independently confirms I2C address 0x35 (2026-11-28)

<!-- CV2005_SENSOR_CONF_ADDR_CONFIRMED_20261128 -->

Evidence: vendor overlay file /overlay/custom/sensor.conf, read from the
live stock-firmware camera over SSH (camera at 192.168.1.111 at time of
read).

Full file contents:

```text
sensor=cv2005
snr_name=cv2005
snr_addr=107
snr_reg=12290
snr_data=5
snr_reg2=12291
snr_data2=32
```

Interpretation:

- snr_addr=107 decimal = 0x6B. On the I2C wire, 7-bit address 0x35
  appears as 0x6A for writes and 0x6B for reads (low bit selects
  direction). The vendor configuration therefore stores the 8-bit read
  form of the same device the bus scan found at 7-bit 0x35.
- snr_reg=12290 = 0x3002 and snr_reg2=12291 = 0x3003, with expected
  values 5 (0x05) and 32 (0x20). These are almost certainly the chip-ID
  registers the vendor firmware reads to identify the CV2005. They give
  the exact vendor probe sequence for sensor_get_chip-style detection.
- sensor=cv2005 and snr_name=cv2005 match ipcinfo --short-sensor output.

Three independent sources now agree on the sensor address:

1. Bus 0 scan found exactly one device, at 7-bit 0x35 (no 0x6a/0x6b
   device, as expected for 7-bit reporting).
2. Vendor sensor.conf uses snr_addr=107 (0x6B), the 8-bit form of 0x35.
3. ipcinfo reports the sensor as cv2005, matching the vendor config.

Address-form nuance for the driver:

The corrected driver uses the 8-bit write form 0x6A. The vendor config
stores 0x6B. Both map to 7-bit 0x35, but which form is correct depends
on how the sensor library's I2C helper forms the address (some HiSilicon
sensor libs take the base address and shift it themselves; others take
the 8-bit form directly). If driver reads ever fail with 0x6A, 0x6B is
the fallback to try. Record both forms.

Status: VENDOR-SIDE ADDRESS CONFIRMATION; runtime probe with the OpenIPC
driver remains pending.

## Vendor overlay backed up before flashing (2026-11-28)

<!-- CV2005_VENDOR_OVERLAY_BACKUP_20261128 -->

The vendor overlay was archived from the live camera and verified on the
PC before any further flashing:

- Camera archive: /tmp/vendor-overlay.tar.gz, 4816 bytes, gzip -t OK.
  (BusyBox tar on this image lacks -z; `tar caf` produced an uncompressed
  tar despite the .tgz name. The tar was renamed and compressed with gzip
  explicitly.)
- PC copy: /c/Users/User/Downloads/CV2005/vendor-overlay.tar.gz,
  4816 bytes, identical size; tar tzf lists the same entries.

Archive contents (16 entries):

```text
custom/
custom/config/
custom/config/ipcam.conf
custom/config/eth_network.conf
custom/config/user.conf
custom/config/hw.conf
custom/config/onvif_user.conf
custom/sensor.conf
eth_network.conf
hw.conf
ipcam.conf
mac.conf
onvif_user.conf
user.conf
xiaocao/
xiaocao/avsdk_backup
```

The archive may contain credentials (onvif_user.conf, user.conf,
eth_network.conf). Do not commit it to GitHub or paste its contents
except sensor.conf, which was reviewed and contains no secrets.

## Flash session status (2026-11-28)

<!-- CV2005_FLASH_SESSION_20261128 -->

Session parameters: TFTP server 192.168.1.3, camera 192.168.1.23 (U-Boot
session), netmask 255.255.255.0, ethaddr E8:B7:23:45:DF:D0. The camera's
Linux DHCP address has changed across the session (.158, .104, .111).

Rootfs release artifact (tag/path f027c188) was downloaded and checked:
- Extracted at 2 MiB: rootfs_4800k.bin, size 4,915,200 bytes.
- squashfs listing includes usr/lib/sensors/libsns_cv2005.so.
- SHA256: cc3579855f29c01728bb3dcf5e7565bd554d93b007e5ebea33b9cff791bc9216

U-Boot progress:
- rootfs loaded at 0x42000000; second load at 0x44000000 failed as
  reserved memory, so the second copy was loaded at 0x43000000.
- Both RAM copies compared equal:
  cmp.b 0x42000000 0x43000000 0x4B0000 -> "Total of 4915200 byte(s)
  were the same."

NOT YET RUN — flash erase/write/readback. Next U-Boot commands:

```text
sf probe 0
sf erase 0x2B0000 0x4B0000
sf write 0x42000000 0x2B0000 ${filesize}
sf read 0x43000000 0x2B0000 ${filesize}
cmp.b 0x42000000 0x43000000 0x12C000
```

Do not reset unless the write succeeds and the readback compare passes.
Then `reset`.

After boot, collect:

```text
dmesg | grep -iE 'wait idle abort|sensor|cv2005' | tail -20
i2cdetect -r -y 0
cat /proc/umap/vi | grep -E 'int_cnt|send_cnt'
ls -l /tmp/snap.jpg
```

## Build/workflow state (2026-11-28)

<!-- CV2005_BUILD_STATE_20261128 -->

- Branch cv2005-hi3516cv6xx commit 8f5df3d494ce30e29036f957262e9c900f9b4ef0
  purportedly combines full CV2005 driver sources with the corrected I2C
  address. Firmware pin update committed/pushed to ErezWeiss1/firmware,
  branch cv2005-hi3516cv6xx, commit 2245b332c.
- Workflow 46081322312 FAILED: cv2005_cmos.c:148 references undeclared
  CV2005_AGAIN_MAX. Suspected header typo (CV2005_GAIN_MAX in a branch);
  no corrective commit or workflow result recorded yet. This must be
  fixed and a green build obtained before the next flash candidate is
  trusted.
- The downloaded release f027c188 artifact (rootfs_4800k.bin above) implies
  a successful build for that release, but the exact workflow/commit
  status for it is not shown in the available evidence.

Scope: documentation only. No runtime probe, MIPI transfer, or camera
operation has been validated in this checkpoint.

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

## First GitHub cross-build and libc mismatch (2026-11-20)

<!-- CV2005_BUILD_LIBC_MISMATCH_20261120_RUN44789077830 -->

GitHub Actions run 44789077830 completed successfully for source commit
1b08be2a78789feb85da51fe49a8ddf69c10c3f3. Its workflow used
arm-linux-gnueabi-gcc from the gcc-arm-linux-gnueabi package.

Preserved artifacts:
- libsns_cv2005.so SHA256 92df43c1496851a4d9fa5db03a45f32856d3275e0b9e0713b49dc37bac45deb8
- libsns_cv2005.a SHA256 b61b7825007a05c4872f568baaa4f139be263d6ee3fc2210cd210513d854906b

The shared library was ELF32 little-endian ARM, Version5 EABI, soft-float
ABI. It required libc.so.6 and ld-linux.so.3, with GLIBC_2.4 and
GLIBC_2.34 version requirements.

The inspected destination majestic executable used PT_INTERP
/lib/ld-musl-arm.so.1 and required libc.so, with no shown GLIBC version
requirement. Matching ARM EABI float flags therefore did not establish
runtime compatibility.

Disposition: preserve the run-44789077830 artifacts as compile/link
evidence only. They are not approved deployment candidates. Do not install
or flash them based on build success alone.

## Corrections to earlier conclusions (2026-11-20)

ARM soft-float ELF flags alone do not fully verify destination-toolchain
compatibility. A successful CI build does not establish deployment
readiness, and static compatibility analysis is not exhausted.

The earlier ISP digital-gain conclusion was overstated. Majestic strings
alone do not establish that the relevant branch applies to CV2005, that
sensor digital gain must remain at 1x, or that the stock
digital-extension records should be disabled. isp.aGain must not be equated with ISP digital
gain. Numerical table agreement, callback semantics, and actual runtime
gain policy remain separate questions.

## Stock-camera boot interruption verified (2026-11-20)

<!-- BOOT_INTERRUPT_H_VERIFIED_20261120 -->

A live stock-camera UART session successfully interrupted autoboot by
sending uppercase H (0x48) every 20 ms while the camera powered on.

Observed interruption:

```text
Hit any key to stop autoboot:  0
# HHHHHHHHHH...
```

This confirms uppercase H for this camera and session. It does not prove
that H is the only accepted key, and Ctrl+H remains unverified.

The interrupt script is not an interactive terminal. After stopping it,
reconnect with:

```bash
sudo picocom -b 115200 --flow n /dev/serial0
```

Clear any residual H characters from the camera input line before issuing
commands. Do not rerun the repeater against an already interrupted prompt.

This establishes live UART boot interruption only. It does not establish
Linux shell access, CV2005 driver compilation, deployment, flashing,
sensor validation, or resolved initialization/ABI.

## Stock bootloader live inspection (2026-11-20)

<!-- STOCK_BOOTLOADER_VERIFIED_20261120 -->

Live U-Boot inspection commands were: version, help, printenv, bdinfo,
help bootm, help env, help getinfo, getinfo bootmode, getinfo spi, and
getinfo version.

Identified bootloader and build toolchain:

```text
U-Boot 2022.07 (Dec 17 2025 - 15:43:33 +0800)hi3516cv608-debug
arm-v01c02-linux-musleabi-gcc
(musl-1.2.3 linux-5.10 CS71.2.10.5.B002 2025-03-05 12:00:00) 10.3.0
GNU ld (GNU Binutils) 2.41
```

This identifies the bootloader build toolchain. It does not independently
establish the float ABI of stock userspace, the sensor module, the H44
executable, or the OpenIPC destination.

Key environment:

```text
arch=arm
board=hi3516cv608
bootargs=mem=40M earlycon=pl011,0x11040000 console=ttyAMA0,115200 clk_ignore_unused initcall_debug root=/dev/mtdblock2 rootfstype=squashfs mtdparts=sfc:192K(uboot),2560K(kernel),4992K(appfs),384K(config),64K(data)
bootcmd=bootupdate 3;sf probe 0;sf read 0x41000000 0x30000 0x280000;bootm 0x41000000
bootdelay=1
cpu=armv7
product=P23H
soc=hi3516cv608
```

The configured bootcmd invokes bootupdate 3 before SPI probe, read, and
boot. Its implementation and exact behavior have not been audited. Do not
invoke boot or run bootcmd merely for inspection.

Board information reported 64 MiB physical DRAM, while the kernel bootargs
specify mem=40M. Leave bootargs unchanged. Zero flash fields in bdinfo do
not mean that SPI flash is absent.

Boot medium and flash:

```text
getinfo bootmode: spi
getinfo spi: Block:64KB Chip:8MB*1 ID:0x68 0x40 0x17 Name:"BY25Q64ES"
getinfo version: U-Boot 2022.07
```

The displayed help lists bootm, env, getinfo, and other commands. The
env help output does not list a save subcommand, and no saveenv command
appears in the displayed top-level help. Do not infer persistent
environment-save support from the initial "Loading Environment from
<NULL>... OK" message.

Only inspection and help commands were executed. No environment changes,
manual memory writes, flash writes, or custom-driver boots occurred.

## ABI verified: soft-float EABI5 (2026-11-20)

Evidence: pi_saved/openipc_sensor_check/usr/lib/sensors/libsns_cv2003.so
Command: readelf -h usr/lib/sensors/libsns_cv2003.so
Output: Flags: 0x5000200, Version5 EABI, soft-float ABI
Machine: ARM, 32-bit, little-endian
Type: DYN (shared object), dynamically linked, stripped

Conclusion: The inspected OpenIPC sensor library is soft-float EABI5 ARM
32-bit little-endian. This supports use of -mfloat-abi=soft or an
equivalent ABI-compatible toolchain configuration, subject to runtime and
SDK compatibility validation.

Status: VERIFIED FOR THE INSPECTED LIBRARY; destination-runtime and
CV2005 SDK compatibility remain unverified.

## ISP digital-gain policy (2026-11-20)

Evidence: analysis/cv2005_destination_abi_20261118_224107_168420/rootfs/usr/bin/majestic
Command: strings majestic | grep -i "dgain\|digital.gain\|isp.dgain\|sensor.dgain"

Majestic strings indicate that, for sensors without a separate digital-gain
control, sensor digital gain is pinned at 1x and ISP gain supplements it.
These strings alone do not establish CV2005-specific runtime behavior,
whether the relevant code path applies to CV2005, or whether stock
digital-extension records should be disabled.

Status: LEAD, NOT CV2005-RUNTIME-VERIFIED. Preserve the distinction
between ISP analog gain, ISP digital gain, and sensor digital gain.

## 1024-unity scaling, MIPI/ISP defaults (2026-11-20)

### 1024-unity scaling — stock-table supported
Evidence: stock_analog_gain_249.csv; 249 records match
(256*1024)/(256-code) with zero mismatches.
Conclusion: 1/1024 is supported as the unity scale for the recovered
combined-gain table. Runtime ISP gain policy remains separate.
Status: STOCK-TABLE SUPPORTED; runtime policy unverified.

### MIPI/ISP defaults — stock-verified, board-transfer pending
Evidence: stock boot log reported 465 Mbps MIPI and successful CV2005
initialization; cv2005_cfg.h carries the stock-derived initialization
table; cv2005_stock_isp_tables.h carries recovered ISP tables.
Conclusion: MIPI rate and ISP defaults are stock-derived, but destination
board behavior requires runtime validation.
Status: STOCK-DERIVED, HARDWARE-TRANSFER PENDING.

### Build blocker — removed
Evidence: ABI evidence, stock init/ISP tables, and successful GitHub
Actions run 44792107411.
Conclusion: The $(error ...) blocker was removed in dbace77. The preserved
musl-linked build is the current build baseline; runtime and hardware
validation remain pending.
Status: REMOVED — build attempt enabled; no hardware validation claimed.

## CV2005 destination callback-registration ABI verified (2026-11-20)

<!-- CV2005_DESTINATION_CALLBACK_REGISTRATION_ABI_20261120 -->

Evidence:
- Destination majestic: analysis/cv2005_destination_abi_20261118_224107_168420/rootfs/usr/bin/majestic
- Destination CV2003 sensor library: analysis/cv2005_destination_abi_20261118_224107_168420/rootfs/usr/lib/sensors/libsns_cv2003.so
- Built CV2005 library: analysis/github_cv2005_build_44792107411/artifacts/libsns_cv2005.so

Majestic exports all six MPI registration symbols required by CV2005:

```text
ot_mpi_isp_sensor_reg_callback
ot_mpi_isp_sensor_unreg_callback
ot_mpi_ae_sensor_reg_callback
ot_mpi_ae_sensor_unreg_callback
ot_mpi_awb_sensor_reg_callback
ot_mpi_awb_sensor_unreg_callback
```

It also exports memcpy_s, memset_s, and snprintf_s. The destination
CV2003 sensor library imports the same six MPI registration symbols and
the same secure-libc helpers. CV2005 imports the identical set.

The destination CV2003 module and built CV2005 module both export a
48-byte sensor object:

```text
CV2003: g_sns_cv2003_obj, size 48
CV2005: g_sns_cv2005_obj, size 48
```

Both objects populate callback pointers at offsets 0x00, 0x04, 0x08,
0x10, 0x14, 0x20, 0x24, 0x28, and 0x3c. All remaining fields are zero.
This supports the same otispsnsexpfunc callback-table layout.

Both modules expose cis_register_callback and cis_unregister_callback,
plus cis_i2c_init, cis_i2c_exit, cis_write_reg, cis_read_reg,
cis_init_attr, cis_wdr_range_check, and cis_delay_ms.

Disassembly confirmed an identical registration sequence:

```text
1. Validate pipe and registration arguments.
2. Lock the per-pipe mutex.
3. Allocate/zero sensor state on first registration.
4. Call ot_mpi_isp_sensor_reg_callback(dev, state_id, ISP callbacks).
5. Call ot_mpi_ae_sensor_reg_callback(dev, AE library, state_id, AE callbacks).
6. Call ot_mpi_awb_sensor_reg_callback(dev, AWB library, state_id, AWB callbacks).
```

Both implementations read the state identifier from sensor-state offset
0x1c and pass it in the same MPI argument positions.

Both registration structures use these offsets:

```text
0x00 AE algorithm-library pointer
0x04 AWB algorithm-library pointer
0x08 ISP callback table
0x38 AE callback table
0x70 AWB callback table
```

Conclusion: CV2005 is compatible with the inspected destination runtime
at the MPI symbol, registration-call-sequence, registration-structure,
ISP/AE/AWB callback-table placement, and 48-byte sensor-object layout
levels.

Scope: this establishes callback-registration ABI compatibility only.
It does not establish correct runtime behavior of individual CV2005
callbacks, initialization, exposure/gain policy, MIPI transfer, ISP
calibration transfer, or camera operation.

Status: VERIFIED — CALLBACK-REGISTRATION ABI; individual callback
semantics remain unverified.

## CV2005 stock eight-register SHS reconciliation applied (2026-11-20)

<!-- CV2005_STOCK_EIGHT_REGISTER_SHS_RECONCILIATION_20261120 -->

Evidence: cv2005_recovered_code.txt, stock CV2005 register-info callback
at 0x44d01c; local patched ot driver cv2005_cmos.c and cv2005_cmos.h.

The stock CV2005 register-info callback establishes:

```text
reg_num = 8
cfg2_valid_delay_max = 2
delay_frame_num = 2
```

Its queued-register payload is:

```text
0x3048 exposure low
0x3049 exposure middle
0x304A exposure high
0x3118 analog gain
0x311D digital gain high
0x311C digital gain low
0x301C frame length low
0x301D frame length high
```

There is no 0x301E register in the stock payload. Stock VMAX is therefore
a two-register, 16-bit frame-length value.

The stock callback compares each current value with the previous-frame
value and marks only changed registers as updated. It does not mark all
eight registers updated on every call.

The stock exposure path uses shutter-start semantics:

```text
SHS = frame_length - integration_time
```

The active ot CV2005 driver has been reconciled to this model:

```text
CV2005_SNS_REGS = 8
VMAX registers = 0x301C, 0x301D
0x301E removed from the payload
20-bit VMAX high-nibble write removed
SHS exposure conversion retained
Selective register update retained
cfg2_valid_delay_max = 2
delay_frame_num = 2
```

The prior nine-register/20-bit VMAX layout was upstream XMedia-specific,
not stock CV2005 behavior. The prior stage-2 note's nine-register and
0x301E statements are superseded for the active ot driver.

Scope: this reconciles the register-info payload and exposure-update
semantics to recovered stock evidence. It does not validate gain mapping,
digital-gain policy, MIPI transfer, ISP calibration transfer, or hardware
behavior.

Status: APPLIED — STOCK EIGHT-REGISTER SHS RECONCILIATION; runtime and
hardware validation remain pending.

## Latest eight-register CI build and downloaded artifact inspection (2026-11-20)

<!-- CV2005_CI_ARTIFACTS_RUN44821720695 -->

Source commit: eb5e0c3ce1c95ce6d23a23f57e5417f88a45d1bc
Commit title: Align CV2005 sync payload with stock eight-register layout
GitHub Actions run: 44821720695, attempt 1
Run URL: https://github.com/ErezWeiss1/openhisilicon/actions/runs/44821720695
Conclusion: completed / success

The artifacts were downloaded locally and both SHA256 values matched
the downloaded evidence/artifacts.sha256 manifest. Saved provenance
identified the expected source commit.

Artifacts:
- libsns_cv2005.so: 92308 bytes
  SHA256: 4a0bfa8a70bfe967a0e3d268186bb91a7170e90897819eb00234c3e2fb7f5917
- libsns_cv2005.a: 83590 bytes
  SHA256: 205f5dadbb0dc659815fce853299df62864af4ff9f4c35db6f2c94e4f65a0fd6

Toolchain: OpenIPC Hi3516CV100 musl SDK
Archive SHA256:
466683e215a493144ba87b3d57a733864b6db318aad847bf0fd0616dda39729e

Saved CI inspection reports:
- ELF32, little-endian ARM shared library, EABI5, soft-float ABI flags.
- ARM v5TEJ / Thumb-1 attributes.
- Declared NEEDED dependency: libc.so.
- No symbol-version information reported.
- Shared library retains debug information and is not stripped.
- Static archive members: cv2005_cmos.o, cv2005_sensor_ctrl.o,
  sensor_common.o.
- No warning: or error: lines found in the saved build log.

Source state:
- Eight queued sync registers, including two VMAX bytes at 0x301C/0x301D.
- Selective current/previous-data update flags.
- SHS conversion retained.
- FPS callback retains margin 10; default/slow-frame AE paths retain 5.
- Makefile blocker remains removed by historical commit dbace77.

This is the latest downloaded, manifest-verified build baseline for
the eight-register revision. Earlier build records and hashes are
historical and do not identify these binaries.

Scope and limitations:
- Compile/link and static artifact inspection only.
- The ELF ABI flags describe these artifacts; they do not independently
  prove complete destination-runtime or SDK structure compatibility.
- MPI/secure-C imports remain in the shared library. Prior destination
  symbol/registration evidence is separate from this build inspection.
- No binaries were executed during this download/inspection.
- No deployment, flashing, or CV2005 hardware validation occurred in
  this checkpoint.
- Sensor startup, exposure/gain/FPS behavior, MIPI acceptance, calibration
  transfer, image quality, and long-run stability remain unvalidated.

Documentation precedence:
This checkpoint supersedes older statements identifying run 44792107411
as the current build baseline. It does not upgrade earlier gain-policy,
MIPI-rate, initialization-table, or SDK-compatibility claims merely
because the source compiled.
