// Copyright 2011 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// -----------------------------------------------------------------------------
//
// Base UI page class.

#include "controller/ui_pages/ui_page.h"

#include "avrlib/string.h"

#include "controller/display.h"
#include "controller/leds.h"
#include "controller/parameter.h"
#include "controller/sequencer.h"
#include "controller/storage.h"
#include "controller/system_settings.h"

namespace ambika {

/* static */
int8_t UiPage::active_control_;

/* static */
EditMode UiPage::edit_mode_;

/* static */
PageInfo* UiPage::info_;

/* static */
const prog_EventHandlers UiPage::event_handlers_ PROGMEM = {
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
void UiPage::OnInit(PageInfo* info) {
  info_ = info;
  edit_mode_ = EDIT_IDLE;
  active_control_ = 0;
}

/* static */
void UiPage::SetActiveControl(ActiveControl active_control) { }

/* static */
uint8_t UiPage::OnIncrement(int8_t increment) {
  return 1;
}

/* static */
uint8_t UiPage::OnClick() {
  if (edit_mode_ != EDIT_IDLE) {
    edit_mode_ = EDIT_IDLE;
  } else {
    edit_mode_ = EDIT_STARTED_BY_ENCODER;
  }
  return 1;
}

/* static */
uint8_t UiPage::OnPot(uint8_t index, uint8_t value) {
  return 1;
}

/* static */
uint8_t UiPage::OnKey(uint8_t key) {
  if (key >= SWITCH_SHIFT_1) {
    // Patch clipboard / history snapshot ops (YAM heritage) removed:
    // snapshot.cc on the System page is the live save/load path.
    return 1;
  }

  if (info_->index == PAGE_SYSTEM_SETTINGS) {
    system_settings.Save();
  }

  // v4.2: step buttons (SWITCH_1..8) toggle the active track's step
  // on/off globally — i.e. on every page that doesn't override OnKey
  // with its own button semantics (sequencer / system / dialog / mixer).
  // A pot-touch during the press flips the inhibit bit upstream so this
  // OnKey is never invoked for the lock-edit case.
  if (key <= SWITCH_8) {
    uint8_t track = ui.state().active_part;
    SeqStep& s = sequencer.mutable_track(track)->steps[key];
    s.step_flags ^= kStepFlagOn;
    return 1;
  }

  return 0;
}

/* static */
uint8_t UiPage::OnIdle() {
  if (edit_mode_ == EDIT_STARTED_BY_POT) {
    edit_mode_ = EDIT_IDLE;
  }
  return 1;
}

/* static */
void UiPage::UpdateScreen() { }

/* static */
void UiPage::OnDialogClosed(uint8_t dialog_id, uint8_t return_value) { }

/* static */
void UiPage::UpdateLeds() {
  // v4.2: step LEDs visible on every page that inherits this default
  // (Osc / Mix / Filter / Env / LFO / Track). Pages that own the S
  // buttons for other purposes (System, SeqMixer, SeqSteps, dialog,
  // info) override UpdateLeds and draw their own.
  uint8_t track = ui.state().active_part;
  const SeqTrack& tr = sequencer.track(track);
  for (uint8_t i = 0; i < kNumStepsPerTrack; ++i) {
    if (tr.steps[i].step_flags & kStepFlagOn) {
      leds.set_pixel(LED_1 + i, 0x0f);  // amber on
    }
  }
  if (sequencer.global().transport == kSeqPlaying) {
    leds.set_pixel(LED_1 + tr.shadow[kShdwLAST], 0xf0);  // green playhead
  }
}

}  // namespace ambika
