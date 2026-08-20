"""Content safety / framing guardrails.

A real filter, not decoration. Every finalized message passes through
``content_safety_check`` before it is returned to the caller. The check
rejects:

* absolute predictive language ("this will happen", "guaranteed", ...),
* death / illness predictions,
* medical, financial, and legal advice framed as instruction or claim.

The blocklist is intentionally phrase-based and word-boundary aware so it
catches the dangerous *sense* while tolerating the benign fragment voice
(e.g. "a good day for straight talk" must pass; "invest in bitcoin" must not).
"""

from __future__ import annotations

import re

# Phrases that assert certainty about the future. These are never acceptable
# in a reflection-based product.
_PREDICTIVE_PHRASES = (
    "will happen",
    "is guaranteed",
    "guaranteed to",
    "you are certain to",
    "you will definitely",
    "destined to",
    "fated to",
    "it is certain that",
    "without a doubt you will",
)

# Death / illness predictions.
_HEALTH_FATE_PHRASES = (
    "you will die",
    "you will get sick",
    "you will fall ill",
    "you will be cured",
    "you will recover from",
    "your diagnosis",
    "you have a disease",
)

# Medical / clinical claim words. Word-boundary matched.
_MEDICAL_WORDS = (
    "diagnosis",
    "diagnose",
    "diagnosed",
    "cure",
    "cured",
    "prescription",
    "prescribe",
    "medication",
    "symptoms",
    "treatment",
    "disease",
    "tumor",
    "tumour",
    "cancer",
)

# Financial advice / instruction. Phrase + word matched.
_FINANCIAL_PHRASES = (
    "invest in",
    "buy stock",
    "buy stocks",
    "sell your",
    "put your money",
    "guaranteed return",
    "guaranteed returns",
    "you will get rich",
    "you will lose money",
    "double your money",
)

# Legal advice / instruction.
_LEGAL_PHRASES = (
    "you should sue",
    "file a lawsuit",
    "plead guilty",
    "plead not guilty",
    "sign the contract",
    "you are legally",
    "this is legal advice",
)


def _contains_phrase(text_lower: str, phrases) -> str | None:
    for phrase in phrases:
        if phrase in text_lower:
            return phrase
    return None


def _contains_word(text_lower: str, words) -> str | None:
    for word in words:
        if re.search(r"\b" + re.escape(word) + r"\b", text_lower):
            return word
    return None


def safety_violation(message: str) -> str | None:
    """Return a human-readable reason string if the message is unsafe, else None."""
    lowered = message.lower()

    hit = _contains_phrase(lowered, _PREDICTIVE_PHRASES)
    if hit:
        return "predictive-certainty: %r" % hit

    hit = _contains_phrase(lowered, _HEALTH_FATE_PHRASES)
    if hit:
        return "health-fate: %r" % hit

    hit = _contains_word(lowered, _MEDICAL_WORDS)
    if hit:
        return "medical-claim: %r" % hit

    hit = _contains_phrase(lowered, _FINANCIAL_PHRASES)
    if hit:
        return "financial-advice: %r" % hit

    hit = _contains_phrase(lowered, _LEGAL_PHRASES)
    if hit:
        return "legal-advice: %r" % hit

    return None


def content_safety_check(message: str) -> bool:
    """True if the message is safe to show; False if it must be regenerated."""
    return safety_violation(message) is None
