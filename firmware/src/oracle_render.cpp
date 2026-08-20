// oracle_render.cpp -- framebuffer primitives, text, layout, BMP export.
#include "oracle_render.h"
#include "font5x7.h"
#include "oracle_text.h"

#include <cstdio>
#include <cstring>

namespace oracle {

namespace {
// Copy `src` into `dst`, converting the ASCII " -- " fragment separator into a
// spaced en dash (" - " visually via a real punctuation glyph is nicer, but our
// ASCII-subset font has the hyphen; we render " – " using the en dash char if
// present, else fall back to a single hyphen with spaces). Keeps text elegant.
void prettify_dashes(const char* src, char* dst, size_t cap) {
  size_t w = 0;
  for (size_t i = 0; src[i] && w + 4 < cap;) {
    if (src[i] == '-' && src[i + 1] == '-') {
      // " -- " -> " - " (single hyphen, serif renders it as a fine dash).
      dst[w++] = '-';
      i += 2;
    } else {
      dst[w++] = src[i++];
    }
  }
  dst[w] = '\0';
}
}  // namespace

// ------------------------- Framebuffer --------------------------------
void frame_clear(Frame* f, bool black) {
  std::memset(f->buf, black ? 0xFF : 0x00, (size_t)f->stride * f->h);
}

void frame_init(Frame* f, uint8_t* buf) {
  f->buf = buf;
  f->w = kCanvasW;
  f->h = kCanvasH;
  f->stride = kRowBytes;
  frame_clear(f, /*black=*/false);
}

void frame_set(Frame* f, int x, int y, bool black) {
  if (x < 0 || y < 0 || x >= f->w || y >= f->h) return;
  uint8_t& byte = f->buf[(size_t)y * f->stride + (x >> 3)];
  uint8_t mask = 0x80 >> (x & 7);  // MSB-first
  if (black) byte |= mask; else byte &= ~mask;
}

void frame_hline(Frame* f, int x0, int x1, int y, bool black) {
  if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
  for (int x = x0; x <= x1; ++x) frame_set(f, x, y, black);
}
void frame_vline(Frame* f, int x, int y0, int y1, bool black) {
  if (y0 > y1) { int t = y0; y0 = y1; y1 = t; }
  for (int y = y0; y <= y1; ++y) frame_set(f, x, y, black);
}
void frame_rect(Frame* f, int x, int y, int w, int h, bool black) {
  frame_hline(f, x, x + w - 1, y, black);
  frame_hline(f, x, x + w - 1, y + h - 1, black);
  frame_vline(f, x, y, y + h - 1, black);
  frame_vline(f, x + w - 1, y, y + h - 1, black);
}
void frame_fill_rect(Frame* f, int x, int y, int w, int h, bool black) {
  for (int yy = y; yy < y + h; ++yy)
    for (int xx = x; xx < x + w; ++xx) frame_set(f, xx, yy, black);
}

// ------------------------- Text ---------------------------------------
int draw_char(Frame* f, int x, int y, char c, int scale, bool black) {
  const uint8_t* g = glyph_for(c);
  for (int col = 0; col < kGlyphW; ++col) {
    uint8_t bits = g[col];
    for (int row = 0; row < kGlyphH; ++row) {
      if (bits & (1 << row)) {
        // Scaled block.
        frame_fill_rect(f, x + col * scale, y + row * scale, scale, scale, black);
      }
    }
  }
  return (kGlyphW + 1) * scale;  // 1px inter-glyph gap, scaled
}

int draw_text(Frame* f, int x, int y, const char* s, int scale, bool black) {
  int cx = x;
  for (; *s; ++s) cx += draw_char(f, cx, y, *s, scale, black);
  return cx;
}

int text_width(const char* s, int scale) {
  int n = 0;
  for (; *s; ++s) ++n;
  return n * (kGlyphW + 1) * scale;
}

// Word wrap. line_gap is extra vertical space between lines (px).
int draw_wrapped(Frame* f, int x, int y, int box_w, const char* s, int scale,
                 int line_gap, bool black) {
  const int char_adv = (kGlyphW + 1) * scale;
  const int line_h = kGlyphH * scale + line_gap;
  int cx = x;
  int cy = y;

  const char* word = s;
  while (*word) {
    // Find the end of the next word.
    const char* end = word;
    while (*end && *end != ' ') ++end;
    int wlen = (int)(end - word);
    int wpx = wlen * char_adv;

    // Wrap if the word won't fit on the current line (and we're not at line start).
    if (cx > x && cx - x + wpx > box_w) {
      cx = x;
      cy += line_h;
    }
    // Draw the word.
    for (const char* p = word; p < end; ++p) cx += draw_char(f, cx, cy, *p, scale, black);
    // Space after word.
    if (*end == ' ') {
      cx += char_adv;
      ++end;
    }
    word = end;
  }
  return cy + line_h;
}

// ------------------------- Icons (drawn, not font) --------------------
// ------------------------- The Oracle layout --------------------------
// Clean, vision-style panel: no borders, no dividers. Left-aligned stack of
//   DATE  (large serif)
//   MOON + PLANET  (one quiet subtitle line)
//   PHRASE  (the message, generous serif)
// `name` (woven into the phrase already), `place`, `crystal`, `crystal_note`
// are accepted for API stability but intentionally not drawn -- the panel
// stays spare, matching the product render.
void render_oracle(Frame* f, const SkyState& st, const char* message,
                   const char* name, const char* place, const char* crystal,
                   const char* crystal_note) {
  (void)name; (void)place; (void)crystal; (void)crystal_note;
  frame_clear(f, /*black=*/false);

  const int W = f->w;
  text_init();

  // Type sizes (px), tuned for the 1360x480 panel + EB Garamond metrics.
  const int kDatePx     = 68;   // "20 August"
  const int kSubtitlePx = 27;   // "Moon in Gemini . Jupiter retrograde"
  const int kPhrasePx   = 40;   // the message

  const int margin_x = 96;      // left margin
  const int box_w = W - 2 * margin_x;

  // ---- DATE ----
  static const char* kMon[] = {"January","February","March","April","May","June",
                               "July","August","September","October","November","December"};
  const char* mon = (st.target.month >= 1 && st.target.month <= 12)
                        ? kMon[st.target.month - 1] : "";
  char dateline[48];
  std::snprintf(dateline, sizeof(dateline), "%d %s", st.target.day, mon);

  int y = 70;
  ttf_draw(f, margin_x, y, dateline, Face::SerifSemiBold, kDatePx, true);
  y += ttf_line_height(Face::SerifSemiBold, kDatePx);

  // ---- MOON + PLANET subtitle ----
  // e.g. "Moon in Sagittarius . Saturn retrograde" (middot as separator).
  char subtitle[128];
  std::snprintf(subtitle, sizeof(subtitle), "Moon in %s \xC2\xB7 %s %s",
                st.moon_sign, planet_label(st.planet.planet),
                st.planet.retrograde ? "retrograde" : "direct");
  y += 10;
  ttf_draw(f, margin_x, y, subtitle, Face::Serif, kSubtitlePx, true);
  y += ttf_line_height(Face::Serif, kSubtitlePx);

  // ---- PHRASE ----
  char pretty_msg[1024];
  prettify_dashes(message, pretty_msg, sizeof(pretty_msg));
  y += 48;  // breathing room before the phrase
  ttf_draw_wrapped(f, margin_x, y, box_w, pretty_msg, Face::Serif,
                   kPhrasePx, 16, true);
}

// ------------------------- BMP export ---------------------------------
// Writes a 1-bit BMP. In BMP 1bpp, bit set = palette index 1. We set palette
// 0 = black, 1 = white, and INVERT our buffer (buf bit set == black) so the
// image looks right in a viewer. Rows are bottom-up and padded to 4 bytes.
bool write_bmp(const char* path, const Frame* f) {
  FILE* fp = std::fopen(path, "wb");
  if (!fp) return false;

  int w = f->w, h = f->h;
  int row_pad = ((w + 31) / 32) * 4;   // 4-byte aligned bytes per row
  int pixel_bytes = row_pad * h;
  int palette_bytes = 2 * 4;           // two palette entries
  int header_bytes = 14 + 40;          // BITMAPFILEHEADER + BITMAPINFOHEADER
  int offset = header_bytes + palette_bytes;
  int filesize = offset + pixel_bytes;

  auto w16 = [&](uint16_t v) { std::fputc(v & 0xFF, fp); std::fputc((v >> 8) & 0xFF, fp); };
  auto w32 = [&](uint32_t v) {
    std::fputc(v & 0xFF, fp); std::fputc((v >> 8) & 0xFF, fp);
    std::fputc((v >> 16) & 0xFF, fp); std::fputc((v >> 24) & 0xFF, fp);
  };

  // BITMAPFILEHEADER
  std::fputc('B', fp); std::fputc('M', fp);
  w32(filesize); w16(0); w16(0); w32(offset);
  // BITMAPINFOHEADER
  w32(40); w32((uint32_t)w); w32((uint32_t)h); w16(1); w16(1);
  w32(0); w32((uint32_t)pixel_bytes); w32(2835); w32(2835); w32(2); w32(0);
  // Palette: index 0 = black, index 1 = white (BGRA).
  std::fputc(0, fp); std::fputc(0, fp); std::fputc(0, fp); std::fputc(0, fp);
  std::fputc(255, fp); std::fputc(255, fp); std::fputc(255, fp); std::fputc(0, fp);

  // Pixel data, bottom-up. Our buffer: bit set == black. BMP index: 0==black.
  // So output bit = !our_bit  => white where we didn't draw.
  uint8_t* rowbuf = new uint8_t[row_pad];
  for (int y = h - 1; y >= 0; --y) {
    std::memset(rowbuf, 0, row_pad);
    const uint8_t* src = &f->buf[(size_t)y * f->stride];
    for (int xbyte = 0; xbyte < f->stride && xbyte < row_pad; ++xbyte) {
      // Invert: BMP 1 == white, our 1 == black.
      rowbuf[xbyte] = (uint8_t)(~src[xbyte]);
    }
    std::fwrite(rowbuf, 1, row_pad, fp);
  }
  delete[] rowbuf;
  std::fclose(fp);
  return true;
}

}  // namespace oracle
