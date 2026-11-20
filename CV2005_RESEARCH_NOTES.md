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
