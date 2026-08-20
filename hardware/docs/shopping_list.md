# Almanac Stone — prototype shopping list (BOM)

Prices are rough single-unit estimates (USD) for a working prototype, not a
production run. You already have the software; this is everything else.

## Core electronics

| # | Item | Qty | ~Price | Notes |
|---|------|-----|--------|-------|
| 1 | **Waveshare 10.85" e-Paper HAT+ (G)** 1360×480, 4-colour + driver HAT | 1 | (you're ordering) | The screen. Comes with the SPI driver HAT. |
| 2 | **ESP32-WROVER dev board** (4 MB flash, **8 MB PSRAM**) | 1 | $10–15 | PSRAM is required for the 160 KB colour framebuffer. ESP32-S3 N8R8 also fine. |
| 3 | **DS3231 RTC module** (with CR2032) | 1 | $3–6 | Keeps the date offline across deep sleep. |
| 4 | Dupont jumper wires (F-F, assorted) | 1 pack | $4 | Panel HAT → ESP32 for bring-up. |
| 5 | Half-size breadboard (optional) | 1 | $4 | For first wiring test. |
| 6 | USB cable to match the ESP32 board | 1 | — | Power + flashing. |

## Power (portable, optional for first light)

| # | Item | Qty | ~Price | Notes |
|---|------|-----|--------|-------|
| 7 | 3.7V LiPo, ~1000–2000 mAh, with JST | 1 | $6–10 | E-ink sips power; lasts weeks between daily refreshes. |
| 8 | TP4056 USB-C charge board (protected) | 1 | $2 | Safe LiPo charging. |
| 9 | MT3608 boost (or a LiPo→5V module) | 1 | $2 | If your board needs 5V in. |
| 10 | Slide switch | 1 | $1 | Master power for the prototype. |

## The crystal & light

| # | Item | Qty | ~Price | Notes |
|---|------|-----|--------|-------|
| 11 | **Crystal point** to match the well (~24×58 mm) | 1+ | $4–12 | Amethyst, clear quartz, etc. Buy a couple of sizes; adjust `crystal_*` in the SCAD to fit. |
| 12 | Warm-white LED (3 mm/5 mm) or a single WS2812 | 1 | $1 | Lights the crystal from behind through the `led_hole`. |
| 13 | 220 Ω resistor (for a plain LED) | 1 | $0.10 | Skip if using a WS2812. |
| 14 | Small piece of frosted acrylic / diffuser film | — | $2 | Softens the glow behind the crystal. |

## Enclosure (3D printing)

| # | Item | Qty | ~Price | Notes |
|---|------|-----|--------|-------|
| 15 | **PLA or PETG filament**, dark/matte | ~250 g | $5 | PETG if it'll sit in sun/heat; PLA is easiest. |
| 16 | 4 mm dowels / smooth rod, ~40 mm × 3 | 3 | $1 | Align the printed left/right halves at the seam. |
| 17 | M3 screws, 8–10 mm, + heat-set inserts (M3) | ~6 | $3 | Base plate → body. Inserts optional but nicer. |
| 18 | Super glue or epoxy | 1 | $3 | Bond the two halves after fitting electronics. |
| 19 | Adhesive rubber feet | 4 | $1 | Non-slip base. |

## Print files (already generated, in `hardware/stl/`)
- `almanac_left.stl`, `almanac_right.stl` — body halves (join at the seam)
- `almanac_base.stl` — screwed-on bottom plate (electronics access + vents)
- `almanac_bezel.stl` — front frame that clips over the screen

## Rough total
Electronics + power + crystal + printing consumables ≈ **$60–100** for one
working prototype, on top of the panel you're already buying.

## Where to buy
- Panel/HAT/ESP32-WROVER/RTC: **Waveshare** (direct) or Amazon; ESP32/RTC also
  AliExpress (cheaper, slower).
- Crystals: any lapidary/etsy shop; measure and match the well.
- Filament/hardware: local or Amazon.
