#!/usr/bin/env python3
"""The Oracle -- interactive daily reading.

Asks for your birthdate, location, and crystal, then shows today's moon, the
planetary picture, and your daily message.

Usage:
    python3 oracle.py                      # interactive prompts
    python3 oracle.py --date 2026-12-21    # read a specific day
    python3 oracle.py --name Sam --birth 1990-06-15 --place "Lyon" \
        --crystal amethyst --date 2026-08-20   # fully non-interactive

Note on the crystal: the astronomical engine is real (moon phase, signs,
retrogrades are computed from Meeus-style formulas). The crystal is a personal
touch you choose -- it flavours the closing line but is kept clearly separate
from the astronomy, in keeping with the product's "reflection, not prediction"
stance.
"""

from __future__ import annotations

import argparse
import sys
from datetime import date, datetime

from oracle_generator import generate_daily_message, sky_state


# --------------------------------------------------------------------------
# Crystals -- a small curated set, each with a one-line intention. Purely a
# personal-ritual flavour; not derived from or claiming any astronomy.
# --------------------------------------------------------------------------

CRYSTALS = {
    "amethyst":   "a stone for calm and clear thinking",
    "rose quartz": "a stone for gentleness, toward others and yourself",
    "citrine":    "a stone for warmth and quiet confidence",
    "obsidian":   "a stone for grounding and honest reflection",
    "moonstone":  "a stone for intuition and new beginnings",
    "clear quartz": "a stone for focus and starting fresh",
    "tigers eye": "a stone for steadiness and courage",
    "jade":       "a stone for balance and patience",
    "carnelian":  "a stone for motivation and warmth",
    "lapis lazuli": "a stone for truth and clear speech",
}


PHASE_GLYPH = {
    "new": "●",              # ●
    "waxing_crescent": "☽",  # ☽
    "first_quarter": "◐",    # ◐
    "waxing_gibbous": "◐",
    "full": "○",             # ○
    "waning_gibbous": "◑",   # ◑
    "last_quarter": "◑",
    "waning_crescent": "☾",  # ☾
}

PLANET_SYMBOL = {
    "mercury": "☿",  # ☿
    "venus": "♀",    # ♀
    "mars": "♂",     # ♂
    "jupiter": "♃",  # ♃
    "saturn": "♄",   # ♄
}


# --------------------------------------------------------------------------
# Input parsing helpers
# --------------------------------------------------------------------------

_DATE_FORMATS = ("%Y-%m-%d", "%d/%m/%Y", "%m/%d/%Y", "%d-%m-%Y", "%Y/%m/%d")


def parse_date(text: str) -> date:
    text = text.strip()
    for fmt in _DATE_FORMATS:
        try:
            return datetime.strptime(text, fmt).date()
        except ValueError:
            continue
    raise ValueError(f"Could not read a date from {text!r}. Try YYYY-MM-DD.")


def prompt(label: str, *, default: str | None = None, required: bool = False) -> str:
    suffix = f" [{default}]" if default else ""
    while True:
        try:
            raw = input(f"{label}{suffix}: ").strip()
        except (EOFError, KeyboardInterrupt):
            print()
            sys.exit(130)
        if not raw and default is not None:
            return default
        if not raw and not required:
            return ""
        if raw:
            return raw
        print("  (this one's needed -- please enter a value)")


def prompt_date(label: str, *, required: bool) -> date | None:
    while True:
        raw = prompt(label, required=required)
        if not raw:
            return None
        try:
            return parse_date(raw)
        except ValueError as exc:
            print(f"  {exc}")


def resolve_crystal(raw: str | None) -> str | None:
    """Map a user input (name or 1-based number) to a crystal name.

    Falls back to the free-text value for an unrecognised name so a personal
    choice outside the curated list is still honoured.
    """
    if not raw:
        return None
    raw = raw.lower().strip()
    names = list(CRYSTALS)
    if raw.isdigit():
        idx = int(raw) - 1
        if 0 <= idx < len(names):
            return names[idx]
        return None  # out of range -> skip
    for name in names:
        if raw == name or raw in name:
            return name
    return raw  # unknown but accepted as free text


def prompt_crystal() -> str | None:
    print("\n  Choose a crystal (or press Enter to skip):")
    names = list(CRYSTALS)
    for i, name in enumerate(names, 1):
        print(f"    {i:2d}. {name.title():14s}-- {CRYSTALS[name]}")
    return resolve_crystal(prompt("  Crystal (name or number)"))


# --------------------------------------------------------------------------
# Rendering
# --------------------------------------------------------------------------

RULE = "─" * 60  # ─


def render(target: date, birth: date | None, name: str | None,
           place: str | None, crystal: str | None) -> str:
    st = sky_state(target, birth)
    lines = []

    lines.append(RULE)
    header = "  THE ORACLE"
    if name:
        header += f"  —  for {name}"
    lines.append(header)
    lines.append(f"  {target.strftime('%A, %d %B %Y')}"
                 + (f"  ·  {place}" if place else ""))
    lines.append(RULE)

    # --- Moon ---
    glyph = PHASE_GLYPH.get(st.moon.phase, "")
    phase_label = st.moon.phase.replace("_", " ").title()
    lines.append(f"  MOON   {glyph}  {phase_label}"
                 f"  ·  {st.moon.illumination * 100:.0f}% lit"
                 f"  ·  in {st.moon_sign}")

    # --- Planets ---
    p = st.planet
    sym = PLANET_SYMBOL.get(p.planet, "")
    state_word = "retrograde" if p.retrograde else "direct"
    lines.append(f"  SKY    {sym}  {p.planet.title()} is {state_word} today")

    # --- Season ---
    if st.season.marker in ("solstice", "equinox"):
        when = "today" if st.season.days_away == 0 else (
            f"in {abs(st.season.days_away)} day(s)"
            if st.season.days_away > 0 else
            f"{abs(st.season.days_away)} day(s) ago")
        lines.append(f"  SEASON ☀  {st.season.name.replace('_', ' ').title()} ({when})")

    # --- Your signs ---
    if st.sun_sign:
        lines.append(f"  YOU    ☉  Sun in {st.sun_sign}"
                     + (f"  ·  {crystal.title()} in hand" if crystal else ""))

    lines.append(RULE)

    # --- The message ---
    message = generate_daily_message(
        birth_date=birth,
        birth_time=None,
        birth_place=place,
        target_date=target,
        name=name,
    )

    # Wrap the message to ~56 columns for a tidy terminal / e-ink-ish column.
    lines.append("")
    for wrapped in _wrap(message, 56):
        lines.append(f"  {wrapped}")

    # Crystal flavour line -- kept explicitly separate from the astronomy.
    if crystal:
        note = CRYSTALS.get(crystal, "your chosen stone")
        lines.append("")
        lines.append(f"  Keep your {crystal} close today — {note}.")

    lines.append("")
    lines.append(RULE)
    return "\n".join(lines)


def _wrap(text: str, width: int):
    words = text.split()
    line = ""
    out = []
    for w in words:
        if line and len(line) + 1 + len(w) > width:
            out.append(line)
            line = w
        else:
            line = f"{line} {w}".strip()
    if line:
        out.append(line)
    return out


# --------------------------------------------------------------------------
# Main
# --------------------------------------------------------------------------

def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description="The Oracle -- daily reading")
    ap.add_argument("--name")
    ap.add_argument("--birth", help="birth date, e.g. 1990-06-15")
    ap.add_argument("--place", help="location, e.g. Lyon")
    ap.add_argument("--crystal", help="crystal name, e.g. amethyst")
    ap.add_argument("--date", help="target date (default: today)")
    args = ap.parse_args(argv)

    # Non-interactive if birth was supplied on the command line.
    interactive = args.birth is None

    if interactive:
        print()
        print("  Welcome. Let's read today's sky for you.")
        print("  (press Enter to skip anything optional)")
        print()
        name = prompt("  Your name", required=False) or None
        birth = prompt_date("  Your birthdate (YYYY-MM-DD)", required=True)
        place = prompt("  Your location") or None
        crystal = prompt_crystal()
        target = date.today()
        if args.date:
            target = parse_date(args.date)
    else:
        name = args.name
        birth = parse_date(args.birth)
        place = args.place
        crystal = resolve_crystal(args.crystal)
        target = parse_date(args.date) if args.date else date.today()

    print()
    print(render(target, birth, name, place, crystal))
    print()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
