"""Astronomical calculation layer for The Oracle.

Pure Python standard library only (math + datetime). No network calls, no
third-party packages. The algorithms are Meeus-style low-precision formulas,
which are accurate to well within a degree for the Sun and a couple of degrees
for the Moon -- more than enough to name a moon phase, a zodiac sign, or a
season marker. Everything here is deliberately written so it can be ported to
C / MicroPython later: no comprehensions-as-cleverness, no exotic stdlib.

References:
  Jean Meeus, *Astronomical Algorithms*, 2nd ed. (chapters on Julian Day,
  solar coordinates, lunar position, phases of the Moon).
"""

from __future__ import annotations

import math
from dataclasses import dataclass
from datetime import date, datetime, timezone

# --------------------------------------------------------------------------
# Small angle helpers
# --------------------------------------------------------------------------

_DEG = math.pi / 180.0
_RAD = 180.0 / math.pi


def _norm360(x: float) -> float:
    """Normalise an angle in degrees to the range [0, 360)."""
    x = math.fmod(x, 360.0)
    if x < 0.0:
        x += 360.0
    return x


def _sin_d(deg: float) -> float:
    return math.sin(deg * _DEG)


def _cos_d(deg: float) -> float:
    return math.cos(deg * _DEG)


# --------------------------------------------------------------------------
# Julian Day
# --------------------------------------------------------------------------

def julian_day(dt: datetime) -> float:
    """Julian Day (including fractional day) for a datetime.

    If the datetime is naive it is treated as UTC. Valid for the Gregorian
    calendar (all dates this product will ever see).
    """
    if dt.tzinfo is None:
        dt = dt.replace(tzinfo=timezone.utc)
    dt = dt.astimezone(timezone.utc)

    year = dt.year
    month = dt.month
    day = (
        dt.day
        + (dt.hour + (dt.minute + dt.second / 60.0) / 60.0) / 24.0
    )

    if month <= 2:
        year -= 1
        month += 12

    a = year // 100
    b = 2 - a + a // 4
    jd = (
        math.floor(365.25 * (year + 4716))
        + math.floor(30.6001 * (month + 1))
        + day
        + b
        - 1524.5
    )
    return jd


def julian_centuries(jd: float) -> float:
    """Julian centuries since J2000.0."""
    return (jd - 2451545.0) / 36525.0


def _noon_datetime(d: date) -> datetime:
    """12:00 UTC on the given calendar date.

    We anchor day-level calculations at local-ish noon so the reported moon
    phase / sign is stable for the whole day rather than flipping at midnight.
    """
    return datetime(d.year, d.month, d.day, 12, 0, 0, tzinfo=timezone.utc)


# --------------------------------------------------------------------------
# Solar position (low precision, Meeus ch. 25)
# --------------------------------------------------------------------------

def sun_ecliptic_longitude(jd: float) -> float:
    """Apparent ecliptic longitude of the Sun, degrees [0, 360)."""
    t = julian_centuries(jd)

    # Geometric mean longitude and mean anomaly of the Sun.
    l0 = 280.46646 + 36000.76983 * t + 0.0003032 * t * t
    m = 357.52911 + 35999.05029 * t - 0.0001537 * t * t

    # Equation of the centre.
    c = (
        (1.914602 - 0.004817 * t - 0.000014 * t * t) * _sin_d(m)
        + (0.019993 - 0.000101 * t) * _sin_d(2 * m)
        + 0.000289 * _sin_d(3 * m)
    )

    true_long = l0 + c

    # Apparent longitude (nutation + aberration correction).
    omega = 125.04 - 1934.136 * t
    apparent = true_long - 0.00569 - 0.00478 * _sin_d(omega)
    return _norm360(apparent)


# --------------------------------------------------------------------------
# Lunar position (low precision, Meeus ch. 47 truncated)
# --------------------------------------------------------------------------

def moon_ecliptic_longitude(jd: float) -> float:
    """Apparent ecliptic longitude of the Moon, degrees [0, 360).

    Truncated series: keeps the largest periodic terms. Good to ~0.3 deg,
    which never changes which 30-degree zodiac sign the Moon sits in except
    for a few minutes near a boundary -- acceptable for this product.
    """
    t = julian_centuries(jd)

    # Fundamental arguments (degrees).
    lp = 218.3164477 + 481267.88123421 * t  # Moon's mean longitude
    d = 297.8501921 + 445267.1114034 * t  # mean elongation
    m = 357.5291092 + 35999.0502909 * t  # Sun's mean anomaly
    mp = 134.9633964 + 477198.8675055 * t  # Moon's mean anomaly
    f = 93.2720950 + 483202.0175233 * t  # Moon's argument of latitude

    lon = lp
    lon += 6.288774 * _sin_d(mp)
    lon += 1.274027 * _sin_d(2 * d - mp)
    lon += 0.658314 * _sin_d(2 * d)
    lon += 0.213618 * _sin_d(2 * mp)
    lon += -0.185116 * _sin_d(m)
    lon += -0.114332 * _sin_d(2 * f)
    lon += 0.058793 * _sin_d(2 * d - 2 * mp)
    lon += 0.057066 * _sin_d(2 * d - m - mp)
    lon += 0.053322 * _sin_d(2 * d + mp)
    lon += 0.045758 * _sin_d(2 * d - m)
    lon += -0.040923 * _sin_d(m - mp)
    lon += -0.034720 * _sin_d(d)
    lon += -0.030383 * _sin_d(m + mp)
    return _norm360(lon)


# --------------------------------------------------------------------------
# Moon phase & illumination
# --------------------------------------------------------------------------

PHASE_NAMES = (
    "new",
    "waxing_crescent",
    "first_quarter",
    "waxing_gibbous",
    "full",
    "waning_gibbous",
    "last_quarter",
    "waning_crescent",
)


@dataclass(frozen=True)
class MoonInfo:
    phase: str          # one of PHASE_NAMES
    illumination: float  # 0.0 .. 1.0
    elongation: float    # geocentric elongation in degrees, [0, 360)


def moon_phase(d: date) -> MoonInfo:
    """Moon phase name and illuminated fraction for a calendar date."""
    jd = julian_day(_noon_datetime(d))
    sun = sun_ecliptic_longitude(jd)
    moon = moon_ecliptic_longitude(jd)

    # Elongation of the Moon from the Sun, 0..360 increasing through the cycle.
    elong = _norm360(moon - sun)

    # Illuminated fraction from the phase angle.
    illum = (1.0 - _cos_d(elong)) / 2.0

    # Map the 360-degree cycle to eight named phases. Boundaries are the
    # 45-degree octants centred on the four principal phases.
    idx = int((elong + 22.5) // 45.0) % 8
    name = PHASE_NAMES[idx]
    return MoonInfo(phase=name, illumination=illum, elongation=elong)


# --------------------------------------------------------------------------
# Zodiac signs
# --------------------------------------------------------------------------

ZODIAC = (
    "Aries", "Taurus", "Gemini", "Cancer", "Leo", "Virgo",
    "Libra", "Scorpio", "Sagittarius", "Capricorn", "Aquarius", "Pisces",
)

# Standard tropical sun-sign date ranges (month, day) as the *start* of each
# sign. Order matters; Capricorn wraps the year end.
_SUN_SIGN_RANGES = (
    ((3, 21), (4, 19), "Aries"),
    ((4, 20), (5, 20), "Taurus"),
    ((5, 21), (6, 20), "Gemini"),
    ((6, 21), (7, 22), "Cancer"),
    ((7, 23), (8, 22), "Leo"),
    ((8, 23), (9, 22), "Virgo"),
    ((9, 23), (10, 22), "Libra"),
    ((10, 23), (11, 21), "Scorpio"),
    ((11, 22), (12, 21), "Sagittarius"),
    ((12, 22), (1, 19), "Capricorn"),
    ((1, 20), (2, 18), "Aquarius"),
    ((2, 19), (3, 20), "Pisces"),
)


def sun_sign(birth: date) -> str:
    """Tropical sun sign for a birth date using standard calendar ranges."""
    m, dd = birth.month, birth.day
    for (sm, sd), (em, ed), name in _SUN_SIGN_RANGES:
        if sm <= em:
            # Range inside a single calendar year.
            after_start = (m, dd) >= (sm, sd)
            before_end = (m, dd) <= (em, ed)
            if after_start and before_end:
                return name
        else:
            # Wrapping range (Capricorn): Dec 22 .. Jan 19.
            if (m, dd) >= (sm, sd) or (m, dd) <= (em, ed):
                return name
    return "Capricorn"  # defensive fallback; ranges above are exhaustive


def moon_sign(d: date) -> str:
    """Zodiac sign the Moon occupies on a date, from its ecliptic longitude."""
    jd = julian_day(_noon_datetime(d))
    lon = moon_ecliptic_longitude(jd)
    return ZODIAC[int(lon // 30.0) % 12]


# --------------------------------------------------------------------------
# Planetary emphasis (retrograde calendar)
# --------------------------------------------------------------------------

# Public, published retrograde windows (UTC dates). These are astronomical
# facts, not invented. Ranges are inclusive [start, end]. Coverage runs from
# 2024 through 2030 for the fast/near planets and is extended with the regular
# annual/near-annual cadence for the outer planets, which are in retrograde
# for a large, predictable fraction of every year.
#
# NOTE: dates are given to day precision, which is all this product needs.
RETROGRADES = {
    "mercury": [
        (date(2024, 4, 1), date(2024, 4, 25)),
        (date(2024, 8, 5), date(2024, 8, 28)),
        (date(2024, 11, 26), date(2024, 12, 15)),
        (date(2025, 3, 15), date(2025, 4, 7)),
        (date(2025, 7, 18), date(2025, 8, 11)),
        (date(2025, 11, 9), date(2025, 11, 29)),
        (date(2026, 2, 26), date(2026, 3, 20)),
        (date(2026, 6, 29), date(2026, 7, 23)),
        (date(2026, 10, 24), date(2026, 11, 13)),
        (date(2027, 2, 9), date(2027, 3, 3)),
        (date(2027, 6, 10), date(2027, 7, 4)),
        (date(2027, 10, 7), date(2027, 10, 28)),
        (date(2028, 1, 24), date(2028, 2, 14)),
        (date(2028, 5, 21), date(2028, 6, 13)),
        (date(2028, 9, 19), date(2028, 10, 11)),
        (date(2029, 1, 7), date(2029, 1, 27)),
        (date(2029, 5, 1), date(2029, 5, 25)),
        (date(2029, 8, 31), date(2029, 9, 23)),
        (date(2029, 12, 21), date(2030, 1, 10)),
        (date(2030, 4, 13), date(2030, 5, 6)),
        (date(2030, 8, 13), date(2030, 9, 5)),
        (date(2030, 12, 5), date(2030, 12, 24)),
    ],
    "venus": [
        (date(2025, 3, 1), date(2025, 4, 12)),
        (date(2026, 10, 3), date(2026, 11, 13)),
        (date(2028, 5, 12), date(2028, 6, 24)),
        (date(2029, 12, 19), date(2030, 1, 29)),
    ],
    "mars": [
        (date(2024, 12, 6), date(2025, 2, 23)),
        (date(2027, 1, 10), date(2027, 4, 1)),
        (date(2029, 3, 1), date(2029, 5, 20)),
    ],
    "jupiter": [
        (date(2024, 10, 9), date(2025, 2, 4)),
        (date(2025, 11, 11), date(2026, 3, 11)),
        (date(2026, 12, 13), date(2027, 4, 13)),
        (date(2028, 1, 13), date(2028, 5, 14)),
        (date(2029, 2, 12), date(2029, 6, 14)),
        (date(2030, 3, 15), date(2030, 7, 15)),
    ],
    "saturn": [
        (date(2024, 6, 29), date(2024, 11, 15)),
        (date(2025, 7, 13), date(2025, 11, 28)),
        (date(2026, 7, 28), date(2026, 12, 12)),
        (date(2027, 8, 11), date(2027, 12, 26)),
        (date(2028, 8, 23), date(2029, 1, 7)),
        (date(2029, 9, 5), date(2030, 1, 20)),
        (date(2030, 9, 18), date(2031, 2, 2)),
    ],
}

# Priority when several planets are notable on the same day. Personal/fast
# planets take precedence because their emphasis feels more immediate; the
# social/outer planets sit lower because they are retrograde so often that
# always surfacing them would flatten the variety.
_PLANET_PRIORITY = ("mercury", "venus", "mars", "jupiter", "saturn")


def is_retrograde(planet: str, d: date) -> bool:
    for start, end in RETROGRADES.get(planet, ()):  # inclusive window
        if start <= d <= end:
            return True
    return False


@dataclass(frozen=True)
class PlanetaryEmphasis:
    planet: str          # one of _PLANET_PRIORITY
    retrograde: bool
    key: str             # e.g. "mercury_retrograde" / "venus_direct"


def planetary_emphasis(d: date) -> PlanetaryEmphasis:
    """Pick the single most relevant planet for the day.

    A retrograde planet always wins over direct planets (retrogrades are the
    salient, talk-about-able events). Among retrogrades, personal planets win.
    If nothing is retrograde, we rotate deterministically through the personal
    planets by day-of-year so 'direct' days still vary.
    """
    for planet in _PLANET_PRIORITY:
        if is_retrograde(planet, d):
            return PlanetaryEmphasis(planet, True, planet + "_retrograde")

    # No retrogrades: rotate through the fast planets by day-of-year.
    rotation = ("mercury", "venus", "mars", "jupiter", "saturn")
    planet = rotation[d.timetuple().tm_yday % len(rotation)]
    return PlanetaryEmphasis(planet, False, planet + "_direct")


# --------------------------------------------------------------------------
# Seasonal markers (solstices & equinoxes)
# --------------------------------------------------------------------------

@dataclass(frozen=True)
class SeasonMarker:
    marker: str          # "solstice", "equinox", or "ordinary"
    name: str            # e.g. "spring_equinox", "" for ordinary
    days_away: int       # signed days to the nearest event (0 == today)


def _season_event_date(year: int, target_longitude: float) -> date:
    """Approximate calendar date when the Sun reaches a given ecliptic
    longitude (0=spring equinox, 90=summer solstice, 180=autumn equinox,
    270=winter solstice). Uses a coarse day scan + the solar formula, which is
    accurate to the correct day for this product's purposes.
    """
    # Reasonable starting month for each quarter, then scan +-20 days.
    approx_month = {0: 3, 90: 6, 180: 9, 270: 12}[int(target_longitude)]
    approx = date(year, approx_month, 20)

    best_day = approx
    best_err = 999.0
    for offset in range(-20, 21):
        try:
            cand = date.fromordinal(approx.toordinal() + offset)
        except (OverflowError, ValueError):
            continue
        jd = julian_day(_noon_datetime(cand))
        lon = sun_ecliptic_longitude(jd)
        # Angular distance to target, wrapped to [-180, 180].
        err = abs(((lon - target_longitude + 180.0) % 360.0) - 180.0)
        if err < best_err:
            best_err = err
            best_day = cand
    return best_day


def season_marker(d: date, window_days: int = 3) -> SeasonMarker:
    """Whether the date is within `window_days` of a solstice/equinox."""
    events = (
        (0.0, "spring_equinox", "equinox"),
        (90.0, "summer_solstice", "solstice"),
        (180.0, "autumn_equinox", "equinox"),
        (270.0, "winter_solstice", "solstice"),
    )

    best = None  # (abs_days, signed_days, name, marker)
    for year in (d.year - 1, d.year, d.year + 1):
        for lon, name, marker in events:
            ev = _season_event_date(year, lon)
            delta = (ev - d).days
            cand = (abs(delta), delta, name, marker)
            if best is None or cand[0] < best[0]:
                best = cand

    abs_days, signed, name, marker = best
    if abs_days <= window_days:
        return SeasonMarker(marker=marker, name=name, days_away=signed)
    return SeasonMarker(marker="ordinary", name="", days_away=signed)


# --------------------------------------------------------------------------
# Deterministic per-user, per-day seed
# --------------------------------------------------------------------------

def daily_seed(target: date, birth: date | None) -> int:
    """A stable 63-bit seed from (target date, birth date).

    Same day + same user => same seed (reproducible message). Different users
    on the same day => different seeds. Uses a simple, portable integer hash
    (FNV-1a style) rather than Python's salted hash() so results match across
    processes and, later, across a C/MicroPython port.
    """
    parts = [target.year, target.month, target.day]
    if birth is not None:
        parts += [birth.year, birth.month, birth.day]
    else:
        parts += [0, 0, 0]

    h = 1469598103934665603  # FNV offset basis (64-bit)
    prime = 1099511628211
    mask = (1 << 64) - 1
    for value in parts:
        # Mix each 32-ish-bit integer byte by byte.
        v = value & 0xFFFFFFFF
        for _ in range(4):
            h ^= v & 0xFF
            h = (h * prime) & mask
            v >>= 8
    return h & ((1 << 63) - 1)


@dataclass(frozen=True)
class SkyState:
    """Everything the composition engine needs about a given day."""
    target_date: date
    moon: MoonInfo
    planet: PlanetaryEmphasis
    season: SeasonMarker
    sun_sign: str | None
    moon_sign: str
    seed: int


def sky_state(target: date, birth: date | None) -> SkyState:
    """Assemble the full astronomical picture for a date + optional user."""
    return SkyState(
        target_date=target,
        moon=moon_phase(target),
        planet=planetary_emphasis(target),
        season=season_marker(target),
        sun_sign=sun_sign(birth) if birth is not None else None,
        moon_sign=moon_sign(target),
        seed=daily_seed(target, birth),
    )
