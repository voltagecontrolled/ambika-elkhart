// Copyright 2011 Emilie Gillet.
//
// Per-track sequencer settings page: DIRN/CDIV/ROTA/LENG (top),
// SCAL/ROOT/BPCH/OLEV (bottom). Operates on the active track's
// SeqTrack.pattern[].

#ifndef CONTROLLER_UI_PAGES_SEQ_TRACK_PAGE_H_
#define CONTROLLER_UI_PAGES_SEQ_TRACK_PAGE_H_

#include "controller/ui_pages/ui_page.h"

namespace ambika {

class SeqTrackPage : public UiPage {
 public:
  SeqTrackPage() { }

  static uint8_t OnIncrement(int8_t increment);
  static uint8_t OnClick();
  static uint8_t OnPot(uint8_t index, uint8_t value);
  static void UpdateScreen();
  static void UpdateLeds();

  static const prog_EventHandlers event_handlers_;

 private:
  // Highlighted knob (0..7).
  static uint8_t cursor_;
  // Snap-on-cross gate for cursor==1 raw-tick mode. Set when entering raw
  // mode or when the cursor lands on RATE; cleared once the pot sweeps near
  // the stored tick value.
  static uint8_t rate_snap_pending_;
  // Cell 6 hosts MCH+MMOD as a single cell after #32. Encoder click toggles
  // which subparam the pot edits / the cell displays. 0 = MCH, 1 = MMOD.
  static uint8_t mch_mmod_active_;
  // Cell 7 hosts the init action (replacing the freed cell). Pot selects
  // 0 = "voic" (reset track patch defaults) / 1 = "ploc" (clear lock pool
  // entries for this track). Encoder long-press executes.
  static uint8_t init_choice_;
  // Acknowledge an init action by flashing LED_6 red. Records the ms
  // timestamp at execution; UpdateLeds drives the flash until elapsed.
  static uint16_t init_feedback_ms_;

  DISALLOW_COPY_AND_ASSIGN(SeqTrackPage);
};

}  // namespace ambika

#endif  // CONTROLLER_UI_PAGES_SEQ_TRACK_PAGE_H_
