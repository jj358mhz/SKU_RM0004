# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working in this repository.

## What this repo is

This is [jj358mhz's fork](https://github.com/jj358mhz/SKU_RM0004) of the vendor
[UCTRONICS/SKU_RM0004](https://github.com/UCTRONICS/SKU_RM0004) repo — the C driver for the
UCTRONICS front-panel LCD (ST7735 over I2C) used on the UCTRONICS Raspberry Pi rack chassis.
Upstream has gone unmaintained, so this fork exists to apply fixes/enhancements without
waiting on it. See `README.md` for the fork note and `CHANGELOG.md` for what's changed.

## How this repo is actually consumed

The `jeffjohnston/ansible-homelab` repo's `roles/uctronics_rack` role clones this repo and
pins it to an exact commit SHA via `uctronics_rack_lcd_version` in
`roles/uctronics_rack/defaults/main.yml`. That role:

- clones this repo to `/usr/local/src/SKU_RM0004` on target Pis (root-owned, detached HEAD at
  the pinned commit — **not** tracking `main`)
- runs `make` (the plain `Makefile` at the repo root, **not** `CMakeLists.txt`) to build the
  `display` binary
- deploys that binary to `/opt/SKU_RM0004/display` and runs it as `lcd_display.service`
  (a systemd unit, not the vendor's own `deployment_service.sh` / `uctronics-display.service`
  path described in `README.md` — that path is for standalone/non-Ansible use)

**Practical implication:** a change here does nothing on real hardware until someone bumps
`uctronics_rack_lcd_version` in `ansible-homelab` to the new commit and re-runs
`playbooks/uctronics-rack.yml`. Merging a PR in this repo is necessary but not sufficient.

Only `hardware/rpiInfo/`, `hardware/st7735/` (including `fonts.c`, which is bitmap data, not
logic), and `project/display.c` are exercised by that build. `python/` and `CMakeLists.txt`
(which build a separate shared library for Python bindings) are not used by the Ansible
deployment path and haven't been audited to the same depth.

## Build

```bash
make clean && make    # produces ./display
```

Requires `gcc` and `make` (installed by the ansible role via apt on the Pi itself; on a dev
machine, install them however's normal for that OS).

## Hardware requirements

The display is driven purely over I2C (`/dev/i2c-1`) — confirmed live on Pis running it with
SPI fully disabled. `get_temperature()` also reads `/sys/class/thermal/thermal_zone0/temp`
(Linux-specific, assumes a Raspberry Pi–style thermal zone).

## Contribution workflow

`main` is branch-protected — always work on a branch and open a PR, never push/commit
directly to `main` (matching the convention in `ansible-homelab`). There is currently no CI
(see [issue #9](https://github.com/jj358mhz/SKU_RM0004/issues/9)), so no required status
checks should be assumed to exist — don't rely on GitHub Actions catching anything here yet.

## Known issues

A full audit of the C driver code turned up a number of pre-existing bugs (some real,
some latent/dormant), filed as GitHub issues — see the "Known issues" section of
`CHANGELOG.md` for the current list with links. The three most impactful — wrong disk-usage
percentage ([#2](https://github.com/jj358mhz/SKU_RM0004/issues/2)), overstated RAM usage
percentage ([#14](https://github.com/jj358mhz/SKU_RM0004/issues/14)), and a crash on missing
thermal zone ([#4](https://github.com/jj358mhz/SKU_RM0004/issues/4)) — are fixed as of v0.2.0,
v0.3.0, and v0.4.0 respectively. Remaining open issues (#3, #5-#10) are lower-severity or
currently dormant.

## Keeping docs current

When landing a fix or enhancement here: update `CHANGELOG.md` (`[Unreleased]` section, or a
new version section if you're cutting one), and update `README.md` if user-facing
build/deploy steps change. Close/update the corresponding GitHub issue rather than leaving it
stale once its bug is fixed.
