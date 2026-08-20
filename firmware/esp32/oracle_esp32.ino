// oracle_esp32.ino -- The Oracle on an ESP32 driving a 1360x480 1-bit e-ink.
//
// This is the thin on-device wrapper. All the real logic lives in the shared,
// portable sources, which compile unchanged here and on desktop:
//     ../src/oracle_astro.{h,cpp}
//     ../src/oracle_composer.{h,cpp}   (+ oracle_fragments.h, generated)
//     ../src/oracle_text.{h,cpp}        (EB Garamond serif via stb_truetype)
//     ../src/oracle_font_ttf.h          (embedded serif, generated)
//     ../third_party/stb_truetype.h
//     ../src/oracle_render.{h,cpp}      (framebuffer + layout)
//
// For Arduino IDE: put the .ino in a folder and copy (or symlink) the src/*
// files next to it, OR use PlatformIO with a lib_extra_dirs pointing at src/.
//
// Panel: this sketch assumes a GxEPD2 driver whose native buffer is MSB-first
// 1bpp, which matches our Frame layout, so we can push the whole framebuffer
// in one shot. Swap in your exact panel class below.
//
// Memory: the framebuffer is 1360*480/8 = 81600 bytes. That fits in ESP32
// SRAM, but if your board is tight, enable PSMALLOC / put `framebuf` in PSRAM
// (ESP32-WROVER) with a heap_caps_malloc(..., MALLOC_CAP_SPIRAM).

#include <GxEPD2_BW.h>          // Adafruit-GFX-based e-ink driver
#include <RTClib.h>             // for a real date; swap for your RTC/NTP

#include "oracle_astro.h"
#include "oracle_composer.h"
#include "oracle_render.h"

using namespace oracle;

// ---- Panel wiring (EDIT for your board) ------------------------------
// Example placeholder class/pins for a large mono panel. Replace with the
// exact GxEPD2 class for your 1360x480 controller (e.g. a tiled driver).
#define EPD_CS   5
#define EPD_DC   17
#define EPD_RST  16
#define EPD_BUSY 4

// NOTE: pick the GxEPD2 class matching your controller. This is illustrative.
GxEPD2_BW<GxEPD2_1360x480_TEMPLATE, GxEPD2_1360x480_TEMPLATE::HEIGHT>
    display(GxEPD2_1360x480_TEMPLATE(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

RTC_DS3231 rtc;

// ---- Static framebuffer (81600 bytes) --------------------------------
// Kept in .bss so we don't fragment the heap. Move to PSRAM if needed.
static uint8_t framebuf[kFrameBytes];

// ---- User profile (persist in NVS/flash in a real build) -------------
static const Date  kBirth   = {1990, 6, 15};
static const char* kName    = "Sam";
static const char* kPlace   = "Lyon";
static const char* kCrystal = "amethyst";
static const char* kCrystalNote = "a stone for calm and clear thinking";

// Persist the collision-avoidance history across days in RTC slow memory so a
// deep-sleep/wake cycle keeps its 30-day window.
RTC_DATA_ATTR MessageHistory g_history;
RTC_DATA_ATTR bool g_history_ready = false;

static Date today_from_rtc() {
  DateTime now = rtc.now();
  Date d;
  d.year = now.year();
  d.month = now.month();
  d.day = now.day();
  return d;
}

void render_and_show() {
  if (!g_history_ready) { history_init(&g_history); g_history_ready = true; }

  Date target = today_from_rtc();
  SkyState st = sky_state(target, &kBirth);

  char message[1024];
  generate_daily_message(target, &kBirth, kName, &g_history, message,
                         sizeof(message));

  Frame f;
  frame_init(&f, framebuf);
  render_oracle(&f, st, message, kName, kPlace, kCrystal, kCrystalNote);

  // Blit the 1bpp framebuffer to the panel. GxEPD2 uses paged drawing; we push
  // our full buffer each page window. Our buffer bit set == black, which is
  // GxEPD2's convention for GxEPD2_BW when using drawBitmap with mode inverted
  // as needed. drawInvertedBitmap draws set-bit as black.
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.drawInvertedBitmap(0, 0, framebuf, kCanvasW, kCanvasH, GxEPD_BLACK);
  } while (display.nextPage());
  display.hibernate();
}

void setup() {
  Serial.begin(115200);
  if (!rtc.begin()) {
    Serial.println("RTC not found; using compile-time date fallback.");
  }
  display.init(115200);

  render_and_show();

  // Deep sleep until tomorrow. Wake once a day to refresh the reading; e-ink
  // holds the image with zero power in between. ~24h in microseconds.
  const uint64_t kOneDayUs = 24ULL * 60ULL * 60ULL * 1000000ULL;
  esp_sleep_enable_timer_wakeup(kOneDayUs);
  esp_deep_sleep_start();
}

void loop() {
  // Never reached: setup() deep-sleeps. Wake re-enters setup().
}
