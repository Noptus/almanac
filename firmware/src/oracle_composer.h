// oracle_composer.h -- deterministic message composition for the ESP32.
//
// Port of oracle_generator/composer.py + safety.py. Writes the finished
// message into a caller-provided char buffer (no heap allocation).
#ifndef ORACLE_COMPOSER_H
#define ORACLE_COMPOSER_H

#include <cstddef>

#include "oracle_astro.h"

namespace oracle {

// Content safety net: returns true if the message is safe to display.
bool content_safety_check(const char* message);

// Compose the daily message for a sky state into `out` (size `out_cap`).
// `name` may be nullptr (placeholder is stripped cleanly). Returns the number
// of characters written (excluding the terminating NUL).
//
// `history` is an optional pointer to a small rolling buffer of recent
// combination signatures (see MessageHistory) to avoid consecutive repeats;
// pass nullptr to skip collision avoidance.
struct MessageHistory {
  static const int kMax = 30;
  uint64_t sigs[kMax];
  int count;
  int head;  // ring insert position
};

void history_init(MessageHistory* h);

size_t compose_message(const SkyState& state, const char* name,
                       MessageHistory* history, char* out, size_t out_cap);

// Convenience: from raw date + optional birth date straight to text.
size_t generate_daily_message(const Date& target, const Date* birth,
                              const char* name, MessageHistory* history,
                              char* out, size_t out_cap);

}  // namespace oracle

#endif  // ORACLE_COMPOSER_H
