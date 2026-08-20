# The Oracle — Roadmap to a legitimate, chart-driven almanac

Goal: readings that are **astrologically correct** (falsifiable sky math) and
**interpretively authoritative** (derived from a codified tradition), with
**no AI at generation time** — pure ephemeris math + correspondence tables.

Legend: `[ ]` todo · `[~]` partial/exists · `[x]` done

---

## Phase 0 — What exists today (baseline)
- [x] Meeus low-precision Sun/Moon longitudes (~0.3°), moon phase + illumination
- [x] Tropical sun sign; simplified moon sign
- [x] Hardcoded retrograde windows 2024–2030
- [x] Deterministic seed `(date, birthdate)`; 278-fragment template bank
- [x] Safety filter (no predictive/medical/financial/legal language)
- [x] C++/ESP32 port, EB Garamond serif render, GitHub Pages method page
- [~] Birth time / place collected in API but **ignored** (no chart yet)

## Phase 1 — Accuracy floor (make the FACTS pro-grade & personal)
- [ ] Adopt **Swiss Ephemeris** (or JPL DE440) as the position source
      - [ ] Resolve licensing: AGPL vs Astrodienst commercial (device = commercial)
- [ ] **Precompute an ephemeris table** (daily geocentric ecliptic longitudes,
      Sun..Pluto + true/mean node, ~1900–2100) → compact binary baked into flash
      - [ ] Interpolation (Chebyshev or cubic) for sub-day precision on device
- [ ] Birthplace → **geocode** to lat/long + IANA timezone (offline city table)
- [ ] Local birth time → **UT/Julian Day** with historical DST (tz database)
- [ ] **Ascendant** from sidereal time + obliquity + latitude
- [ ] **House cusps** (offer Whole-sign default + Placidus option)
- [ ] Validate every output against astro.com for a test panel of charts

## Phase 2 — Interpretive engine (make the MEANING traditional, no AI)
- [ ] **Correspondence tables**, each row citing a named source:
      - [ ] planet → domains / keywords / tone
      - [ ] sign → element, modality, ruling planet, keywords
      - [ ] house → life area
      - [ ] aspect → nature (harmonious/dynamic), orb, keyword
      - [ ] moon phase → cycle intention
      - [ ] stone → intention (crystal correspondence)
- [ ] **Transit detection**: today's planets vs natal → aspects within orb,
      ranked by weight (aspect tightness × planet significance)
- [ ] **Template grammar** keyed on (transiting planet, aspect, natal planet,
      house): fixed sentence skeletons filled from correspondence keywords
- [ ] Rewrite composer to build from the **top-ranked transit of the day**,
      not from today's sky alone (so charts genuinely differ)
- [ ] Keep deterministic seed for phrasing variety + 30-day no-repeat history
- [ ] Port tables + grammar to C++ (generated header, like fragments today)

## Phase 3 — Authority & provenance
- [ ] Cite sources for every table (Rudhyar, R. Hand *Planets in Transit*,
      Rider–Waite crystal correspondences, etc.) in an "About the method" card
- [ ] **Professional astrologer review** of tables + sample outputs (audit
      dignities, orbs, house logic); use as a marketing asset
- [ ] Optional: print the chart data on the device / order confirmation
      (e.g. "Sun 27° Gemini · Asc Libra · Moon □ Saturn today") to show the work
- [ ] Keep the "reflection, not prediction" framing (ethical + what serious
      astrologers themselves say)

## Phase 4 — Order & fulfilment pipeline
- [ ] Order form: name, birth date, birth time (+ "unknown"), birthplace,
      stone, tone/language
- [ ] Validation + geocoding at order time; store the computed natal chart once
- [ ] Per-device config blob (natal chart + preferences) flashed at manufacture
- [ ] Device: RTC/NTP date → daily transit → render → deep-sleep

## Cross-cutting
- [ ] Golden-file tests: chart + transit outputs pinned against astro.com
- [ ] Python ↔ C++ parity tests extended to the chart/transit layer
- [ ] Document the whole method publicly (GitHub Pages) — **done for the
      current design; update as Phases 1–2 land**
