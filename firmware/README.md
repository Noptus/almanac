# The Oracle — ESP32 / e-ink firmware

C++ port of the Python `oracle_generator`, targeting a low-power **ESP32**
driving a **1360×480 1-bit e-ink** panel laid out as **5 columns × 1 row**.

## What's here

| File | Role | Runs on |
|------|------|---------|
| `src/oracle_astro.{h,cpp}` | Meeus-style astronomy (moon phase, signs, retrogrades, seasons, seed). Pure `<cmath>` + integer math. | desktop + ESP32 |
| `src/oracle_fragments.h` | 278-fragment bank, **auto-generated** from `oracle_generator/fragments.py`. | desktop + ESP32 |
| `src/oracle_composer.{h,cpp}` | Deterministic selection (xorshift64\*), `{name}` handling, safety filter, 30-day collision history. No heap. | desktop + ESP32 |
| `src/oracle_text.{h,cpp}` | **EB Garamond serif** via stb_truetype, antialiasing dithered to 1-bit. | desktop + ESP32 |
| `src/oracle_font_ttf.h` | Embedded ASCII-subset EB Garamond (Regular + SemiBold), **auto-generated**. ~40 KB flash. | desktop + ESP32 |
| `third_party/stb_truetype.h` | Public-domain single-header TrueType rasteriser. | desktop + ESP32 |
| `src/font5x7.h` | Legacy 5×7 bitmap font (kept for reference; no longer used in the layout). | — |
| `src/oracle_render.{h,cpp}` | 1bpp framebuffer, word-wrap, moon icon, 5-col layout, BMP export. | desktop + ESP32 |
| `main.cpp` | Desktop harness: prints the reading + writes a `.bmp` preview. | desktop |
| `esp32/oracle_esp32.ino` | Thin on-device wrapper: RTC → render → blit to GxEPD2 → deep-sleep 24h. | ESP32 |

The astronomy, fragments, composer, and renderer compile **unchanged** on both
platforms — the only board-specific code is the `.ino`.

## Build & preview on desktop

```bash
./firmware/build_desktop.sh          # regenerates fragments, compiles oracle_cpp
./oracle_cpp --birth 1990-06-15 --name Sam --place Lyon \
    --crystal amethyst --date 2026-08-20 --out reading.bmp
```

`reading.bmp` is exactly what the panel shows. The C++ output is
**byte-identical** to the Python engine for the same inputs (verified across
dates/users), because both share the same FNV-1a seed, xorshift64\* stream, and
selection order.

## Determinism / source of truth

`oracle_generator/fragments.py` is the single source of truth. Never hand-edit
`src/oracle_fragments.h` — run `python3 tools/gen_fragments_cpp.py` (the build
script does this automatically) so the two platforms can't drift.

## ESP32 notes

- **Framebuffer**: `1360 × 480 / 8 = 81,600 bytes` in `.bss`. Fits ESP32 SRAM
  (~320 KB). On tight boards, move `framebuf` to PSRAM (`heap_caps_malloc(...,
  MALLOC_CAP_SPIRAM)`).
- **Fonts**: the two ASCII-subset EB Garamond faces embed as ~40 KB of flash
  (`oracle_font_ttf.h`). stb_truetype rasterises glyphs on demand into small
  temporary bitmaps (freed immediately), so there's no glyph cache to size.
  Small UI text is hard-thresholded for crispness; large text (message, title)
  is ordered-dithered for smooth curves.
- **Panel class**: `oracle_esp32.ino` uses a *placeholder* GxEPD2 class name —
  replace `GxEPD2_1360x480_TEMPLATE` with the exact class for your controller
  (large panels are often tiled; use the matching multi-panel driver). Our
  buffer is MSB-first 1bpp (bit set = black), so `drawInvertedBitmap` blits it
  directly.
- **Power**: render once, then `esp_deep_sleep` for 24 h. E-ink holds the image
  at zero power between refreshes. The 30-day history lives in
  `RTC_DATA_ATTR` so it survives deep sleep.
- **Date**: read from a DS3231 RTC (or NTP once, then RTC). No `<chrono>` in the
  core keeps it portable and test-deterministic.

## Layout

Spare and vision-matched — no borders, no dividers. A left-aligned stack on
white:

```
   20 August                              ← date, large serif
   Moon in Sagittarius · Saturn retrograde ← moon + planet, one quiet line

   Half-lit and climbing: the moon is at    ← the phrase, generous serif,
   the point where effort meets resistance,   word-wrapped
   Sam. …
```
