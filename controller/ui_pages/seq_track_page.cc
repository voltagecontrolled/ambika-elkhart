// Copyright 2011 Emilie Gillet.
//
// Per-track sequencer settings page (S6). Knobs map directly to
// SeqTrack.pattern[] for the active track. Layout matches the YAM
// 4-cells-per-row convention; encoder walks an 8-element cursor and
// spills to neighboring pages at the boundary.

#include "controller/ui_pages/seq_track_page.h"

#include "avrlib/string.h"
#include "controller/display.h"
#include "controller/multi.h"
#include "controller/sequencer.h"
#include "controller/ui.h"
#include "controller/voicecard_tx.h"

namespace ambika {

/* static */
uint8_t SeqTrackPage::cursor_ = 0;
/* static */
uint8_t SeqTrackPage::rate_snap_pending_ = 0;

// 4-char short_name per knob (lowercase by default; uppercased on cursor).
// Cells 6 + 7 (formerly retired BPCH and now-redundant VOL — the mixer page
// covers per-track volume) carry MIDI channel + INT/EXT mode for the active
// track. They write to multi.data() instead of tr.pattern[].
static const prog_char kAbbr[] PROGMEM =
  "dirnraterotalengscalrootmch mmod";

// Rate display labels (matches sequencer.cc kRateValues): musical-notation values.
// 4 chars × 15 entries = 60 bytes PROGMEM. Shared with seq_steps_page.cc via
// extern declaration. Right-justified within each 4-char field.
//   32, 16t, 16, 8t, 16d, 8, 4t, 8d, 4, 2t, 4d, 2, 1, 1d, 2B
extern const prog_char kRateLabels[] PROGMEM =
  "  32 16t  16  8t 16d   8  4t  8d   4  2t  4d   2   1  1d  2B";

// Defined in sequencer.cc. Used for the click-toggle raw↔preset snap scan.
extern const prog_uint8_t kRateValues[] PROGMEM;

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
  OnClick,  // custom — toggles raw-tick mode on RATE cell
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
  rate_snap_pending_ = (cursor_ == 1) ? 1 : 0;
  return 1;
}

/* static */
uint8_t SeqTrackPage::OnClick() {
  if (cursor_ == 1) {
    uint8_t track = ui.state().active_part;
    SeqTrack* tr = sequencer.mutable_track(track);
    uint8_t v = tr->pattern[1];
    if (v & 0x80) {
      // raw → preset: snap to nearest entry in kRateValues.
      uint8_t period = v & 0x7F;
      uint8_t best_idx = 0;
      uint8_t best_diff = 0xFF;
      for (uint8_t i = 0; i < 15; ++i) {
        uint8_t p = pgm_read_byte(kRateValues + i);
        uint8_t diff = (p > period) ? (p - period) : (period - p);
        if (diff < best_diff) {
          best_diff = diff;
          best_idx = i;
        }
      }
      tr->pattern[1] = best_idx;
    } else {
      // preset → raw: resolve to ticks, store 0x80|period.
      uint8_t idx = v;
      if (idx >= 15) idx = 14;
      uint8_t period = pgm_read_byte(kRateValues + idx);
      if (period > 96) period = 96;
      tr->pattern[1] = 0x80 | period;
    }
    rate_snap_pending_ = 1;
    return 1;
  }
  return UiPage::OnClick();
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
    case 1: {  // rate (track): preset 0..14, or raw ticks if bit 7 set.
      uint8_t cur = tr->pattern[1];
      if (cur & 0x80) {
        uint8_t new_period = 2 + ((static_cast<uint16_t>(value) * 94) >> 7);
        if (new_period > 96) new_period = 96;
        if (rate_snap_pending_) {
          uint8_t cur_period = cur & 0x7F;
          uint8_t diff = (new_period > cur_period)
              ? (new_period - cur_period) : (cur_period - new_period);
          if (diff > 2) return 1;
          rate_snap_pending_ = 0;
        }
        tr->pattern[1] = 0x80 | new_period;
        return 1;
      }
      mapped = (static_cast<uint16_t>(value) * 15) >> 7;  // 0..127 → 0..14
      break;
    }
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
    case 6: {  // MCH: MIDI channel 1..16 for the active track. Writes to
               // multi.data().midi_channel[track], not tr->pattern[].
      uint8_t ch = 1 + ((static_cast<uint16_t>(value) * 16) >> 7);
      multi.mutable_data()->midi_channel[track] = ch;
      return 1;
    }
    case 7: {  // MMOD: 0=INT, 1=EXT. EXT skips the voicecard trigger and
               // emits configurable CCs at fire time. On INT→EXT release the
               // voice so any sounding note is silenced cleanly.
      uint8_t want_ext = (value >= 64) ? 1 : 0;
      uint8_t was_ext  = multi.track_is_ext(track);
      uint8_t mask = multi.mutable_data()->midi_only_mask;
      mask = want_ext ? (mask | (1 << track)) : (mask & ~(1 << track));
      multi.mutable_data()->midi_only_mask = mask;
      if (want_ext && !was_ext) {
        voicecard_tx.Release(track);
      }
      return 1;
    }
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

    // Value (4 chars) at offset 6, with separator space at 5.
    uint8_t v = tr.pattern[i];
    buffer[5] = ' ';
    char* val = &buffer[6];
    switch (i) {
      case 0:  // DIRN
        memcpy_P(val, kDirnLabels + (v & 3) * 4, 4);
        break;
      case 1: {  // rate
        if (v & 0x80) {
          val[0] = ' '; val[1] = 't';
          UnsafeItoa<uint8_t>(v & 0x7F, 2, val + 2);
          AlignRight(val + 2, 2);
        } else {
          uint8_t i = v;
          if (i >= 15) i = 14;
          memcpy_P(val, kRateLabels + i * 4, 4);
        }
        break;
      }
      case 4:  // SCAL
        memcpy_P(val, kScaleLabels + (v & 7) * 4, 4);
        break;
      case 5:  // ROOT
        memcpy_P(val, kRootLabels + (v % 12) * 4, 4);
        break;
      case 6:  // MCH — MIDI channel 1..16 from multi data, not tr.pattern.
        UnsafeItoa<uint8_t>(multi.data().midi_channel[track], 4, val);
        AlignRight(val, 4);
        break;
      case 7:  // MMOD — INT/EXT from midi_only_mask bit.
        if (multi.track_is_ext(track)) {
          val[0] = ' '; val[1] = 'E'; val[2] = 'X'; val[3] = 'T';
        } else {
          val[0] = ' '; val[1] = 'I'; val[2] = 'N'; val[3] = 'T';
        }
        break;
      default:
        UnsafeItoa<uint8_t>(v, 4, val);
        AlignRight(val, 4);
        break;
    }
  }
}

}  // namespace ambika
