# Changelog

All notable changes to this fork are documented here. Format loosely follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

This is [jj358mhz's fork](https://github.com/jj358mhz/SKU_RM0004) of
[UCTRONICS/SKU_RM0004](https://github.com/UCTRONICS/SKU_RM0004), created because upstream
has gone unmaintained. See `CLAUDE.md` for how this fork is consumed (a pinned commit,
deployed via Ansible).

## [Unreleased]

## [0.8.0] - 2026-09-28

### Removed
- [#8](https://github.com/jj358mhz/SKU_RM0004/issues/8) — `get_ip_address()`: dead code, fully
  superseded by `get_ip_address_new()` (the eth0→wlan0 fallback version, the only one actually
  called). Its `IPADDRESS_TYPE`/`ETH0_ADDRESS`/`WLAN0_ADDRESS` `#define`s were only referenced
  there, so those went too.

### Changed
- [#8](https://github.com/jj358mhz/SKU_RM0004/issues/8) — `get_cpu_memory()` now converts
  `/proc/meminfo`'s KB values to GiB via binary `/1024.0/1024.0` instead of decimal
  `/1000.0/1000.0`, matching the binary-GB convention already used by `get_sd_memory()`/
  `get_hard_disk_memory()`.

### Fixed
- [#8](https://github.com/jj358mhz/SKU_RM0004/issues/8) — cosmetic: `project/display.c`'s
  main loop had two consecutive `sleep(1)` calls instead of one `sleep(2)`; a leftover German
  comment (`// Fehler`) in `get_hard_disk_memory()` translated to English.

## [0.7.0] - 2026-09-28

### Fixed
- [#6](https://github.com/jj358mhz/SKU_RM0004/issues/6) — `lcd_draw_image()`: `col`/`row` were
  computed as `h - y`/`w - x` instead of just `h`/`w`, which would have transferred the wrong
  number of bytes (or underflowed if `y > h`/`x > w`) had this function ever been called —
  currently dead code, not called anywhere. Simplified to pass `w`/`h` directly, matching the
  convention already used by `lcd_fill_rectangle()`.
- [#7](https://github.com/jj358mhz/SKU_RM0004/issues/7) — `lcd_write_char()`: no bounds check
  on the character index into the font bitmap table (`font.data[(ch - 32) * font.height + i]`)
  — a non-printable/out-of-ASCII-range `ch` would read out of bounds. Now clamps anything
  outside the supported `32-126` range to a space before indexing.

## [0.6.0] - 2026-09-28

### Fixed
- [#5](https://github.com/jj358mhz/SKU_RM0004/issues/5) — `lcd_fill_rectangle()`: a `w == 0`
  call underflowed `w - 1` (a `uint16_t`) to `65535`, so a zero-width rectangle paradoxically
  filled up to the max width instead of being a no-op; now guarded with an early return. Also,
  the pixel buffer was a hardcoded `buff[320]`, only correct because `ST7735_WIDTH` currently
  happens to be `160` — `st7735.h` has commented-out alternate panel configs with different
  widths that would have silently stack-overflowed this buffer if ever switched to. Now sized
  from `ST7735_WIDTH * sizeof(uint16_t)` so it can't fall out of sync.

## [0.5.0] - 2026-09-28

### Fixed
- [#3](https://github.com/jj358mhz/SKU_RM0004/issues/3) — CPU load readings were corrupted
  for any load ≥ 10%: `get_cpu_message()` parsed `top`/`awk` output into buffers too small to
  hold it (`"12.34"` needs 6 bytes, `usCpuBuff`/`syCpubuff` were `[5]`), silently truncating
  via `fgets()`. Replaced the whole `popen("top ...")`/`awk` pipeline with a direct read of
  `/proc/stat` (two samples 200ms apart, delta of busy vs. total jiffies) — cheaper (no shell
  spawn per read) and structurally eliminates this class of bug rather than just widening a
  buffer.

## [0.4.0] - 2026-09-28

### Fixed
- [#4](https://github.com/jj358mhz/SKU_RM0004/issues/4) — `get_temperature()` could crash the
  service: `fopen("/sys/class/thermal/thermal_zone0/temp")`'s return value was never checked,
  so a missing thermal zone (different kernel/hardware) caused a NULL-pointer dereference in
  the following `fgets()`. Now returns `0` instead of dereferencing NULL when the file can't
  be opened.

## [0.3.0] - 2026-09-28

### Fixed
- [#14](https://github.com/jj358mhz/SKU_RM0004/issues/14) — RAM usage % on the LCD was
  overstated (e.g. ~34% shown vs ~12% real usage on a host with 8GB RAM and 1.9GB of disk
  cache). `get_cpu_memory()` now reads `MemAvailable` from `/proc/meminfo` instead of
  `MemFree` — `MemFree` doesn't count the kernel's disk cache/buffers, which Linux uses
  opportunistically for any unused RAM and reclaims instantly under pressure.

## [0.2.0] - 2026-09-28

### Fixed
- [#2](https://github.com/jj358mhz/SKU_RM0004/issues/2) — Disk usage % on the LCD was wrong
  (e.g. showing 53% on a host at ~22% real usage). `lcd_display_disk()` no longer adds
  `get_hard_disk_memory()`'s reading to `get_sd_memory()`'s — both hardcode `statfs`/`statvfs`
  on `"/"`, so combining them double-counted a single disk. The display now uses
  `get_sd_memory()` alone. Also: `get_sd_memory()` now checks `statfs()`'s return value instead
  of using uninitialized data on failure, and `get_hard_disk_memory()`'s output widened from
  `uint16_t` to `uint32_t` so it no longer overflows past 64GB (it's currently unused by any
  display, but is fixed for future reuse against a genuinely separate secondary disk mount).

## [0.1.0] - 2026-09-28

Baseline release for this fork: the disk-usage fix cherry-picked from upstream, plus a full
audit of the driver code with findings filed as issues for follow-up.

### Fixed
- Cherry-picked upstream [UCTRONICS/SKU_RM0004#47](https://github.com/UCTRONICS/SKU_RM0004/pull/47)
  (open, unmerged upstream): `get_hard_disk_memory()` now uses `statvfs("/")` instead of
  hardcoded `/dev/sda` `df`/`awk` parsing, so it reports real data on any root device name.
  Also fixes a missing `return` in that function. ([#1](https://github.com/jj358mhz/SKU_RM0004/pull/1))

### Known issues
A full audit of the C driver code (`hardware/`, `project/`) turned up several pre-existing
bugs, filed as GitHub issues:

- [#3](https://github.com/jj358mhz/SKU_RM0004/issues/3) — CPU load readings corrupted above ~10%
  (buffer too small for `popen`'s own output).
- [#4](https://github.com/jj358mhz/SKU_RM0004/issues/4) — `get_temperature()` can crash on an
  unchecked `fopen()`.
- [#5](https://github.com/jj358mhz/SKU_RM0004/issues/5) — `lcd_fill_rectangle()` zero-width
  underflow + magic buffer size not tied to `ST7735_WIDTH`.
- [#6](https://github.com/jj358mhz/SKU_RM0004/issues/6) — `lcd_draw_image()` incorrect
  width/height math (currently dead code).
- [#7](https://github.com/jj358mhz/SKU_RM0004/issues/7) — `lcd_write_char()` missing bounds
  check on character index.
- [#8](https://github.com/jj358mhz/SKU_RM0004/issues/8) — Cleanup: dead code, unit
  inconsistency, cosmetic issues.
- [#9](https://github.com/jj358mhz/SKU_RM0004/issues/9) — No CI build check exists yet.
- [#10](https://github.com/jj358mhz/SKU_RM0004/issues/10) — Display preferences are
  compile-time `#define`s rather than runtime-configurable.
