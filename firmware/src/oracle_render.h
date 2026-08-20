// oracle_render.h -- 1-bit framebuffer + layout renderer for a 1360x480 panel.
//
// The framebuffer is MSB-first packed 1bpp (bit set = black), matching common
// e-ink conventions (GxEPD2 / Waveshare buffers). Width is padded to a byte
// boundary per row. No heap: the buffer is caller-provided.
#ifndef ORACLE_RENDER_H
#define ORACLE_RENDER_H

#include <cstddef>
#include <cstdint>

#include "oracle_astro.h"

namespace oracle {

// Panel geometry. "5 wide by 1 high" => a 5-column x 1-row layout grid.
static const int kCanvasW = 1360;
static const int kCanvasH = 480;
static const int kCols = 5;
static const int kRows = 1;

// Bytes per row (row-padded to whole bytes) and total buffer size.
static const int kRowBytes = (kCanvasW + 7) / 8;         // 170
static const size_t kFrameBytes = (size_t)kRowBytes * kCanvasH;  // 81600

// A view over a caller-owned 1bpp buffer.
struct Frame {
  uint8_t* buf;     // kFrameBytes bytes
  int w, h, stride; // stride == kRowBytes
};

void frame_init(Frame* f, uint8_t* buf);  // clears to white
void frame_clear(Frame* f, bool black);
void frame_set(Frame* f, int x, int y, bool black);
void frame_hline(Frame* f, int x0, int x1, int y, bool black);
void frame_vline(Frame* f, int x, int y0, int y1, bool black);
void frame_rect(Frame* f, int x, int y, int w, int h, bool black);       // outline
void frame_fill_rect(Frame* f, int x, int y, int w, int h, bool black);  // filled

// Draw one char scaled by integer `scale` at top-left (x,y). Returns advance px.
int draw_char(Frame* f, int x, int y, char c, int scale, bool black);
// Draw a string; no wrapping. Returns end x.
int draw_text(Frame* f, int x, int y, const char* s, int scale, bool black);
// Word-wrap `s` into a box; returns the y just past the last drawn line.
int draw_wrapped(Frame* f, int x, int y, int box_w, const char* s, int scale,
                 int line_gap, bool black);

// Text measurement.
int text_width(const char* s, int scale);

// ---- The Oracle daily layout -----------------------------------------
// Renders the full reading (moon / planets / signs / message / crystal) into
// the framebuffer using the 5-column grid. `name`, `place`, `crystal` may be
// nullptr. `crystal_note` is the one-line crystal intention (may be nullptr).
void render_oracle(Frame* f, const SkyState& st, const char* message,
                   const char* name, const char* place, const char* crystal,
                   const char* crystal_note);

// ---- BMP export (desktop preview) ------------------------------------
// Writes a 1-bit .bmp of the framebuffer. Returns true on success. On the
// ESP32 you would omit this and blit `buf` to the panel instead.
bool write_bmp(const char* path, const Frame* f);

}  // namespace oracle

#endif  // ORACLE_RENDER_H
