// oracle_astro.cpp -- implementation. Port of astronomy.py.
#include "oracle_astro.h"

#include <cmath>

namespace oracle {

namespace {
constexpr double kDeg = 3.14159265358979323846 / 180.0;

double norm360(double x) {
  x = std::fmod(x, 360.0);
  if (x < 0.0) x += 360.0;
  return x;
}
double sind(double deg) { return std::sin(deg * kDeg); }
double cosd(double deg) { return std::cos(deg * kDeg); }

// ---- Julian Day for a date at 12:00 UTC (matches Python _noon_datetime) --
double julian_day_noon(const Date& d) {
  int year = d.year;
  int month = d.month;
  double day = static_cast<double>(d.day) + 12.0 / 24.0;  // noon
  if (month <= 2) {
    year -= 1;
    month += 12;
  }
  int a = year / 100;
  int b = 2 - a + a / 4;
  double jd = std::floor(365.25 * (year + 4716)) +
              std::floor(30.6001 * (month + 1)) + day + b - 1524.5;
  return jd;
}

double julian_centuries(double jd) { return (jd - 2451545.0) / 36525.0; }

double sun_ecliptic_longitude(double jd) {
  double t = julian_centuries(jd);
  double l0 = 280.46646 + 36000.76983 * t + 0.0003032 * t * t;
  double m = 357.52911 + 35999.05029 * t - 0.0001537 * t * t;
  double c = (1.914602 - 0.004817 * t - 0.000014 * t * t) * sind(m) +
             (0.019993 - 0.000101 * t) * sind(2 * m) +
             0.000289 * sind(3 * m);
  double true_long = l0 + c;
  double omega = 125.04 - 1934.136 * t;
  double apparent = true_long - 0.00569 - 0.00478 * sind(omega);
  return norm360(apparent);
}

double moon_ecliptic_longitude(double jd) {
  double t = julian_centuries(jd);
  double lp = 218.3164477 + 481267.88123421 * t;
  double d = 297.8501921 + 445267.1114034 * t;
  double m = 357.5291092 + 35999.0502909 * t;
  double mp = 134.9633964 + 477198.8675055 * t;
  double f = 93.2720950 + 483202.0175233 * t;
  double lon = lp;
  lon += 6.288774 * sind(mp);
  lon += 1.274027 * sind(2 * d - mp);
  lon += 0.658314 * sind(2 * d);
  lon += 0.213618 * sind(2 * mp);
  lon += -0.185116 * sind(m);
  lon += -0.114332 * sind(2 * f);
  lon += 0.058793 * sind(2 * d - 2 * mp);
  lon += 0.057066 * sind(2 * d - m - mp);
  lon += 0.053322 * sind(2 * d + mp);
  lon += 0.045758 * sind(2 * d - m);
  lon += -0.040923 * sind(m - mp);
  lon += -0.034720 * sind(d);
  lon += -0.030383 * sind(m + mp);
  return norm360(lon);
}
}  // namespace

// ------------------------- Date helpers -------------------------------
// Proleptic Gregorian ordinal, matching Python date.toordinal() offset so the
// day-scan in season_marker behaves identically.
long date_to_ordinal(const Date& d) {
  int y = d.year, m = d.month, day = d.day;
  // Days before year.
  int yy = y - 1;
  long days = yy * 365L + yy / 4 - yy / 100 + yy / 400;
  static const int cum[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  days += cum[m - 1];
  bool leap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
  if (leap && m > 2) days += 1;
  days += day;
  return days;  // Python's date(1,1,1).toordinal() == 1; this matches.
}

Date ordinal_to_date(long ord) {
  // Inverse: find year, then month/day. Simple loop (season scan is tiny).
  long n = ord;
  int year = 1;
  for (;;) {
    bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    long ydays = leap ? 366 : 365;
    if (n <= ydays) break;
    n -= ydays;
    ++year;
  }
  bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
  static const int mdaysN[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int mdays[12];
  for (int i = 0; i < 12; ++i) mdays[i] = mdaysN[i];
  if (leap) mdays[1] = 29;
  int month = 1;
  for (int i = 0; i < 12; ++i) {
    if (n <= mdays[i]) { month = i + 1; break; }
    n -= mdays[i];
  }
  Date d{year, month, static_cast<int>(n)};
  return d;
}

bool date_le(const Date& a, const Date& b) {
  if (a.year != b.year) return a.year < b.year;
  if (a.month != b.month) return a.month < b.month;
  return a.day <= b.day;
}
bool date_ge(const Date& a, const Date& b) {
  if (a.year != b.year) return a.year > b.year;
  if (a.month != b.month) return a.month > b.month;
  return a.day >= b.day;
}
int day_of_year(const Date& d) {
  Date jan1{d.year, 1, 1};
  return static_cast<int>(date_to_ordinal(d) - date_to_ordinal(jan1)) + 1;
}

// ------------------------- Moon phase ---------------------------------
static const char* kPhaseName[] = {
    "new", "waxing_crescent", "first_quarter", "waxing_gibbous",
    "full", "waning_gibbous", "last_quarter", "waning_crescent"};
static const char* kPhaseLabel[] = {
    "New", "Waxing Crescent", "First Quarter", "Waxing Gibbous",
    "Full", "Waning Gibbous", "Last Quarter", "Waning Crescent"};

const char* moon_phase_name(MoonPhase p) { return kPhaseName[static_cast<int>(p)]; }
const char* moon_phase_label(MoonPhase p) { return kPhaseLabel[static_cast<int>(p)]; }

MoonInfo moon_phase(const Date& d) {
  double jd = julian_day_noon(d);
  double sun = sun_ecliptic_longitude(jd);
  double moon = moon_ecliptic_longitude(jd);
  double elong = norm360(moon - sun);
  double illum = (1.0 - cosd(elong)) / 2.0;
  int idx = static_cast<int>(std::floor((elong + 22.5) / 45.0)) % 8;
  if (idx < 0) idx += 8;
  MoonInfo mi;
  mi.phase = static_cast<MoonPhase>(idx);
  mi.illumination = illum;
  mi.elongation = elong;
  return mi;
}

// ------------------------- Zodiac -------------------------------------
static const char* kZodiac[] = {
    "Aries", "Taurus", "Gemini", "Cancer", "Leo", "Virgo",
    "Libra", "Scorpio", "Sagittarius", "Capricorn", "Aquarius", "Pisces"};

struct SunRange { int sm, sd, em, ed; const char* name; };
static const SunRange kSunRanges[] = {
    {3, 21, 4, 19, "Aries"},     {4, 20, 5, 20, "Taurus"},
    {5, 21, 6, 20, "Gemini"},    {6, 21, 7, 22, "Cancer"},
    {7, 23, 8, 22, "Leo"},       {8, 23, 9, 22, "Virgo"},
    {9, 23, 10, 22, "Libra"},    {10, 23, 11, 21, "Scorpio"},
    {11, 22, 12, 21, "Sagittarius"}, {12, 22, 1, 19, "Capricorn"},
    {1, 20, 2, 18, "Aquarius"},  {2, 19, 3, 20, "Pisces"}};

static bool md_ge(int m, int d, int m2, int d2) {
  return (m > m2) || (m == m2 && d >= d2);
}
static bool md_le(int m, int d, int m2, int d2) {
  return (m < m2) || (m == m2 && d <= d2);
}

const char* sun_sign(const Date& birth) {
  int m = birth.month, dd = birth.day;
  for (const auto& r : kSunRanges) {
    if (md_le(r.sm, r.sd, r.em, r.ed)) {  // range within one year
      if (md_ge(m, dd, r.sm, r.sd) && md_le(m, dd, r.em, r.ed)) return r.name;
    } else {  // wrapping range (Capricorn)
      if (md_ge(m, dd, r.sm, r.sd) || md_le(m, dd, r.em, r.ed)) return r.name;
    }
  }
  return "Capricorn";
}

const char* moon_sign(const Date& d) {
  double jd = julian_day_noon(d);
  double lon = moon_ecliptic_longitude(jd);
  int idx = static_cast<int>(std::floor(lon / 30.0)) % 12;
  if (idx < 0) idx += 12;
  return kZodiac[idx];
}

// ------------------------- Retrograde tables --------------------------
// Same data as RETROGRADES in astronomy.py, inclusive [start, end].
struct Window { Date start, end; };

static const Window kMercury[] = {
    {{2024, 4, 1}, {2024, 4, 25}},   {{2024, 8, 5}, {2024, 8, 28}},
    {{2024, 11, 26}, {2024, 12, 15}}, {{2025, 3, 15}, {2025, 4, 7}},
    {{2025, 7, 18}, {2025, 8, 11}},  {{2025, 11, 9}, {2025, 11, 29}},
    {{2026, 2, 26}, {2026, 3, 20}},  {{2026, 6, 29}, {2026, 7, 23}},
    {{2026, 10, 24}, {2026, 11, 13}}, {{2027, 2, 9}, {2027, 3, 3}},
    {{2027, 6, 10}, {2027, 7, 4}},   {{2027, 10, 7}, {2027, 10, 28}},
    {{2028, 1, 24}, {2028, 2, 14}},  {{2028, 5, 21}, {2028, 6, 13}},
    {{2028, 9, 19}, {2028, 10, 11}}, {{2029, 1, 7}, {2029, 1, 27}},
    {{2029, 5, 1}, {2029, 5, 25}},   {{2029, 8, 31}, {2029, 9, 23}},
    {{2029, 12, 21}, {2030, 1, 10}}, {{2030, 4, 13}, {2030, 5, 6}},
    {{2030, 8, 13}, {2030, 9, 5}},   {{2030, 12, 5}, {2030, 12, 24}}};

static const Window kVenus[] = {
    {{2025, 3, 1}, {2025, 4, 12}},   {{2026, 10, 3}, {2026, 11, 13}},
    {{2028, 5, 12}, {2028, 6, 24}},  {{2029, 12, 19}, {2030, 1, 29}}};

static const Window kMars[] = {
    {{2024, 12, 6}, {2025, 2, 23}},  {{2027, 1, 10}, {2027, 4, 1}},
    {{2029, 3, 1}, {2029, 5, 20}}};

static const Window kJupiter[] = {
    {{2024, 10, 9}, {2025, 2, 4}},   {{2025, 11, 11}, {2026, 3, 11}},
    {{2026, 12, 13}, {2027, 4, 13}}, {{2028, 1, 13}, {2028, 5, 14}},
    {{2029, 2, 12}, {2029, 6, 14}},  {{2030, 3, 15}, {2030, 7, 15}}};

static const Window kSaturn[] = {
    {{2024, 6, 29}, {2024, 11, 15}}, {{2025, 7, 13}, {2025, 11, 28}},
    {{2026, 7, 28}, {2026, 12, 12}}, {{2027, 8, 11}, {2027, 12, 26}},
    {{2028, 8, 23}, {2029, 1, 7}},   {{2029, 9, 5}, {2030, 1, 20}},
    {{2030, 9, 18}, {2031, 2, 2}}};

struct PlanetTable { const Window* w; int n; };
static PlanetTable table_for(Planet p) {
  switch (p) {
    case Planet::Mercury: return {kMercury, (int)(sizeof(kMercury)/sizeof(Window))};
    case Planet::Venus:   return {kVenus,   (int)(sizeof(kVenus)/sizeof(Window))};
    case Planet::Mars:    return {kMars,    (int)(sizeof(kMars)/sizeof(Window))};
    case Planet::Jupiter: return {kJupiter, (int)(sizeof(kJupiter)/sizeof(Window))};
    case Planet::Saturn:  return {kSaturn,  (int)(sizeof(kSaturn)/sizeof(Window))};
  }
  return {nullptr, 0};
}

static const char* kPlanetName[] = {"mercury", "venus", "mars", "jupiter", "saturn"};
static const char* kPlanetLabel[] = {"Mercury", "Venus", "Mars", "Jupiter", "Saturn"};
const char* planet_name(Planet p) { return kPlanetName[static_cast<int>(p)]; }
const char* planet_label(Planet p) { return kPlanetLabel[static_cast<int>(p)]; }

bool is_retrograde(Planet p, const Date& d) {
  PlanetTable t = table_for(p);
  for (int i = 0; i < t.n; ++i) {
    if (date_ge(d, t.w[i].start) && date_le(d, t.w[i].end)) return true;
  }
  return false;
}

PlanetaryEmphasis planetary_emphasis(const Date& d) {
  static const Planet priority[] = {Planet::Mercury, Planet::Venus, Planet::Mars,
                                     Planet::Jupiter, Planet::Saturn};
  for (Planet p : priority) {
    if (is_retrograde(p, d)) {
      int idx = static_cast<int>(p) * 2 + 0;  // retrograde slot
      return {p, true, idx};
    }
  }
  // No retrogrades: rotate by day-of-year through the same 5 planets.
  int doy = day_of_year(d);
  Planet p = priority[doy % 5];
  int idx = static_cast<int>(p) * 2 + 1;  // direct slot
  return {p, false, idx};
}

// ------------------------- Season marker ------------------------------
static Date season_event_date(int year, int target_longitude) {
  int approx_month;
  switch (target_longitude) {
    case 0: approx_month = 3; break;
    case 90: approx_month = 6; break;
    case 180: approx_month = 9; break;
    default: approx_month = 12; break;  // 270
  }
  Date approx{year, approx_month, 20};
  long base = date_to_ordinal(approx);
  Date best = approx;
  double best_err = 999.0;
  for (int off = -20; off <= 20; ++off) {
    Date cand = ordinal_to_date(base + off);
    double jd = julian_day_noon(cand);
    double lon = sun_ecliptic_longitude(jd);
    double err = std::fabs(std::fmod(lon - target_longitude + 180.0, 360.0) - 180.0);
    if (err < best_err) { best_err = err; best = cand; }
  }
  return best;
}

SeasonMarker season_marker(const Date& d, int window_days) {
  struct Ev { int lon; const char* name; SeasonKind kind; };
  static const Ev events[] = {
      {0, "spring_equinox", SeasonKind::Equinox},
      {90, "summer_solstice", SeasonKind::Solstice},
      {180, "autumn_equinox", SeasonKind::Equinox},
      {270, "winter_solstice", SeasonKind::Solstice}};
  long today = date_to_ordinal(d);
  int best_abs = 1 << 30, best_signed = 0;
  const char* best_name = "";
  SeasonKind best_kind = SeasonKind::Ordinary;
  for (int y = d.year - 1; y <= d.year + 1; ++y) {
    for (const auto& e : events) {
      Date ev = season_event_date(y, e.lon);
      int delta = static_cast<int>(date_to_ordinal(ev) - today);
      int adelta = delta < 0 ? -delta : delta;
      if (adelta < best_abs) {
        best_abs = adelta; best_signed = delta; best_name = e.name; best_kind = e.kind;
      }
    }
  }
  if (best_abs <= window_days) return {best_kind, best_name, best_signed};
  return {SeasonKind::Ordinary, "", best_signed};
}

// ------------------------- Seed (FNV-1a, must match Python) -----------
uint64_t daily_seed(const Date& target, const Date* birth) {
  int parts[6];
  parts[0] = target.year; parts[1] = target.month; parts[2] = target.day;
  if (birth) { parts[3] = birth->year; parts[4] = birth->month; parts[5] = birth->day; }
  else { parts[3] = 0; parts[4] = 0; parts[5] = 0; }

  uint64_t h = 1469598103934665603ULL;  // FNV offset basis
  const uint64_t prime = 1099511628211ULL;
  for (int i = 0; i < 6; ++i) {
    uint32_t v = static_cast<uint32_t>(parts[i]) & 0xFFFFFFFFu;
    for (int b = 0; b < 4; ++b) {
      h ^= (v & 0xFFu);
      h *= prime;
      v >>= 8;
    }
  }
  return h & ((1ULL << 63) - 1);
}

// ------------------------- Assemble -----------------------------------
SkyState sky_state(const Date& target, const Date* birth) {
  SkyState s;
  s.target = target;
  s.moon = moon_phase(target);
  s.planet = planetary_emphasis(target);
  s.season = season_marker(target);
  s.sun_sign = birth ? sun_sign(*birth) : nullptr;
  s.moon_sign = moon_sign(target);
  s.seed = daily_seed(target, birth);
  return s;
}

}  // namespace oracle
