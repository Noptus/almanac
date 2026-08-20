#!/usr/bin/env python3
"""Manual review harness for oracle_generator.

Prints 20 sample messages across varied dates, moon phases, and users so the
writing quality and variety can be eyeballed. Also runs a handful of cheap
sanity checks (fragment counts, determinism, per-user difference, safety) and
prints a short PASS/FAIL summary.

Run:  python3 test_harness.py
"""

from __future__ import annotations

from datetime import date, timedelta

from oracle_generator import (
    MessageHistory,
    content_safety_check,
    generate_daily_message,
    sky_state,
)
from oracle_generator import fragments


# --------------------------------------------------------------------------
# Sample users and interesting dates
# --------------------------------------------------------------------------

USERS = [
    ("Sam", date(1990, 6, 15)),      # Gemini
    ("Iris", date(1985, 11, 3)),     # Scorpio
    ("Theo", date(2001, 2, 28)),     # Pisces
    ("Noa", date(1978, 8, 9)),       # Leo
    (None, date(1995, 12, 25)),      # no name -> placeholder-stripping path
]

# A spread of dates chosen to hit different phases, retrogrades, and seasons.
DATES = [
    date(2026, 1, 3),
    date(2026, 3, 20),    # near spring equinox
    date(2026, 3, 5),     # Mercury retrograde window
    date(2026, 6, 21),    # near summer solstice
    date(2026, 7, 4),     # Mercury retrograde window
    date(2026, 9, 23),    # near autumn equinox
    date(2026, 10, 15),   # Venus retrograde window
    date(2026, 11, 10),   # Mercury retrograde window
    date(2026, 12, 21),   # near winter solstice
    date(2027, 2, 14),    # Mars retrograde window
]


def print_samples() -> None:
    print("=" * 74)
    print("  THE ORACLE -- 20 sample daily messages")
    print("=" * 74)

    count = 0
    for i, target in enumerate(DATES):
        # Two different users per date -> 20 messages total.
        for name, birth in (USERS[i % len(USERS)], USERS[(i + 2) % len(USERS)]):
            count += 1
            st = sky_state(target, birth)
            msg = generate_daily_message(
                birth_date=birth,
                birth_time=None,
                birth_place=None,
                target_date=target,
                name=name,
            )
            label = name if name else "(no name)"
            phase = st.moon.phase.replace("_", " ")
            planet = st.planet.key.replace("_", " ")
            season = st.season.marker
            print()
            print(f"[{count:02d}] {target.isoformat()}  |  {label}")
            print(f"     sky: moon={phase} ({st.moon.illumination*100:.0f}%)"
                  f"  planet={planet}  season={season}"
                  f"  moon_sign={st.moon_sign}")
            print(f"     {msg}")
    print()


# --------------------------------------------------------------------------
# Cheap sanity checks
# --------------------------------------------------------------------------

def run_checks() -> bool:
    results = []

    def check(label, cond):
        results.append((label, bool(cond)))

    # 1. Fragment bank sizes meet the spec minimums.
    total_fragments = 0
    moon_ok = True
    for phase, frags in fragments.MOON_PHASE.items():
        total_fragments += len(frags)
        if len(frags) < 15:
            moon_ok = False
    check("moon phases: 8 keys", len(fragments.MOON_PHASE) == 8)
    check("moon phases: >=15 each", moon_ok)

    planet_ok = True
    for key, frags in fragments.PLANETARY_EMPHASIS.items():
        total_fragments += len(frags)
        if len(frags) < 10:
            planet_ok = False
    check("planetary states: 10 keys", len(fragments.PLANETARY_EMPHASIS) == 10)
    check("planetary states: >=10 each", planet_ok)

    for frags in fragments.SEASON_MARKER.values():
        total_fragments += len(frags)
    total_fragments += len(fragments.CLOSING_LINES)
    check("closing lines: >=20", len(fragments.CLOSING_LINES) >= 20)
    check("total fragments >= 150", total_fragments >= 150)

    # 2. Determinism: same (date, user) -> same message.
    a = generate_daily_message(date(1990, 6, 15), None, None, date(2026, 8, 20), "Sam")
    b = generate_daily_message(date(1990, 6, 15), None, None, date(2026, 8, 20), "Sam")
    check("deterministic for same user/date", a == b)

    # 3. Different users on the same day usually differ.
    m1 = generate_daily_message(date(1990, 6, 15), None, None, date(2026, 8, 20), "Sam")
    m2 = generate_daily_message(date(1985, 11, 3), None, None, date(2026, 8, 20), "Iris")
    check("different users differ same day", m1 != m2)

    # 4. No consecutive-day repeats for one user over 30 days (history on).
    hist = MessageHistory(max_days=30)
    prev = None
    repeats = 0
    for d in range(30):
        target = date(2026, 1, 1) + timedelta(days=d)
        msg = generate_daily_message(date(1990, 6, 15), None, None, target, "Sam", history=hist)
        if msg == prev:
            repeats += 1
        prev = msg
    check("no consecutive repeats over 30 days", repeats == 0)

    # 5. Every generated message this run is safe, and every raw fragment is safe.
    all_safe = True
    all_texts = (
        [t for lst in fragments.MOON_PHASE.values() for (t, _tone) in lst]
        + [t for lst in fragments.PLANETARY_EMPHASIS.values() for (t, _tone) in lst]
        + [t for lst in fragments.SEASON_MARKER.values() for (t, _tone) in lst]
        + [t for (t, _tone) in fragments.CLOSING_LINES]
    )
    for frag_text in all_texts:
        if not content_safety_check(frag_text.replace("{name}", "Sam")):
            all_safe = False
    check("all raw fragments pass safety", all_safe)

    # 6. Safety net actually rejects bad input.
    check("safety rejects 'you will die'", not content_safety_check("you will die soon"))
    check("safety rejects 'invest in'", not content_safety_check("invest in this stock"))
    check("safety rejects 'guaranteed'", not content_safety_check("success is guaranteed"))
    check("safety allows benign line", content_safety_check("A good day for straight talk."))

    # 7. Word count target band (soft) mostly respected.
    in_band = 0
    samples = 0
    for name, birth in USERS:
        for d in range(0, 40, 3):
            target = date(2026, 1, 1) + timedelta(days=d)
            msg = generate_daily_message(birth, None, None, target, name)
            samples += 1
            wc = len(msg.split())
            if 30 <= wc <= 80:
                in_band += 1
    check("word count in [30,80] for >=90% samples", in_band >= 0.9 * samples)

    # Report.
    print("=" * 74)
    print("  SANITY CHECKS")
    print("=" * 74)
    all_pass = True
    for label, ok in results:
        print(f"  [{'PASS' if ok else 'FAIL'}]  {label}")
        all_pass = all_pass and ok
    print()
    print(f"  total fragments in bank: {total_fragments}")
    print(f"  OVERALL: {'PASS' if all_pass else 'FAIL'}")
    print("=" * 74)
    return all_pass


if __name__ == "__main__":
    print_samples()
    ok = run_checks()
    raise SystemExit(0 if ok else 1)
