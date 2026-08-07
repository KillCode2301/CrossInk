---
title: Simulator
nav_order: 15
---

# Development Device Simulator

CrossInk can run in the [CrossPoint simulator](https://github.com/uxjulia/crosspoint-simulator), which renders the e-ink display in an SDL2 window. Use it for quick sanity checks without flashing firmware every time.

## Platform Support

The simulator is currently configured for macOS on Apple Silicon.

The `platformio.ini` `[env:simulator]` section contains hardcoded `-arch arm64` and Homebrew paths under `/opt/homebrew`.

- Intel Mac users need to remove `-arch arm64` and change Homebrew paths to `/usr/local`.
- Linux requires similar path changes plus a replacement for `lib/simulator_mock/src/MD5Builder.h`, which uses the macOS-only `CommonCrypto` API.
- Native Windows is not supported. Use WSL and follow the Linux adjustments.

## Prerequisites

```sh
# macOS
brew install sdl2

# Linux (Debian/Ubuntu)
sudo apt install libsdl2-dev
```

## Setup

Place EPUB books in `./fs_/books/` relative to the project root. That maps to the SD-card `/books/` path on device.

## Build And Run

```sh
pio run -e simulator
.pio/build/simulator/program
```

## Keyboard Controls

| Key | Action |
| --- | --- |
| Up / Down | Page back / forward (side buttons) |
| Left / Right | Left / right front buttons |
| Return | Confirm / Select |
| Escape | Back |
| P | Power |

## Cache Note

On first open of an EPUB, an **Indexing...** popup appears while the section cache is built in `.crosspoint/`.

If rendering looks stale after a code change, delete `./fs_/.crosspoint/` to clear simulator caches.

## Reading Stats Preview

The simulator has no DS3231 RTC, so monthly reading stats (days read this/last month, streaks, charts) are hidden by default — the same gate as X4 hardware without a clock.

To preview monthly reading stats in the simulator:

```sh
CROSSINK_SIMULATOR_FAKE_RTC=1 CROSSINK_SIMULATOR_SEED_STATS=1 pio run -e simulator -t exec
```

Or run the built binary directly:

```sh
CROSSINK_SIMULATOR_FAKE_RTC=1 CROSSINK_SIMULATOR_SEED_STATS=1 .pio/build/simulator/program
```

- `CROSSINK_SIMULATOR_FAKE_RTC=1` — uses the host system date/time as a stand-in RTC.
- `CROSSINK_SIMULATOR_SEED_STATS=1` — writes dummy `/.crosspoint/global_stats.bin` with reading days in the current and previous month.

Then open **Home → Reading Stats** (global All Books card). The third row should show **This Month**, **Last Month**, and **Longest Streak**.

For Minimal Stats sleep overlay preview, set the sleep screen to Minimal Stats in settings (or `crossink-settings.json`) and sleep while a book is open.
