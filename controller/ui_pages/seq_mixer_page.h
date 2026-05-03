// Performance mixer (S6b): per-voice volume on top1-3 / bot1-3 pots,
// SMUT / AMUT / SOLO toggles on S1-S6.
// S7 tap = cycle mode; S7 hold = shift-gate (S1-S6 toggles queued, applied
// on release as a single batch). S8 = unmute-all (clears all bit sets).
//
// State is transient (file-static, cleared on power-cycle). Volume writes
// directly into SeqTrack.pattern[kPatVOL]. Mute/solo bits are exposed to
// Sequencer via skip_mask() used in FireStep().

#ifndef CONTROLLER_UI_PAGES_SEQ_MIXER_PAGE_H_
#define CONTROLLER_UI_PAGES_SEQ_MIXER_PAGE_H_

#include "controller/ui_pages/ui_page.h"

namespace ambika {

class SeqMixerPage : public UiPage {
 public:
  SeqMixerPage() { }

  static void OnInit(PageInfo* info);
  static uint8_t OnIncrement(int8_t increment);
  static uint8_t OnPot(uint8_t index, uint8_t value);
  static uint8_t OnKey(uint8_t key);
  static void UpdateScreen();
  static void UpdateLeds();

  static const prog_EventHandlers event_handlers_;

  // Used by Sequencer::FireStep — bit i set means voice i is silenced.
  static uint8_t skip_mask();

 private:
  static uint8_t cursor_;  // 0..7, walks 4-cell × 2-row layout

  DISALLOW_COPY_AND_ASSIGN(SeqMixerPage);
};

}  // namespace ambika

#endif  // CONTROLLER_UI_PAGES_SEQ_MIXER_PAGE_H_
