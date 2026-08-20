"""oracle_generator -- astronomy-backed daily message engine for The Oracle.

A deterministic, offline-capable content generator. Real astronomical
calculations (pure stdlib, portable to C / MicroPython) seed varied, warm,
non-predictive daily reflections for a personal e-ink almanac.

Typical use::

    from datetime import date
    from oracle_generator import generate_daily_message

    print(generate_daily_message(
        birth_date=date(1990, 6, 15),
        birth_time=None,
        birth_place=None,
        target_date=date(2026, 8, 20),
        name="Sam",
    ))
"""

from __future__ import annotations

from .astronomy import (
    MoonInfo,
    PlanetaryEmphasis,
    SeasonMarker,
    SkyState,
    moon_phase,
    moon_sign,
    planetary_emphasis,
    season_marker,
    sky_state,
    sun_sign,
)
from .composer import (
    MessageHistory,
    compose_from_state,
    generate_daily_message,
)
from .safety import content_safety_check, safety_violation

__all__ = [
    "generate_daily_message",
    "compose_from_state",
    "MessageHistory",
    "content_safety_check",
    "safety_violation",
    "sky_state",
    "SkyState",
    "moon_phase",
    "moon_sign",
    "sun_sign",
    "planetary_emphasis",
    "season_marker",
    "MoonInfo",
    "PlanetaryEmphasis",
    "SeasonMarker",
]

__version__ = "0.1.0"
