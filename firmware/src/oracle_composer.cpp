// oracle_composer.cpp -- port of composer.py + safety.py.
#include "oracle_composer.h"
#include "oracle_fragments.h"

#include <cstring>
#include <cctype>

namespace oracle {

// ------------------------- PRNG (xorshift64*) -------------------------
// Must reproduce composer.py::_Rng exactly.
namespace {
struct Rng {
  uint64_t s;
  explicit Rng(uint64_t seed) {
    s = seed & 0xFFFFFFFFFFFFFFFFULL;
    if (s == 0) s = 0x9E3779B97F4A7C15ULL;
  }
  uint64_t next_u64() {
    uint64_t x = s;
    x ^= (x >> 12);
    x ^= (x << 25);
    x ^= (x >> 27);
    s = x;
    return s * 0x2545F4914F6CDD1DULL;
  }
  int below(int n) {
    if (n <= 1) return 0;
    return static_cast<int>(next_u64() % static_cast<uint64_t>(n));
  }
};

// ---- lowercase-copy helper for safety scan ----
void to_lower_copy(const char* src, char* dst, size_t cap) {
  size_t i = 0;
  for (; src[i] && i + 1 < cap; ++i) dst[i] = static_cast<char>(std::tolower((unsigned char)src[i]));
  dst[i] = '\0';
}

bool has_substr(const char* hay, const char* needle) {
  return std::strstr(hay, needle) != nullptr;
}

// Word-boundary search: needle must be delimited by non-alnum on both sides.
bool has_word(const char* hay, const char* needle) {
  size_t nlen = std::strlen(needle);
  const char* p = hay;
  while ((p = std::strstr(p, needle)) != nullptr) {
    bool left_ok = (p == hay) || !std::isalnum((unsigned char)p[-1]);
    char after = p[nlen];
    bool right_ok = (after == '\0') || !std::isalnum((unsigned char)after);
    if (left_ok && right_ok) return true;
    p += 1;
  }
  return false;
}
}  // namespace

// ------------------------- Safety ------------------------------------
bool content_safety_check(const char* message) {
  char low[1024];
  to_lower_copy(message, low, sizeof(low));

  static const char* predictive[] = {
      "will happen", "is guaranteed", "guaranteed to", "you are certain to",
      "you will definitely", "destined to", "fated to", "it is certain that",
      "without a doubt you will"};
  static const char* health_fate[] = {
      "you will die", "you will get sick", "you will fall ill",
      "you will be cured", "you will recover from", "your diagnosis",
      "you have a disease"};
  static const char* medical_words[] = {
      "diagnosis", "diagnose", "diagnosed", "cure", "cured", "prescription",
      "prescribe", "medication", "symptoms", "treatment", "disease", "tumor",
      "tumour", "cancer"};
  static const char* financial[] = {
      "invest in", "buy stock", "buy stocks", "sell your", "put your money",
      "guaranteed return", "guaranteed returns", "you will get rich",
      "you will lose money", "double your money"};
  static const char* legal[] = {
      "you should sue", "file a lawsuit", "plead guilty", "plead not guilty",
      "sign the contract", "you are legally", "this is legal advice"};

  for (const char* s : predictive) if (has_substr(low, s)) return false;
  for (const char* s : health_fate) if (has_substr(low, s)) return false;
  for (const char* s : medical_words) if (has_word(low, s)) return false;
  for (const char* s : financial) if (has_substr(low, s)) return false;
  for (const char* s : legal) if (has_substr(low, s)) return false;
  return true;
}

// ------------------------- History (ring of signatures) ---------------
void history_init(MessageHistory* h) {
  h->count = 0;
  h->head = 0;
  for (int i = 0; i < MessageHistory::kMax; ++i) h->sigs[i] = 0;
}

static bool history_has(const MessageHistory* h, uint64_t sig) {
  for (int i = 0; i < h->count; ++i) if (h->sigs[i] == sig) return true;
  return false;
}

static void history_record(MessageHistory* h, uint64_t sig) {
  if (h->count < MessageHistory::kMax) {
    h->sigs[h->count++] = sig;
  } else {
    h->sigs[h->head] = sig;
    h->head = (h->head + 1) % MessageHistory::kMax;
  }
}

// ------------------------- {name} handling ----------------------------
namespace {
// Append `src` to out[pos], stripping "{name}" cleanly (mirrors the Python
// regex tidy: drop optional leading comma + spaces, fix space-before-punct,
// collapse doubles, tidy " --"). If `name` given and token present and not yet
// used, substitute it and set *used.
size_t append_fragment(char* out, size_t pos, size_t cap, const char* src,
                       const char* name, bool* used) {
  const char* token = std::strstr(src, "{name}");
  // Fast path: no token.
  if (!token) {
    while (*src && pos + 1 < cap) out[pos++] = *src++;
    return pos;
  }
  if (name && *name && !*used) {
    // Substitute name for the (first) token; copy rest verbatim.
    const char* p = src;
    while (*p && pos + 1 < cap) {
      if (std::strncmp(p, "{name}", 6) == 0) {
        const char* q = name;
        while (*q && pos + 1 < cap) out[pos++] = *q++;
        p += 6;
      } else {
        out[pos++] = *p++;
      }
    }
    *used = true;
    return pos;
  }
  // Strip the token. Walk char by char reproducing the Python cleanup.
  // Step 1: build with ",?\s*\{name\}" removed.
  // We implement by scanning: when we see "{name}", back up over preceding
  // spaces and an optional comma already written.
  const char* p = src;
  while (*p) {
    if (std::strncmp(p, "{name}", 6) == 0) {
      // Remove preceding spaces already in `out`.
      while (pos > 0 && out[pos - 1] == ' ') pos--;
      // Remove one optional preceding comma.
      if (pos > 0 && out[pos - 1] == ',') pos--;
      p += 6;
      // Skip following spaces in source too (they'll be re-added by next word).
      continue;
    }
    if (pos + 1 < cap) out[pos++] = *p;
    ++p;
  }
  return pos;
}

int word_count(const char* s) {
  int n = 0;
  bool in = false;
  for (; *s; ++s) {
    if (*s == ' ' || *s == '\n' || *s == '\t') {
      in = false;
    } else if (!in) {
      in = true;
      ++n;
    }
  }
  return n;
}
}  // namespace

// ------------------------- Selection & assembly -----------------------
namespace {
// A chosen fragment is a pointer to its text (tone unused in output).
struct Selected {
  const char* text[4];
  int n;
};

Selected select_fragments(const SkyState& st, Rng* rng) {
  Selected sel;
  sel.n = 0;

  // 1. Moon phase (always).
  int mi = static_cast<int>(st.moon.phase);
  const Fragment* mp = kMoonPools[mi];
  sel.text[sel.n++] = mp[rng->below(kMoonCounts[mi])].text;

  // 2. Planetary emphasis (always).
  int pk = st.planet.key_index;
  const Fragment* pp = kPlanetPools[pk];
  sel.text[sel.n++] = pp[rng->below(kPlanetCounts[pk])].text;

  // 3. Season marker.
  if (st.season.kind == SeasonKind::Solstice) {
    const Fragment* sp = kSeasonPools[1];
    sel.text[sel.n++] = sp[rng->below(kSeasonCounts[1])].text;
  } else if (st.season.kind == SeasonKind::Equinox) {
    const Fragment* sp = kSeasonPools[2];
    sel.text[sel.n++] = sp[rng->below(kSeasonCounts[2])].text;
  } else {
    // Ordinary day: include ~40% of the time (below(5) < 2), matching Python.
    if (rng->below(5) < 2 && sel.n < 4) {
      const Fragment* sp = kSeasonPools[0];
      sel.text[sel.n++] = sp[rng->below(kSeasonCounts[0])].text;
    }
  }

  // 4. Closing (unless already at 4).
  if (sel.n < 4) {
    sel.text[sel.n++] = kClosing[rng->below(kClosing_count)].text;
  }
  return sel;
}

// Strip the "{name}" token for signature purposes (name-neutral text).
uint64_t signature_of(const Selected& sel) {
  uint64_t acc = 1469598103934665603ULL;  // FNV offset basis
  for (int i = 0; i < sel.n; ++i) {
    // Hash the text but skip the 6-char "{name}" token wherever it appears,
    // mirroring _strip_name_token (which removes the token; surrounding text,
    // incl. the comma/space, is preserved by Python's .replace("{name}","")).
    const char* p = sel.text[i];
    while (*p) {
      if (std::strncmp(p, "{name}", 6) == 0) { p += 6; continue; }
      acc ^= (unsigned char)*p;
      acc *= 1099511628211ULL;
      ++p;
    }
    // Python strips leading/trailing whitespace on the neutralised text but the
    // fragments have no leading/trailing space, so this matches.
  }
  return acc & 0xFFFFFFFFFFFFFFFFULL;
}

size_t assemble(const Selected& sel, const char* name, char* out, size_t cap) {
  size_t pos = 0;
  bool used = false;
  for (int i = 0; i < sel.n; ++i) {
    if (i > 0 && pos + 1 < cap) out[pos++] = ' ';
    pos = append_fragment(out, pos, cap, sel.text[i], name, &used);
  }
  // Trim any accidental double spaces / trailing space.
  // Single pass compaction of runs of spaces.
  size_t w = 0;
  bool prev_space = false;
  for (size_t r = 0; r < pos; ++r) {
    char c = out[r];
    if (c == ' ') {
      if (prev_space) continue;
      prev_space = true;
    } else {
      prev_space = false;
    }
    out[w++] = c;
  }
  while (w > 0 && out[w - 1] == ' ') w--;
  out[w] = '\0';
  return w;
}
}  // namespace

static const int kMaxRerolls = 12;

size_t compose_message(const SkyState& state, const char* name,
                       MessageHistory* history, char* out, size_t out_cap) {
  Rng rng(state.seed);
  char last_safe[1024];
  bool have_safe = false;

  for (int attempt = 0; attempt < kMaxRerolls; ++attempt) {
    Selected sel = select_fragments(state, &rng);
    uint64_t sig = signature_of(sel);

    char buf[1024];
    size_t len = assemble(sel, name, buf, sizeof(buf));

    if (!content_safety_check(buf)) continue;  // unsafe -> reroll

    std::strncpy(last_safe, buf, sizeof(last_safe) - 1);
    last_safe[sizeof(last_safe) - 1] = '\0';
    have_safe = true;

    if (history && history_has(history, sig)) continue;  // repeat -> reroll

    int wc = word_count(buf);
    if (wc < 30 || wc > 80) continue;  // outside soft band -> reroll

    if (history) history_record(history, sig);
    std::strncpy(out, buf, out_cap - 1);
    out[out_cap - 1] = '\0';
    return len < out_cap - 1 ? len : out_cap - 1;
  }

  if (have_safe) {
    std::strncpy(out, last_safe, out_cap - 1);
    out[out_cap - 1] = '\0';
    return std::strlen(out);
  }
  // Fallback: fresh RNG, one pass, ignore constraints.
  Rng rng2(state.seed);
  Selected sel = select_fragments(state, &rng2);
  return assemble(sel, name, out, out_cap);
}

size_t generate_daily_message(const Date& target, const Date* birth,
                              const char* name, MessageHistory* history,
                              char* out, size_t out_cap) {
  SkyState st = sky_state(target, birth);
  return compose_message(st, name, history, out, out_cap);
}

}  // namespace oracle
