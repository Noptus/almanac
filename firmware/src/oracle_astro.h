// oracle_astro.h -- astronomy layer for The Oracle (ESP32 / portable C++).
//
// Faithful port of oracle_generator/astronomy.py. Uses only <cmath> and
// integer math -- no heap, no STL containers in the hot path, no RTTI. Safe
// for a low-power ESP32. All angle math is in double; the results only need
// to name a moon phase / zodiac sign / season, so double is ample and the
// determinism that matters (the PRNG seed) is pure integer.
//
// Everything here is header-only-friendly but split into .cpp for clarity.
#ifndef ORACLE_ASTRO_H
#define ORACLE_ASTRO_H

#include <cstdint>

namespace oracle {

// ---- Simple calendar date (no <chrono> dependency) ---------------------
struct Date {
  int year;
  int month;  // 1..12
  int day;    // 1..31
};

// Comparison helpers (inclusive range checks for retrograde windows).
bool date_le(const Date& a, const Date& b);   // a <= b
bool date_ge(const Date& a, const Date& b);   // a >= b
int  day_of_year(const Date& d);              // 1..366
long date_to_ordinal(const Date& d);          // proleptic Gregorian ordinal
Date ordinal_to_date(long ord);

// ---- Moon phase --------------------------------------------------------
enum class MoonPhase : uint8_t {
  New = 0,
  WaxingCrescent,
  FirstQuarter,
  WaxingGibbous,
  Full,
  WaningGibbous,
  LastQuarter,
  WaningCrescent,
};

struct MoonInfo {
  MoonPhase phase;
  double illumination;  // 0..1
  double elongation;    // degrees 0..360
};

MoonInfo moon_phase(const Date& d);
const char* moon_phase_name(MoonPhase p);        // "new", "waxing_crescent"...
const char* moon_phase_label(MoonPhase p);       // "New", "Waxing Crescent"...

// ---- Zodiac ------------------------------------------------------------
const char* sun_sign(const Date& birth);         // "Aries".. "Pisces"
const char* moon_sign(const Date& d);

// ---- Planetary emphasis ------------------------------------------------
enum class Planet : uint8_t { Mercury = 0, Venus, Mars, Jupiter, Saturn };

struct PlanetaryEmphasis {
  Planet planet;
  bool retrograde;
  // Stable key index into the fragment bank's 10 planetary states:
  //   index = planet*2 + (retrograde ? 0 : 1)
  int key_index;
};

bool is_retrograde(Planet p, const Date& d);
PlanetaryEmphasis planetary_emphasis(const Date& d);
const char* planet_name(Planet p);               // "mercury"...
const char* planet_label(Planet p);              // "Mercury"...

// ---- Season markers ----------------------------------------------------
enum class SeasonKind : uint8_t { Ordinary = 0, Solstice, Equinox };

struct SeasonMarker {
  SeasonKind kind;
  const char* name;   // "spring_equinox".."winter_solstice" or ""
  int days_away;      // signed days to nearest event (0 == today)
};

SeasonMarker season_marker(const Date& d, int window_days = 3);

// ---- Deterministic seed (must match Python FNV-1a exactly) -------------
uint64_t daily_seed(const Date& target, const Date* birth /* nullptr ok */);

// ---- Assembled sky state ----------------------------------------------
struct SkyState {
  Date target;
  MoonInfo moon;
  PlanetaryEmphasis planet;
  SeasonMarker season;
  const char* sun_sign;   // nullptr if no birth date
  const char* moon_sign;
  uint64_t seed;
};

SkyState sky_state(const Date& target, const Date* birth /* nullptr ok */);

}  // namespace oracle

#endif  // ORACLE_ASTRO_H
