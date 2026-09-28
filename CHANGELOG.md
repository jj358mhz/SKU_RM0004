# Changelog

All notable changes to this fork are documented here. Format loosely follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

This is [jj358mhz's fork](https://github.com/jj358mhz/SKU_RM0004) of
[UCTRONICS/SKU_RM0004](https://github.com/UCTRONICS/SKU_RM0004), created because upstream
has gone unmaintained. See `CLAUDE.md` for how this fork is consumed (a pinned commit,
deployed via Ansible).

## [Unreleased]

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

- [#2](https://github.com/jj358mhz/SKU_RM0004/issues/2) — Disk usage % is wrong: `lcd_display_disk()`
  double-counts the same root filesystem via `get_sd_memory()` + `get_hard_disk_memory()`,
  compounded by a `uint16_t` MB overflow past 64GB and an unchecked `statfs()` call.
  Note: cherry-picking #1 made this *more* visible in practice, since it made
  `get_hard_disk_memory()` return real (non-zero) numbers on hosts where it previously
  silently returned 0 (any root device not literally named `/dev/sda`).
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
