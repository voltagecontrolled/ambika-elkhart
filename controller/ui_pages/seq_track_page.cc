// Copyright 2011 Emilie Gillet.
//
// Per-track sequencer settings page (S6). Knobs map directly to
// SeqTrack.pattern[] for the active track. Layout matches the YAM
// 4-cells-per-row convention; encoder walks an 8-element cursor and
// spills to neighboring pages at the boundary.

#include "controller/ui_pages/seq_track_page.h"

#include "avrlib/string.h"
#include "controller/display.h"
#include "controller/sequencer.h"
#include "controller/ui.h"

namespace ambika {

/* static */
uint8_t SeqTrackPage::cursor_ = 0;

// 4-char short_name per knob (lowercase by default; uppercased on cursor).
// `----` slot at index 6 is the retired BPCH cell (round 5: track-transpose
// will live on the future Performance page; pot is inhibited here).
static const prog_char kAbbr[] PROGMEM =
  "dirnraterotalengscalroot----vol ";

// Rate display labels (matches sequencer.cc kRateValues): musical-notation values.
// 4 chars × 15 entries = 60 bytes PROGMEM. Shared with seq_steps_page.cc via
// extern declaration. Right-justified within each 4-char field.
//   32, 16t, 16, 8t, 16d, 8, 4t, 8d, 4, 2t, 4d, 2, 1, 1d, 2B
extern const prog_char kRateLabels[] PROGMEM =
  "  32 16t  16  8t 16d   8  4t  8d   4  2t  4d   2   1  1d  2B";

// DIRN labels right-justified into 4-char fields.
static const prog_char kDirnLabels[] PROGMEM = " fwd rev pendrnd ";

// 12 root note names, 3 chars each, padded with leading space to 4.
static const prog_char kRootLabels[] PROGMEM =
  " C   C#  D   D#  E   F   F#  G   G#  A   A#  B  ";

// 8 scale labels, 4 chars each — leading-space pattern so the value field
// has a visible separator from the abbr (renders "SCAL pMi" not "SCALpMi").
// "chro" → " chr"; full word still readable.
static const prog_char kScaleLabels[] PROGMEM =
  " chr maj min dor mix pMa pMi blu";

/* static */
const prog_EventHandlers SeqTrackPage::event_handlers_ PROGMEM = {
  OnInit,
  SetActiveControl,
  OnIncrement,
  OnClick,
  OnPot,
  OnKey,
  NULL,
  OnIdle,
  UpdateScreen,
  UpdateLeds,
  OnDialogClosed,
};

/* static */
uint8_t SeqTrackPage::OnIncrement(int8_t increment) {
  int8_t next = static_cast<int8_t>(cursor_) + increment;
  if (next < 0) {
    cursor_ = 0;
    ui.ShowPageRelative(-1);
    return 1;
  }
  if (next >= 8) {
    cursor_ = 7;
    ui.ShowPageRelative(1);
    return 1;
  }
  cursor_ = next;
  return 1;
}

/* static */
uint8_t SeqTrackPage::OnPot(uint8_t index, uint8_t value) {
  if (index >= 8) return 0;
  uint8_t track = ui.state().active_part;
  SeqTrack* tr = sequencer.mutable_track(track);
  cursor_ = index;
  // Pot value range is 0..127 (HysteresisPotScanner resolution=7).
  uint8_t mapped;
  switch (index) {
    case 0:  // DIRN: 0..3
      mapped = value >> 5;  // 0..127 → 0..3
      break;
    case 1:  // rate (track): index 0..14
      mapped = (static_cast<uint16_t>(value) * 15) >> 7;  // 0..127 → 0..14
      break;
    case 2:  // ROTA: 0..7
      mapped = value >> 4;
      break;
    case 3:  // LENG: 1..8
      mapped = (value >> 4) + 1;
      break;
    case 4:  // SCAL: 0..7 indexes kScaleMasks[]
      mapped = value >> 4;
      break;
    case 5:  // ROOT: 0..11
      mapped = (static_cast<uint16_t>(value) * 12) >> 7;
      break;
    case 6:  // BPCH retired — pot is inhibited; ignore writes.
      return 0;
    case 7:  // VOL: 0..255 — scales velocity in FireStep
      mapped = value << 1;
      break;
    default:
      return 0;
  }
  tr->pattern[index] = mapped;
  return 1;
}

/* static */
void SeqTrackPage::UpdateScreen() {
  uint8_t track = ui.state().active_part;
  const SeqTrack& tr = sequencer.track(track);

  for (uint8_t i = 0; i < 8; ++i) {
    uint8_t line = i < 4 ? 0 : 1;
    uint8_t row = (i & 3) * 10;
    char* buffer = display.line_buffer(line) + row;

    if (row != 0)                buffer[0]  = kDelimiter;
    if ((row + 10) != kLcdWidth) buffer[10] = kDelimiter;

    // Short name (4 chars) at offset 1, uppercased on the active control.
    for (uint8_t c = 0; c < 4; ++c) {
      char ch = pgm_read_byte(kAbbr + i * 4 + c);
      if (i == cursor_ && ch >= 'a' && ch <= 'z') {
        ch -= 0x20;
      }
      buffer[1 + c] = ch;
    }

    // Value (4 chars) at offset 5.
    uint8_t v = tr.pattern[i];
    char* val = &buffer[5];
    switch (i) {
      case 0:  // DIRN
        memcpy_P(val, kDirnLabels + (v & 3) * 4, 4);
        break;
      case 1: {  // rate
        uint8_t i = v;
        if (i >= 15) i = 14;
        memcpy_P(val, kRateLabels + i * 4, 4);
        break;
      }
      case 4:  // SCAL
        memcpy_P(val, kScaleLabels + (v & 7) * 4, 4);
        break;
      case 5:  // ROOT
        memcpy_P(val, kRootLabels + (v % 12) * 4, 4);
        break;
      case 6:  // BPCH retired — show ----.
        val[0] = ' '; val[1] = '-'; val[2] = '-'; val[3] = '-';
        break;
      default:
        val[0] = ' ';
        UnsafeItoa<uint8_t>(v, 3, &val[1]);
        AlignRight(&val[1], 3);
        break;
    }
    buffer[9] = ' ';
  }
}

}  // namespace ambika
