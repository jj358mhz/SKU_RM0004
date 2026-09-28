# Changelog

All notable changes to this fork are documented here. Format loosely follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

This is [jj358mhz's fork](https://github.com/jj358mhz/SKU_RM0004) of
[UCTRONICS/SKU_RM0004](https://github.com/UCTRONICS/SKU_RM0004), created because upstream
has gone unmaintained. See `CLAUDE.md` for how this fork is consumed (a pinned commit,
deployed via Ansible).

## [Unreleased]

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
