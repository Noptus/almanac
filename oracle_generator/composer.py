"""The composition engine.

Turns a day's :class:`SkyState` into a short, coherent, safe paragraph.

Key properties
--------------
* Deterministic: selection is driven only by the day's seed, so the same
  (date, user) always yields the same message.
* Varied: 2-4 fragments are chosen from distinct categories, tone-aware, with
  a rolling per-user history so consecutive days don't repeat a combination.
* Framed, never predictive: every candidate passes ``content_safety_check``
  before it can be returned.
"""

from __future__ import annotations

import re
from datetime import date, timedelta
from typing import Optional

from . import fragments
from .astronomy import SkyState, sky_state
from .safety import content_safety_check


# --------------------------------------------------------------------------
# A tiny, portable, deterministic PRNG (xorshift64*). We avoid `random` so the
# stream is identical here and in a future C / MicroPython port.
# --------------------------------------------------------------------------

class _Rng:
    __slots__ = ("_s",)

    def __init__(self, seed: int) -> None:
        # State must be non-zero.
        self._s = (seed & ((1 << 64) - 1)) or 0x9E3779B97F4A7C15

    def next_u64(self) -> int:
        x = self._s
        x ^= (x >> 12) & ((1 << 64) - 1)
        x ^= (x << 25) & ((1 << 64) - 1)
        x ^= (x >> 27)
        self._s = x & ((1 << 64) - 1)
        return (self._s * 0x2545F4914F6CDD1D) & ((1 << 64) - 1)

    def below(self, n: int) -> int:
        """Uniform-ish integer in [0, n). n is small (fragment counts)."""
        if n <= 1:
            return 0
        return self.next_u64() % n

    def choice(self, seq):
        return seq[self.below(len(seq))]


# --------------------------------------------------------------------------
# Rolling history to avoid repeating fragment combinations for a user.
# --------------------------------------------------------------------------

class MessageHistory:
    """Remembers recently used fragment-combination signatures per user.

    Kept deliberately small and simple so it can live in a few KB of flash on
    the device. Signatures are frozensets of the exact fragment texts chosen.
    """

    def __init__(self, max_days: int = 30) -> None:
        self.max_days = max_days
        self._seen: list[frozenset] = []

    def is_collision(self, signature: frozenset) -> bool:
        return signature in self._seen

    def record(self, signature: frozenset) -> None:
        self._seen.append(signature)
        if len(self._seen) > self.max_days:
            self._seen.pop(0)


# --------------------------------------------------------------------------
# {name} handling
# --------------------------------------------------------------------------

def _fill_name(text: str, name: Optional[str]) -> str:
    """Insert `name` for the `{name}` placeholder, or remove it cleanly.

    When no name is supplied we strip the placeholder and repair the small
    punctuation damage that leaves behind:
      "..., {name} -- ..."  -> "... -- ..."
      "..., {name}."        -> "..."
      "Hello, {name}!"      -> "Hello!"
    """
    if "{name}" not in text:
        return text

    if name:
        return text.replace("{name}", name)

    # Remove ", {name}" or " {name}" and tidy leftover spacing/punctuation.
    out = text
    out = re.sub(r",?\s*\{name\}", "", out)          # drop the token + leading comma
    out = re.sub(r"\s+([,.!?;:])", r"\1", out)          # no space before punctuation
    out = re.sub(r"\s{2,}", " ", out)                    # collapse double spaces
    out = re.sub(r"\s+--", " --", out)                   # tidy em-dash spacing
    return out.strip()


def _strip_name_token(text: str) -> str:
    """A version of the text with the placeholder neutralised (for signatures)."""
    return text.replace("{name}", "").strip()


# --------------------------------------------------------------------------
# Selection
# --------------------------------------------------------------------------

def _select_fragments(state: SkyState, rng: _Rng):
    """Choose 2-4 (text, tone) fragments across relevant categories.

    Moon phase is always included. Planetary emphasis is always relevant (there
    is always a most-notable planet). Season marker is included only when the
    day is actually near a solstice/equinox -- an ordinary Tuesday gets an
    'ordinary' line only sometimes, to keep variety without forcing drama.
    """
    chosen = []  # list of (category, text, tone)

    # 1. Moon phase -- always.
    moon_pool = fragments.MOON_PHASE[state.moon.phase]
    chosen.append(("moon", *rng.choice(moon_pool)))

    # 2. Planetary emphasis -- always (there is always a most-notable planet).
    planet_pool = fragments.PLANETARY_EMPHASIS[state.planet.key]
    chosen.append(("planet", *rng.choice(planet_pool)))

    # 3. Season marker.
    if state.season.marker in ("solstice", "equinox"):
        pool = fragments.SEASON_MARKER[state.season.marker]
        chosen.append(("season", *rng.choice(pool)))
    else:
        # Ordinary day: include an 'ordinary' framing ~40% of the time so the
        # message isn't always the same length, but never force it.
        if rng.below(5) < 2:
            pool = fragments.SEASON_MARKER["ordinary"]
            chosen.append(("season", *rng.choice(pool)))

    # 4. Closing line -- included when it keeps the paragraph in the 2-4
    #    fragment / 40-70 word target. Always add unless we already have 4.
    if len(chosen) < 4:
        chosen.append(("closing", *rng.choice(fragments.CLOSING_LINES)))

    return chosen


def _word_count(text: str) -> int:
    return len(text.split())


def _assemble(selected, name: Optional[str]) -> str:
    """Join fragments into one paragraph, using `name` in exactly one of them.

    We place the name in the first selected fragment that contains a `{name}`
    token; every other fragment has its token stripped so the name appears at
    most once.
    """
    name_used = False
    parts = []
    for _cat, text, _tone in selected:
        has_token = "{name}" in text
        if has_token and name and not name_used:
            parts.append(_fill_name(text, name))
            name_used = True
        else:
            parts.append(_fill_name(text, None))  # strip token cleanly
    return " ".join(p.strip() for p in parts if p.strip())


# --------------------------------------------------------------------------
# Public API
# --------------------------------------------------------------------------

_MAX_REROLLS = 12


def compose_from_state(
    state: SkyState,
    name: Optional[str] = None,
    history: Optional[MessageHistory] = None,
) -> str:
    """Compose one message from a precomputed SkyState.

    Rerolls (deterministically, by advancing the RNG) on: history collision,
    safety failure, or word count outside the 40-70 target band. Falls back to
    the last safe candidate if it can't satisfy every soft constraint.
    """
    rng = _Rng(state.seed)
    last_safe = None

    for _attempt in range(_MAX_REROLLS):
        selected = _select_fragments(state, rng)

        # Signature = the set of name-neutral fragment texts.
        signature = frozenset(_strip_name_token(t) for _c, t, _to in selected)

        message = _assemble(selected, name)

        if not content_safety_check(message):
            continue  # unsafe -> reroll (RNG has already advanced)

        last_safe = message  # remember a safe option even if other checks fail

        if history is not None and history.is_collision(signature):
            continue  # repeat combination -> reroll

        wc = _word_count(message)
        if not (30 <= wc <= 80):  # a little slack around the 40-70 target
            continue

        if history is not None:
            history.record(signature)
        return message

    # Couldn't satisfy every soft constraint; return the last safe candidate.
    return last_safe if last_safe is not None else _assemble(
        _select_fragments(state, _Rng(state.seed)), name
    )


def generate_daily_message(
    birth_date: Optional[date],
    birth_time: Optional[str],
    birth_place: Optional[str],
    target_date: date,
    name: Optional[str] = None,
    history: Optional[MessageHistory] = None,
) -> str:
    """Generate the daily message for a user on a given date.

    Parameters mirror the product spec. ``birth_time`` and ``birth_place`` are
    accepted for API stability and future rising-sign work; the current
    astronomical layer does not require them, so they are not used yet.

    Returns plain text ready to be laid out on an 800x480 e-ink canvas.
    """
    _ = (birth_time, birth_place)  # reserved for future use (rising sign, houses)
    state = sky_state(target_date, birth_date)
    return compose_from_state(state, name=name, history=history)
