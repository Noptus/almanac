# Almanac Stone — hardware (prototype)

Everything to build one working prototype around the **Waveshare 10.85"
e-Paper HAT+ (G)** (1360×480, 4-colour) and an **ESP32-WROVER** (PSRAM).

| Doc | What |
|-----|------|
| [docs/shopping_list.md](docs/shopping_list.md) | BOM — everything to buy (~$60–100 + the panel). |
| [docs/electronics.md](docs/electronics.md) | MCU choice, panel→ESP32 wiring, firmware notes. |
| [docs/blueprint.md](docs/blueprint.md) | Enclosure dimensions, print settings, assembly. |
| [scad/almanac_stone.scad](scad/almanac_stone.scad) | Parametric source (edit one line, re-render). |
| `stl/` | Printable parts: `left`, `right`, `base`, `bezel`. |

## Quick start
1. Order the panel + parts from `shopping_list.md`.
2. Wire and bring up the panel per `electronics.md` (Waveshare demo image first).
3. Print the four STLs (`blueprint.md` for settings/orientation).
4. Assemble; integrate our reading through the Waveshare display call.

## Requirements to regenerate STLs
OpenSCAD (`brew install --cask openscad`). Then see the snippet at the bottom of
`docs/blueprint.md`.

> The enclosure is a **prototype** proportioned to a printer bed, not the final
> product. See "Known limitation" in the blueprint about the crystal placement.
