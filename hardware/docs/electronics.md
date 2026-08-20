# Almanac Stone — electronics & wiring (prototype)

## The panel you're ordering

**Waveshare 10.85" e-Paper HAT+ (G)** — 1360×480, 4-colour (Red/Yellow/Black/White),
SPI, with driver HAT (onboard voltage translator, 3.3V/5V MCU compatible).

- Panel outline: 270.56 × 105.92 × 1.20 mm · active area 259.76 × 91.68 mm
- Full refresh ~21 s (fast ~12 s) — normal for large colour e-ink; fine for a
  once-a-day almanac, not for animation.
- Refresh power < 70 mW; standby ~0 (holds image with no power).

> **Framebuffer note:** 4 colours = 2 bits/pixel → 1360×480×2/8 ≈ **160 KB**.
> That does not fit comfortably in the ESP32's internal RAM alongside Wi-Fi/BLE
> stacks. **Use an ESP32 with PSRAM** and allocate the framebuffer in PSRAM.

## Microcontroller

**ESP32 with PSRAM** — recommended board: **ESP32-WROVER** dev board
(e.g. *ESP32-WROVER-E DevKit*, 4 MB flash + **8 MB PSRAM**) or an
**ESP32-S3 DevKitC-1 (N8R8, 8 MB PSRAM)**.

Why PSRAM: the ~160 KB colour framebuffer (and any working/scratch buffers)
lives in SPIRAM, leaving internal SRAM for the app.

## Wiring — panel HAT → ESP32 (standard Waveshare 8-wire SPI)

The G-series HAT uses Waveshare's standard e-paper SPI pinout. Connect:

| HAT pin | Meaning            | ESP32-WROVER GPIO | Notes |
|---------|--------------------|-------------------|-------|
| VCC     | Power              | 3V3               | HAT translates 3.3/5V; 3V3 is fine |
| GND     | Ground             | GND               | common ground |
| DIN     | SPI MOSI           | GPIO 23           | VSPI MOSI |
| CLK     | SPI SCLK           | GPIO 18           | VSPI CLK |
| CS      | Chip select        | GPIO 5            | active low |
| DC      | Data/Command       | GPIO 17           | |
| RST     | Reset              | GPIO 16           | |
| BUSY    | Busy (input)       | GPIO 4            | HIGH/LOW while refreshing |
| PWR     | Panel power enable | GPIO 2            | some HAT revisions; tie high if absent |

> Confirm the exact silk-screen labels and the PWR pin against the Waveshare
> wiki for **10.85inch e-Paper HAT+ (G)** when the board arrives — Waveshare
> occasionally renames PWR/BUSY across revisions. The GPIOs above are a safe,
> conflict-free default for a WROVER (avoids the PSRAM/flash pins 6–11 and the
> input-only 34–39).

### Power for the prototype
- Bench: USB from the ESP32 dev board powers both. That's enough to bring it up.
- Portable: a 3.7V LiPo + a charge/boost board (e.g. TP4056 + MT3608 to 5V, or a
  LiPo "power bank" module). E-ink draws almost nothing between daily refreshes,
  so a small cell lasts weeks. Add a **DS3231 RTC** so it knows the date without
  Wi-Fi.

## Firmware status (important)

Our current firmware renders **1-bit black/white** via GxEPD2. This 4-colour
G-panel is **not** a stock GxEPD2 model — for the prototype:

1. **Fastest path:** flash **Waveshare's own `EPD_10in85g` Arduino example**
   first, confirm the panel lights and refreshes with their demo image. This
   validates wiring before touching our code.
2. **Then integrate our reading:** render our framebuffer as black-on-white and
   push it through Waveshare's `EPD_10in85g_Display(buffer)` call. Map our
   1-bpp buffer to their 2-bpp (black + white only) format — a thin adapter,
   ~30 lines. Colour (accent lines in red/yellow) can come later.

> The `firmware/esp32/oracle_esp32.ino` GxEPD2 wrapper is written for a mono
> panel and uses a placeholder class. Treat it as the structure; the G-panel
> driver call comes from Waveshare's library, not GxEPD2.

## Bring-up checklist
- [ ] Waveshare demo image displays (wiring correct)
- [ ] ESP32 allocates 160 KB framebuffer in PSRAM (`heap_caps_malloc(..., MALLOC_CAP_SPIRAM)`)
- [ ] Our reading renders as black-on-white through the Waveshare display call
- [ ] RTC keeps the date across a deep-sleep cycle
- [ ] One full daily cycle: wake → render → sleep, measured power draw
