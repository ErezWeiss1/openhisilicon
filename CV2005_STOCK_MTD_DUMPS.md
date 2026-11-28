# Stock flash partitions dumped and verified (2026-11-28)

<!-- CV2005_STOCK_MTD_DUMP_VERIFIED_20261128 -->

Read-only dumps of mtd0 (uboot), mtd1 (kernel), mtd3 (config) and mtd4
(data) were taken from the running camera over SSH (`cat /dev/mtdN`) and
verified twice: a second independent read compared byte-identical
(sha256sum -c, all OK), and copies to a second folder on the PC also
verified OK (three verified copies in total). The dump files are kept off
GitHub.

SHA-256 hashes:

- mtd0.bin (uboot):  caee59dcbbe3eda50d2e7c6c9d7021257775c41504c1d63be9d16a3c334fb5c7
- mtd1.bin (kernel): bd1a6e2703924685b75ba683345f4f6e36ace5b7e0edc113843b5f9644c26711
- mtd3.bin (config): 8fabc11735d75d6c8c4df990e3133a810083fcec72195e51ba02d13d0ef8e277
- mtd4.bin (data):   ed6a6776d272570a02e285172bd89858a6e07ce6d04a5edfaa44763d72b3903c

Findings:

- mtd3 (config, 384K) begins with the jffs2 magic (85 19 03 20) and is a
  valid jffs2 filesystem. OpenIPC would have reformatted this partition.
- mtd4 (data, 64K) is not erased: it holds 64 KB of high-entropy bytes
  with no readable text, including a repeated 8-byte pattern at offset
  0x30, which suggests encrypted or scrambled content rather than random
  noise. Inference (not confirmed): this is likely the per-device identity
  record (P2P ID, MAC, serial, "read auth ok", 211 bytes) that the stock
  boot log reads at startup. If so, it is unique to this camera and cannot
  be rebuilt from firmware.

Rule: treat mtd4.bin as irreplaceable. Never erase or write flash at
0x7F0000-0x800000, and do not point any OpenIPC overlay at the data
partition.

Scope: read-only dumps and documentation only. No flash write, erase,
partition, bootargs or image change was made in this checkpoint.
