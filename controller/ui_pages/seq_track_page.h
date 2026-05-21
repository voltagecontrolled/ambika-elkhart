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

  static const prog_EventHandlers event_handlers_;

 private:
  // Highlighted knob (0..7).
  static uint8_t cursor_;
  // Snap-on-cross gate for cursor==1 raw-tick mode. Set when entering raw
  // mode or when the cursor lands on RATE; cleared once the pot sweeps near
  // the stored tick value.
  static uint8_t rate_snap_pending_;

  DISALLOW_COPY_AND_ASSIGN(SeqTrackPage);
};

}  // namespace ambika

#endif  // CONTROLLER_UI_PAGES_SEQ_TRACK_PAGE_H_
