// oracle_text.h -- antialiased TrueType text onto the 1-bit framebuffer.
//
// Uses stb_truetype to rasterise EB Garamond (embedded, ASCII subset) and
// dithers the coverage to 1bpp so a refined serif renders crisply on e-ink.
// The same code path runs on desktop and ESP32.
#ifndef ORACLE_TEXT_H
#define ORACLE_TEXT_H

#include "oracle_render.h"

namespace oracle {

enum class Face { Serif, SerifSemiBold };

// Lazily initialised font handles (call once; safe to call repeatedly).
void text_init();

// Draw a UTF-8-ish (ASCII) string at a baseline-independent top-left (x, y),
// where `px` is the cap-ish pixel height. Returns the advance width in pixels.
// Non-wrapping.
int ttf_draw(Frame* f, int x, int y, const char* s, Face face, int px, bool black);

// Measure the advance width of a string at pixel height `px`.
int ttf_width(const char* s, Face face, int px);

// The line height (ascent+descent) for a face at pixel height `px`.
int ttf_line_height(Face face, int px);

// Word-wrap `s` inside a box of width `box_w`, starting at top-left (x, y).
// `leading` is extra pixels between lines. Returns the y just past the last
// baseline row drawn.
int ttf_draw_wrapped(Frame* f, int x, int y, int box_w, const char* s,
                     Face face, int px, int leading, bool black);

}  // namespace oracle

#endif  // ORACLE_TEXT_H
