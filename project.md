# Almanac & Stone
## Master Project Document

---

## 1. Concept

A screen-free, single-purpose ambient hardware object: a wide, low triangular-wedge enclosure housing a 7.5-inch e-ink display that refreshes once a day with a personalized "page" — an astronomy-grounded almanac reading generated from the owner's birth date, time, and location.

**Core positioning:** No app. No login. No Bluetooth pairing. No subscription. No notifications. No settings menu. One page, once a day, forever. It is explicitly a *digital-sabbath object* — the value isn't the astrology content itself, it's removing the phone from a daily ritual moment. This "subtraction" framing is the strongest brand asset, stronger than the zodiac content itself, because it appeals even to people (like the founder) who don't believe in astrology.

**Working name:** **Almanac & Stone** (alternatives below).

---

## 2. Market Rationale

| Signal | Data |
|---|---|
| Astrology app market size | $5.7–8.3B in 2026, ~20%+ CAGR; Co-Star monetizes successfully on personalization [cite:98][cite:99] |
| Devotional/Bible daily ritual (fast-follow SKU) | 100M+ MAU on YouVersion, 14–22M daily engaged users, growing double-digit YoY; a screen-free "wearable cross" already crowdfunded successfully [cite:117][cite:120][cite:123] |
| Financial ticker (secondary SKU) | Existing commercial e-paper tickers (TickrMeter) and an Etsy category already prove demand [cite:112][cite:115] |
| Digital minimalism / "no app" appeal | 53% of Americans want to reduce phone use; "dumbphone"/digital-minimalism trend (Analog 2026) is a fast-growing category driven by notification fatigue [cite:176][cite:179][cite:180] |
| AI-hardware backlash | Humane AI Pin failed on slow responses, subscription stacking, and unclear value over a phone — the cautionary tale this product deliberately avoids [cite:12][cite:8][cite:4] |
| Notetaker category complaints | Reddit users actively hunt for "no subscription" alternatives to Plaud/Limitless — proof that subscription fatigue is a real purchase blocker [cite:181][cite:178][cite:186] |
| Crystal/gemstone market (stone accessory) | Healing crystal market ~$1.2–1.8B in the US; personalized/intention-based crystal products carry 35–55% price premiums; ~42% of Americans believe objects can hold spiritual energy [cite:198][cite:199][cite:201] |

**Top 3 content niches identified (in priority order):**
1. **Astrology/personal almanac** — largest willing-to-pay audience, strongest case for screen-free upgrade over apps.
2. **Bible/devotional** — largest daily-active audience of any niche, proven hardware-gifting behavior.
3. **DnD/tabletop companion** — smaller market, highest engagement intensity, strong prop/collectible appeal, aligns with founder's own hobby.

---

## 3. Physical Design

Evolved through several iterations: book-shaped grimoire → triangular wedge ("standing on one rectangle face, like a sleek clock") → final wide, low wedge with panoramic screen and gemstone accent.

- **Form:** Elongated triangular prism standing on its long rectangular base, sloped front face carrying the display.
- **Dimensions:** ~210 mm wide × 108 mm tall × 78 mm deep. Slope ~55° from the desk.
- **Materials:** Matte obsidian-black anodized-look shell, thin gold trim lip, fine gold constellation engravings.
- **Display:** 7.5" monochrome e-ink, 800×480, panoramic/edge-to-edge with narrow gold bezel.
- **Controls:** One polished brass "seal" button on the top ridge (reveal/advance content) — deliberately ceremonial, not a settings interface. No visible pairing button, no status LEDs, no "connecting..." spinner.
- **Gemstone accent:** A small, visible, naturally-cut crystal (amethyst standard; rose quartz and citrine as swappable upsell variants) set into the top ridge beside the seal button, softly lit by a warm LED. Framed honestly in fine print as decorative, marketed evocatively ("a moonstone sits at its heart"). Swappable stones become a $10–15 upsell SKU; a "Founder's Edition" with a real gemstone vs. a lower-cost polished glass/resin version creates a natural two-tier product line.
- **USB-C** hidden on the rear for charging only.

### Enclosure schematic (3D printing)

| Part | Size (mm) | Role |
|---|---|---|
| Front bezel | 210 × 125 × 8 | Holds the panel; gold-painted lip |
| Rear wedge | 210 × 78 × 108 | Main body, button bosses, HAT standoffs |
| Base plate | 210 × 78 × 6 | Battery tray, USB-C cutout, rubber feet |

PETG, 0.2mm layers, 4 walls, 25% gyroid infill, M2.5 heat-set inserts to join bezel to rear, M3 screws for base.

---

## 4. Electronics & Firmware

**Fully offline architecture** — no internet dependency after initial factory programming.

```
USB-C 5V → charge IC (TP4056/IP5306) → 1S LiPo 2500mAh
                                              │
                                        3.3V regulator
                                              │
                    ┌─────────────────────────┼─────────────────────┐
                    │                         │                     │
              ESP32-S3 (MCU)          e-paper driver HAT       DS3231 RTC
                    │                  (UC8179, SPI)          (alarm wake)
                    │                         │
              3× buttons              7.5" 800×480 e-ink panel
           (or 1 ceremonial seal)
```

- **MCU:** ESP32-S3 — no ongoing Wi-Fi needed; astronomical calculations run entirely on-device.
- **Storage:** 16MB flash (~€10–25) holds the ephemeris/algorithm library, hundreds of hand-written phrase fragments, and a bitmap font renderer — all locally, no image downloads needed [cite:143][cite:153][cite:156].
- **Astronomy engine:** Meeus-style algorithms compute moon phase, moon sign, planetary retrogrades, solstices/equinoxes from date math alone — no ephemeris files or network calls required [cite:147][cite:152][cite:151].
- **Personalization at order time:** Customer enters name, birth date/time/place on a web form at checkout. Place is geocoded once, server-side (the only moment the system ever touches the internet). Each unit is individually flashed with this config at final assembly — "built once, for you, forever."
- **No app for daily use.** Optional future update path: one-time USB-C connection to a web page to change themes/correct birth data, rather than a persistent Bluetooth link — preserves the "it just works" promise while still allowing occasional edits.
- **Battery life:** 4–8 weeks per charge (one full e-ink refresh/day, RTC-driven wake).

### Content generation engine (design brief)
A deterministic, offline-capable Python module (`oracle_generator`) combining:
1. Real astronomical calculations (moon phase, moon sign, retrogrades, seasonal markers).
2. A large (150+) hand-written fragment bank tagged by moon phase, planetary emphasis, season, and tone — assembled into 40–70 word daily messages, seeded deterministically by `(date, user birth data)` so output is varied but reproducible and never repeats within a 30-day rolling window.
3. A hard-coded `content_safety_check()` guardrail blocking absolute predictive language ("will happen," "guaranteed," medical/financial/legal claims) before any message reaches the screen — this is both a writing-quality control and the primary legal-compliance mechanism (see Section 6).

Full engineering prompt for this module has been produced separately (`oracle-content-generator-prompt.md`).

---

## 5. Bill of Materials & Manufacturing Cost

### Prototype (unit 1)

| Item | Spec | Cost |
|---|---|---|
| 7.5" e-ink panel + driver HAT | 800×480, UC8179 | €65–110 |
| ESP32-S3 DevKit | USB-C, Wi-Fi | €8–15 |
| LiPo 2500mAh + PCM | 1S | €8–12 |
| USB-C charge board | TP4056/IP5306 | €3–6 |
| DS3231 RTC | — | €2–4 |
| Buttons, magnets, inserts | — | €6–10 |
| PETG filament (~180g) | — | €4–6 |
| Finishing (paint, gold foil, feet) | — | €10–20 |
| Gemstone accent (amethyst) | small polished stone + LED mount | €5–15 |
| **Total, unit 1** | | **€111–198** |

### At scale (injection-molded enclosure)

| Volume | Tooling (one-time) | Enclosure cost/unit |
|---|---|---|
| 200–500 units (crowdfunding batch) | $2,500–$14,000 | $5–$10 |
| 1,000–5,000 units | $3,000–$25,000 | $1.50–$4.00 |
| 10,000+ units | $12,000–$50,000+ | $0.80–$1.75 |

**Landed unit cost at 1,000–5,000 units: ~€55–75** (including panel, electronics, assembly), supporting a **€149–189 retail price** (+€10–15 for gemstone variants).

### Certification (hardware-only cost line)
Using pre-certified Wi-Fi/BLE modules and certified battery packs: FCC ~$3,000–$10,000, CE self-certification lab testing ~$1,500–$3,500 — avoid custom RF design to stay at the low end [cite:163][cite:169][cite:165].

---

## 6. Legal & Risk Considerations

| Risk area | Assessment | Mitigation |
|---|---|---|
| "Fortune telling" consumer-protection laws (several US states restrict paid predictive claims) [cite:166][cite:160] | Low risk if content avoids certainty language | Label "for reflection and entertainment purposes only"; content-generator safety filter blocks absolute predictive phrasing by design |
| GDPR / birth data privacy | Low risk — birth date/time/place is ordinary personal data, not GDPR "special category" data [cite:158][cite:159] | Market the offline architecture as a genuine privacy feature: "your birth chart lives only on your device, never on a server" |
| Trademark: "Almanac" | The word itself is generic and unprotectable; "The Old Farmer's Almanac" is a specific registered mark to avoid confusion with [cite:195][cite:190][cite:193] | Use "Almanac" generically but ensure the full brand name is distinct; run a proper USPTO + EU trademark search before finalizing |
| Product safety certification | Manageable, budgeted cost (see Section 5) | Use pre-certified wireless modules and battery packs |
| Stone/crystal marketing claims | Low risk if framed as decorative, not medicinal | Fine-print disclaimer ("decorative, does not affect device function") alongside evocative marketing copy |

---

## 7. Business Model & Go-to-Market

**Revenue streams:**
1. Hardware margin — €149–189 retail, ~€75–115 gross margin/unit.
2. Content subscription (year 2+) — €3–6/month for deeper personalization or additional content packs (astrology, devotional, DnD), mirroring Co-Star/YouVersion monetization [cite:98][cite:120].
3. Accessory SKUs — swappable gemstones (€10–15 each), interchangeable bezels, themed content packs.
4. B2B/gifting channels — faith organizations, wellness studios, metaphysical shops for the devotional SKU.

**Phased rollout:**
1. **Phase 0 (0–3 months):** 3–5 working prototypes, validate with 20–30 real users.
2. **Phase 1 (3–6 months):** Crowdfunding campaign (Kickstarter/Indiegogo), astrology as flagship SKU, €99–119 early-bird pricing.
3. **Phase 2 (6–12 months):** First production run (500–2,000 units, aluminum tooling), fulfill backers, open direct-to-consumer.
4. **Phase 3 (12+ months):** Launch subscription tier and second SKU (devotional or finance) on shared hardware platform.

---

## 8. Naming

"Almanac" is safe to use generically (unprotectable common word); avoid confusion with the specific "Old Farmer's Almanac" registered marks.

| Name | Notes |
|---|---|
| **Almanac & Stone** *(current pick)* | Names both core differentiators (daily page + gemstone) in one phrase; distinct enough for trademark clearance |
| The Quiet Almanac | Strongest tie-in to the "no app, no noise" positioning |
| The Sealed Almanac | References the brass seal-button ritual |
| The Wedge Almanac | Literal nod to the enclosure shape |
| Oraculum / Astra / The Vespera | Earlier-round alternatives not tied to "Almanac" |

**Next step:** Run a formal USPTO + EU IP office trademark search on "Almanac & Stone" and "The Quiet Almanac" before committing.

---

## 9. Visual References Generated

- Triangular wedge concept renders (obsidian black + gold, sci-fi monolith and celestial-instrument directions).
- Wide panoramic-screen version with full sample daily reading ("Jupiter Ascending...") rendered on-screen.
- Final concept render: **Almanac & Stone** with integrated amethyst crystal accent, warm LED glow, brass seal button, panoramic e-ink display.
- Exploded 3D-print schematic showing bezel/rear-wedge/base-plate assembly with labeled electronics stack.

---

## 10. Open Questions / Next Steps

1. Build first breadboard prototype (Waveshare/Good Display 7.5" panel + ESP32-S3) and validate wiring/refresh cycle.
2. Finalize CAD for the three-part enclosure and print first PETG shell.
3. Build the `oracle_generator` content module (astronomy layer + fragment bank + safety filter) and pilot-test output quality with real users, believers and skeptics alike.
4. Source gemstone supplier for amethyst/rose quartz/citrine variants at prototype quantities.
5. Run trademark search on finalist names.
6. Draft crowdfunding page copy explicitly positioned against phone-based astrology apps and against the AI-hardware subscription-fatigue backlash ("no app, no login, no subscription — just built for you, once, forever").
