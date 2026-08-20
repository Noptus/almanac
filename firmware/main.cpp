// main.cpp -- desktop harness for The Oracle C++ core.
//
// Produces the same reading as the Python CLI and writes a 1360x480 1-bit BMP
// preview of exactly what the e-ink panel would show. This is the "portable"
// half of the deliverable; esp32/oracle_esp32.ino is the on-device half and
// reuses the identical oracle_* sources.
//
// Build (desktop):
//   c++ -std=c++17 -O2 -o oracle_cpp firmware/main.cpp \
//       firmware/src/oracle_astro.cpp firmware/src/oracle_composer.cpp \
//       firmware/src/oracle_render.cpp
//
// Run:
//   ./oracle_cpp --birth 1990-06-15 --name Sam --place Lyon \
//       --crystal amethyst --date 2026-08-20 --out reading.bmp
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "src/oracle_astro.h"
#include "src/oracle_composer.h"
#include "src/oracle_render.h"

using namespace oracle;

// Crystal catalogue (mirrors oracle.py). Name -> one-line intention.
struct Crystal { const char* name; const char* note; };
static const Crystal kCrystals[] = {
    {"amethyst", "a stone for calm and clear thinking"},
    {"rose quartz", "a stone for gentleness, toward others and yourself"},
    {"citrine", "a stone for warmth and quiet confidence"},
    {"obsidian", "a stone for grounding and honest reflection"},
    {"moonstone", "a stone for intuition and new beginnings"},
    {"clear quartz", "a stone for focus and starting fresh"},
    {"tigers eye", "a stone for steadiness and courage"},
    {"jade", "a stone for balance and patience"},
    {"carnelian", "a stone for motivation and warmth"},
    {"lapis lazuli", "a stone for truth and clear speech"},
};

static const char* crystal_note_for(const char* name) {
  if (!name) return nullptr;
  for (const auto& c : kCrystals)
    if (std::strcmp(c.name, name) == 0) return c.note;
  return "your chosen stone";
}

static bool parse_date(const char* s, Date* out) {
  int y, m, d;
  if (std::sscanf(s, "%d-%d-%d", &y, &m, &d) == 3) { *out = {y, m, d}; return true; }
  if (std::sscanf(s, "%d/%d/%d", &d, &m, &y) == 3) { *out = {y, m, d}; return true; }
  return false;
}

static const char* opt(int argc, char** argv, const char* key) {
  for (int i = 1; i + 1 < argc; ++i)
    if (std::strcmp(argv[i], key) == 0) return argv[i + 1];
  return nullptr;
}

int main(int argc, char** argv) {
  const char* birth_s = opt(argc, argv, "--birth");
  const char* date_s = opt(argc, argv, "--date");
  const char* name = opt(argc, argv, "--name");
  const char* place = opt(argc, argv, "--place");
  const char* crystal = opt(argc, argv, "--crystal");
  const char* out_path = opt(argc, argv, "--out");
  if (!out_path) out_path = "reading.bmp";

  Date birth{0, 0, 0};
  bool have_birth = birth_s && parse_date(birth_s, &birth);

  Date target;
  if (date_s) {
    if (!parse_date(date_s, &target)) {
      std::fprintf(stderr, "Bad --date (use YYYY-MM-DD)\n");
      return 2;
    }
  } else {
    // No <chrono> to keep it portable/deterministic in tests; default to a
    // fixed date if none given. On device you'd read the RTC here.
    target = {2026, 8, 20};
  }

  SkyState st = sky_state(target, have_birth ? &birth : nullptr);

  char message[1024];
  MessageHistory hist;
  history_init(&hist);
  generate_daily_message(target, have_birth ? &birth : nullptr, name, &hist,
                         message, sizeof(message));

  // ---- Text summary to stdout (matches the Python CLI spirit) ----
  std::printf("------------------------------------------------------------\n");
  std::printf("  THE ORACLE%s%s\n", name ? "  -  for " : "", name ? name : "");
  std::printf("  %04d-%02d-%02d%s%s\n", target.year, target.month, target.day,
              place ? "   " : "", place ? place : "");
  std::printf("------------------------------------------------------------\n");
  std::printf("  MOON   %s  -  %d%% lit  -  in %s\n",
              moon_phase_label(st.moon.phase),
              (int)(st.moon.illumination * 100 + 0.5), st.moon_sign);
  std::printf("  SKY    %s is %s today\n", planet_label(st.planet.planet),
              st.planet.retrograde ? "retrograde" : "direct");
  if (st.season.kind != SeasonKind::Ordinary)
    std::printf("  SEASON %s (%+d days)\n", st.season.name, st.season.days_away);
  if (st.sun_sign)
    std::printf("  YOU    Sun in %s%s%s\n", st.sun_sign,
                crystal ? "  -  " : "", crystal ? crystal : "");
  std::printf("------------------------------------------------------------\n");
  std::printf("  %s\n", message);
  if (crystal)
    std::printf("\n  Keep your %s close today - %s.\n", crystal,
                crystal_note_for(crystal));
  std::printf("------------------------------------------------------------\n");

  // ---- Render the e-ink image ----
  static uint8_t framebuf[kFrameBytes];
  Frame f;
  frame_init(&f, framebuf);
  render_oracle(&f, st, message, name, place, crystal, crystal_note_for(crystal));

  if (!write_bmp(out_path, &f)) {
    std::fprintf(stderr, "Failed to write %s\n", out_path);
    return 1;
  }
  std::printf("\n  Wrote %dx%d 1-bit image -> %s\n", kCanvasW, kCanvasH, out_path);
  return 0;
}
