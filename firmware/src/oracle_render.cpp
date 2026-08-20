// oracle_render.cpp -- framebuffer primitives, text, layout, BMP export.
#include "oracle_render.h"
#include "font5x7.h"

#include <cstdio>
#include <cstring>

namespace oracle {

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
namespace {
// A simple moon disc with a shadow terminator, sized to `r` radius at (cx,cy).
// `illum` 0..1 controls how much of the disc is lit (white) vs dark (black).
// We draw the dark part filled black over a black-outlined white disc.
void draw_moon_icon(Frame* f, int cx, int cy, int r, double illum,
                    bool waxing) {
  // Outline circle.
  for (int yy = -r; yy <= r; ++yy) {
    for (int xx = -r; xx <= r; ++xx) {
      int d2 = xx * xx + yy * yy;
      if (d2 <= r * r) {
        bool dark;
        // Terminator: a vertical-ish split scaled by illumination.
        // frac in [-1,1]; boundary x = r*(1-2*illum).
        double bx = r * (1.0 - 2.0 * illum);
        if (waxing) {
          dark = (xx < bx);   // lit on the right
        } else {
          dark = (xx > -bx);  // lit on the left
        }
        // Draw dark pixels black; lit pixels white but keep a thin outline.
        if (dark) frame_set(f, cx + xx, cy + yy, true);
      }
    }
  }
  // Thin outline so the lit half is visible against white.
  const int STEPS = 720;
  for (int i = 0; i < STEPS; ++i) {
    double a = (6.28318530718 * i) / STEPS;
    int px = cx + (int)(r * __builtin_cos(a));
    int py = cy + (int)(r * __builtin_sin(a));
    frame_set(f, px, py, true);
  }
}

bool phase_is_waxing(MoonPhase p) {
  switch (p) {
    case MoonPhase::New:
    case MoonPhase::WaxingCrescent:
    case MoonPhase::FirstQuarter:
    case MoonPhase::WaxingGibbous:
    case MoonPhase::Full:
      return true;
    default:
      return false;
  }
}
}  // namespace

// ------------------------- The Oracle layout --------------------------
// 5 columns across 1360 px. We use column boundaries as guides but let the
// message span the wide middle for readability.
void render_oracle(Frame* f, const SkyState& st, const char* message,
                   const char* name, const char* place, const char* crystal,
                   const char* crystal_note) {
  frame_clear(f, /*black=*/false);

  const int W = f->w, H = f->h;
  const int col_w = W / kCols;  // 272

  // ---- Outer border + header band ----
  frame_rect(f, 4, 4, W - 8, H - 8, true);
  const int header_h = 78;
  frame_hline(f, 4, W - 5, header_h, true);

  // Header: title + date + place.
  char header[160];
  if (name && *name)
    std::snprintf(header, sizeof(header), "THE ORACLE  -  for %s", name);
  else
    std::snprintf(header, sizeof(header), "THE ORACLE");
  draw_text(f, 24, 20, header, 4, true);

  // Date line (right-aligned-ish in the header).
  static const char* kMon[] = {"Jan","Feb","Mar","Apr","May","Jun",
                               "Jul","Aug","Sep","Oct","Nov","Dec"};
  char dateline[96];
  const char* mon = (st.target.month >= 1 && st.target.month <= 12)
                        ? kMon[st.target.month - 1] : "";
  if (place && *place)
    std::snprintf(dateline, sizeof(dateline), "%02d %s %d  -  %s",
                  st.target.day, mon, st.target.year, place);
  else
    std::snprintf(dateline, sizeof(dateline), "%02d %s %d",
                  st.target.day, mon, st.target.year);
  int dw = text_width(dateline, 2);
  draw_text(f, W - 24 - dw, 30, dateline, 2, true);

  // ---- Column 1: MOON panel ----
  int c1x = 12;
  int body_top = header_h + 22;
  draw_text(f, c1x, body_top, "MOON", 3, true);

  // Moon icon.
  int icon_cx = c1x + 70, icon_cy = body_top + 96, icon_r = 52;
  draw_moon_icon(f, icon_cx, icon_cy, icon_r, st.moon.illumination,
                 phase_is_waxing(st.moon.phase));

  // Phase label + illumination + moon sign.
  char l1[64], l2[64], l3[64];
  std::snprintf(l1, sizeof(l1), "%s", moon_phase_label(st.moon.phase));
  std::snprintf(l2, sizeof(l2), "%d%% lit", (int)(st.moon.illumination * 100 + 0.5));
  std::snprintf(l3, sizeof(l3), "in %s", st.moon_sign);
  int ty = icon_cy + icon_r + 18;
  ty = draw_wrapped(f, c1x, ty, col_w - 20, l1, 2, 4, true);
  draw_text(f, c1x, ty + 2, l2, 2, true);
  draw_text(f, c1x, ty + 24, l3, 2, true);

  // Column divider after col 1.
  frame_vline(f, col_w, header_h + 8, H - 12, true);

  // ---- Columns 2-4: the MESSAGE (wide reading area) ----
  int msg_x = col_w + 24;
  int msg_w = col_w * 3 - 40;  // spans columns 2,3,4
  int msg_y = body_top + 6;
  draw_text(f, msg_x, msg_y, "TODAY", 3, true);
  draw_wrapped(f, msg_x, msg_y + 40, msg_w, message, 3, 8, true);

  // Column divider before col 5.
  frame_vline(f, col_w * 4, header_h + 8, H - 12, true);

  // ---- Column 5: SKY facts + crystal ----
  int c5x = col_w * 4 + 16;
  int c5w = col_w - 28;
  int sy = body_top;
  draw_text(f, c5x, sy, "SKY", 3, true);
  sy += 40;

  char pf[80];
  std::snprintf(pf, sizeof(pf), "%s %s", planet_label(st.planet.planet),
                st.planet.retrograde ? "retrograde" : "direct");
  sy = draw_wrapped(f, c5x, sy, c5w, pf, 2, 4, true);
  sy += 6;

  if (st.season.kind != SeasonKind::Ordinary) {
    // Convert "winter_solstice" -> "Winter Solstice".
    char season_pretty[48];
    std::snprintf(season_pretty, sizeof(season_pretty), "%s", st.season.name);
    for (char* p = season_pretty; *p; ++p) {
      if (*p == '_') *p = ' ';
      if (p == season_pretty || p[-1] == ' ') {
        if (*p >= 'a' && *p <= 'z') *p = (char)(*p - 'a' + 'A');
      }
    }
    sy = draw_wrapped(f, c5x, sy, c5w, season_pretty, 2, 4, true);
    sy += 6;
  }

  if (st.sun_sign) {
    char sun[48];
    std::snprintf(sun, sizeof(sun), "Sun in %s", st.sun_sign);
    sy = draw_wrapped(f, c5x, sy, c5w, sun, 2, 4, true);
    sy += 10;
  }

  if (crystal && *crystal) {
    sy += 6;
    frame_hline(f, c5x, c5x + c5w, sy, true);
    sy += 12;
    char cz[48];
    // Uppercase-first crystal name for the label.
    std::snprintf(cz, sizeof(cz), "%s", crystal);
    if (cz[0] >= 'a' && cz[0] <= 'z') cz[0] = (char)(cz[0] - 'a' + 'A');
    sy = draw_wrapped(f, c5x, sy, c5w, cz, 2, 4, true);
    if (crystal_note && *crystal_note) {
      sy += 4;
      draw_wrapped(f, c5x, sy, c5w, crystal_note, 1, 3, true);
    }
  }
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
