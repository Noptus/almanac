// oracle_text.cpp -- stb_truetype rasteriser + 1-bit dithering.
#include "oracle_text.h"
#include "oracle_font_ttf.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../third_party/stb_truetype.h"

#include <cstring>

namespace oracle {

namespace {
stbtt_fontinfo g_regular;
stbtt_fontinfo g_semibold;
bool g_ready = false;

stbtt_fontinfo* face_info(Face face) {
  return face == Face::SerifSemiBold ? &g_semibold : &g_regular;
}

// 8x8 Bayer ordered-dither threshold matrix, normalised to 1..64.
const int kBayer8[8][8] = {
    { 0, 48, 12, 60,  3, 51, 15, 63},
    {32, 16, 44, 28, 35, 19, 47, 31},
    { 8, 56,  4, 52, 11, 59,  7, 55},
    {40, 24, 36, 20, 43, 27, 39, 23},
    { 2, 50, 14, 62,  1, 49, 13, 61},
    {34, 18, 46, 30, 33, 17, 45, 29},
    {10, 58,  6, 54,  9, 57,  5, 53},
    {42, 26, 38, 22, 41, 25, 37, 21},
};
}  // namespace

void text_init() {
  if (g_ready) return;
  stbtt_InitFont(&g_regular, kEBGaramondRegular,
                 stbtt_GetFontOffsetForIndex(kEBGaramondRegular, 0));
  stbtt_InitFont(&g_semibold, kEBGaramondSemiBold,
                 stbtt_GetFontOffsetForIndex(kEBGaramondSemiBold, 0));
  g_ready = true;
}

int ttf_line_height(Face face, int px) {
  text_init();
  stbtt_fontinfo* fi = face_info(face);
  float scale = stbtt_ScaleForPixelHeight(fi, (float)px);
  int ascent, descent, linegap;
  stbtt_GetFontVMetrics(fi, &ascent, &descent, &linegap);
  return (int)((ascent - descent) * scale + 0.5f);
}

int ttf_width(const char* s, Face face, int px) {
  text_init();
  stbtt_fontinfo* fi = face_info(face);
  float scale = stbtt_ScaleForPixelHeight(fi, (float)px);
  float x = 0.0f;
  for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
    int adv, lsb;
    stbtt_GetCodepointHMetrics(fi, *p, &adv, &lsb);
    x += adv * scale;
  }
  return (int)(x + 0.5f);
}

// When a glyph is small, ordered dithering adds speckle that reads as noise;
// a flat coverage threshold is crisper. Above this pixel height we dither
// (helps big text look smooth); at or below it we hard-threshold.
static const int kDitherMinPx = 30;

// Draw one glyph. (x, y) is the top-left of the text box; we place glyphs on a
// baseline computed from the ascent so callers can think in top-left terms.
static float draw_glyph(Frame* f, float pen_x, int top_y, int codepoint,
                        stbtt_fontinfo* fi, float scale, int ascent_px,
                        bool black, bool dither) {
  int adv, lsb;
  stbtt_GetCodepointHMetrics(fi, codepoint, &adv, &lsb);

  int gw, gh, gx_off, gy_off;
  unsigned char* bmp = stbtt_GetCodepointBitmap(
      fi, scale, scale, codepoint, &gw, &gh, &gx_off, &gy_off);

  if (bmp) {
    int x0 = (int)(pen_x + 0.5f) + gx_off;
    int y0 = top_y + ascent_px + gy_off;  // gy_off is negative above baseline
    for (int yy = 0; yy < gh; ++yy) {
      for (int xx = 0; xx < gw; ++xx) {
        int cov = bmp[yy * gw + xx];  // 0..255 coverage
        if (cov == 0) continue;
        int px = x0 + xx, py = y0 + yy;
        int thr;
        if (dither) {
          // Ordered-dither: coverage vs Bayer matrix scaled to 0..255.
          thr = (kBayer8[py & 7][px & 7] * 255) / 64;
        } else {
          // Flat threshold at ~50% coverage for crisp small text.
          thr = 128;
        }
        if (cov > thr) frame_set(f, px, py, black);
      }
    }
    stbtt_FreeBitmap(bmp, nullptr);
  }
  return pen_x + adv * scale;
}

int ttf_draw(Frame* f, int x, int y, const char* s, Face face, int px,
             bool black) {
  text_init();
  stbtt_fontinfo* fi = face_info(face);
  float scale = stbtt_ScaleForPixelHeight(fi, (float)px);
  int ascent, descent, linegap;
  stbtt_GetFontVMetrics(fi, &ascent, &descent, &linegap);
  int ascent_px = (int)(ascent * scale + 0.5f);

  bool dither = px > kDitherMinPx;
  float pen = (float)x;
  for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
    pen = draw_glyph(f, pen, y, *p, fi, scale, ascent_px, black, dither);
  }
  return (int)(pen + 0.5f);
}

int ttf_draw_wrapped(Frame* f, int x, int y, int box_w, const char* s,
                     Face face, int px, int leading, bool black) {
  text_init();
  stbtt_fontinfo* fi = face_info(face);
  float scale = stbtt_ScaleForPixelHeight(fi, (float)px);
  int ascent, descent, linegap;
  stbtt_GetFontVMetrics(fi, &ascent, &descent, &linegap);
  int ascent_px = (int)(ascent * scale + 0.5f);
  int line_h = (int)((ascent - descent) * scale + 0.5f) + leading;

  int space_adv;
  {
    int a, l;
    stbtt_GetCodepointHMetrics(fi, ' ', &a, &l);
    space_adv = (int)(a * scale + 0.5f);
  }

  bool dither = px > kDitherMinPx;
  int cy = y;
  float pen = (float)x;

  const char* word = s;
  while (*word) {
    const char* end = word;
    while (*end && *end != ' ') ++end;

    // Measure this word.
    float wpx = 0.0f;
    for (const unsigned char* p = (const unsigned char*)word;
         p < (const unsigned char*)end; ++p) {
      int adv, lsb;
      stbtt_GetCodepointHMetrics(fi, *p, &adv, &lsb);
      wpx += adv * scale;
    }

    // Wrap if needed (not at line start).
    if (pen > (float)x && (pen - x) + wpx > (float)box_w) {
      cy += line_h;
      pen = (float)x;
    }

    // Draw the word.
    for (const unsigned char* p = (const unsigned char*)word;
         p < (const unsigned char*)end; ++p) {
      pen = draw_glyph(f, pen, cy, *p, fi, scale, ascent_px, black, dither);
    }

    // Space.
    if (*end == ' ') {
      pen += space_adv;
      ++end;
    }
    word = end;
  }
  return cy + line_h;
}

}  // namespace oracle
