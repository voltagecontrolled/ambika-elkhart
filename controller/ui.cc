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
// User interface.

#include "controller/ui.h"

#include "avrlib/string.h"
#include "avrlib/time.h"

#include "controller/display.h"
#include "controller/leds.h"
#include "controller/multi.h"
#include "controller/resources.h"
#include "controller/sequencer.h"
#include "controller/system_settings.h"

#include "controller/ui_pages/card_info_page.h"
#include "controller/ui_pages/dialog_box.h"
#include "controller/ui_pages/multi_page.h"
#include "controller/ui_pages/os_info_page.h"
#include "controller/ui_pages/seq_mixer_page.h"
#include "controller/ui_pages/seq_steps_page.h"
#include "controller/ui_pages/seq_track_page.h"
#include "controller/ui_pages/parameter_editor.h"
#include "controller/ui_pages/system_page.h"
#include "controller/voicecard_tx.h"

namespace ambika {
  
const prog_PageInfo page_registry[] PROGMEM = {
  { PAGE_OSCILLATORS,
    &ParameterEditor::event_handlers_,
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    PAGE_MIXER, 0, 0xf0,
  },
  
  { PAGE_MIXER,
    &ParameterEditor::event_handlers_,
    { 8, 13, 12, 11, 9, 10, 14, 15 },
    PAGE_OSCILLATORS, 0, 0x0f,
  },
  
  { PAGE_FILTER,
    &ParameterEditor::event_handlers_,
    { 16, 17, 0xff, 18, 22, 0xff, 0xff, 0xff },
    PAGE_FILTER, 1, 0xf0,
  },

  { PAGE_ENV_LFO,
    &ParameterEditor::event_handlers_,
    { 24, 25, 26, 27, 28, 75, 76, 77 },
    PAGE_VOICE_LFO, 2, 0xf0,
  },

  { PAGE_VOICE_LFO,
    &ParameterEditor::event_handlers_,
    { 78, 79, 80, 81, 32, 33, 82, 83 },
    PAGE_ENV_LFO, 2, 0x0f,
  },

  { PAGE_MODULATIONS,
    &ParameterEditor::event_handlers_,
    { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
    PAGE_ENV_LFO, 2, 0xf0,
  },
  
  // S5 group 4: sequencer mode (3 lock pages cycled by encoder).
  { PAGE_PART_SEQUENCER,
    &SeqStepsPage::event_handlers_,
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    PAGE_PART_SEQUENCER, 4, 0xff,
  },

  // S6a group 5: per-track sequencer settings.
  { PAGE_PART,
    &SeqTrackPage::event_handlers_,
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    PAGE_SEQ_MIXER, 5, 0xf0,
  },

  // S6b group 5: performance mixer (volumes + mute/solo).
  { PAGE_SEQ_MIXER,
    &SeqMixerPage::event_handlers_,
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    PAGE_PART, 5, 0x0f,
  },

  // S7 group 6: transport (relocated from S5). Single-page group — clock
  // params (groove amount aka swng) are surfaced inline on PAGE_MULTI.
  { PAGE_MULTI,
    &MultiPage::event_handlers_,
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    PAGE_MULTI, 6, 0xf0,
  },

  // Vestigial entry kept so PAGE_MULTI_CLOCK (the upper bound used by
  // ShowPageRelative wraparound) stays in the registry. All-0xff data makes
  // the encoder skip past it.
  { PAGE_MULTI_CLOCK,
    &ParameterEditor::event_handlers_,
    { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
    PAGE_MULTI, 6, 0x0f,
  },

  // PAGE_PERFORMANCE and PAGE_KNOB_ASSIGN: placeholder until Perf page is built.
  { PAGE_PERFORMANCE,
    &ParameterEditor::event_handlers_,
    { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
    PAGE_PERFORMANCE, 6, 0xf0,
  },

  { PAGE_KNOB_ASSIGN,
    &ParameterEditor::event_handlers_,
    { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
    PAGE_PERFORMANCE, 6, 0x0f,
  },

  // PAGE_LIBRARY: repurposed as the System page (Save/Load/Info menu).
  { PAGE_LIBRARY,
    &SystemPage::event_handlers_,
    { 0, 0, 0, 0, 0, 0, 0, 0, },
    PAGE_LIBRARY, 7, 0xf0,
  },

  { PAGE_VERSION_MANAGER,
    &OsInfoPage::event_handlers_,
    { 0, 0, 0, 0, 0, 0, 0, 0, },
    PAGE_OS_INFO, 8, 0xf0,
  },

  { PAGE_SYSTEM_SETTINGS,
    &ParameterEditor::event_handlers_,
    { 66, 67, 71, 72, 68, 69, 0xff, 70, },
    PAGE_SYSTEM_SETTINGS, 8, 0xf0,
  },

  { PAGE_OS_INFO,
    &OsInfoPage::event_handlers_,
    { 0, 0, 0, 0, 0, 0, 0, 0, },
    PAGE_OS_INFO, 8, 0xf0,
  },
};

static const prog_uint8_t default_most_recent_page_in_group[9] PROGMEM = {
  PAGE_OSCILLATORS,        // S1
  PAGE_FILTER,             // S2
  PAGE_ENV_LFO,            // S3
  PAGE_ENV_LFO,            // S4 (shares group 2 with S3 — both env+LFO)
  PAGE_PART_SEQUENCER,     // S5: sequencer mode (lock pages)
  PAGE_PART,               // S6: per-track settings
  PAGE_MULTI,              // S7: transport
  PAGE_LIBRARY,            // S8: System page (Save/Load/Info menu)
  PAGE_SYSTEM_SETTINGS
};

/* <static> */
UiPageNumber Ui::active_page_;
UiPageNumber Ui::most_recent_non_system_page_;
uint8_t Ui::cycle_;
uint8_t Ui::inhibit_switch_;
uint16_t Ui::switch_press_ms_[8];
uint16_t Ui::switch_last_hold_ms_[8];
uint16_t Ui::transport_ccw_arm_ms_;
uint8_t Ui::transport_ccw_armed_;
Encoder Ui::encoder_;
Switches Ui::switches_;
Pots Ui::pots_;

UiState Ui::state_;
EventQueue<8> Ui::queue_;
uint8_t Ui::pot_value_[8];
UiPageNumber Ui::most_recent_page_in_group_[9];

PageInfo Ui::page_info_;
EventHandlers Ui::event_handlers_;
/* </static> */

/* extern */
Ui ui;

static char line[41];

/* static */
void Ui::Init() {
  memset(&state_, 0, sizeof(UiState));
  memcpy_P(most_recent_page_in_group_, default_most_recent_page_in_group, 8);

  encoder_.Init();
  switches_.Init();
  pots_.Init();
  lcd.Init();
  display.Init();
  leds.Init();
  lcd.SetCustomCharMapRes(character_table[0], 7, 1);
  
  ShowPage(PAGE_FILTER);
  
  memset(line, ' ', 41);
  line[40] = '\0';
  inhibit_switch_ = 0;
}

/* static */
void Ui::Poll() {
  ++cycle_;
  // I
  int8_t increment = encoder_.Read();
  uint8_t clicked = encoder_.clicked();

  // Hold-S5 + encoder turn = global transport chord.
  // CW = toggle Play/Pause; CCW = Stop, double-CCW within window = Panic
  // (Kill all voices). Encoder click is not consumed here so S5 + click
  // remains available to the page handler (e.g. SUBS editor on step 5).
  if (switches_.low(3) && increment != 0) {
    inhibit_switch_ |= (1 << 3);
    uint16_t now = static_cast<uint16_t>(avrlib::milliseconds());
    if (increment < 0) {
      if (transport_ccw_armed_ && (now - transport_ccw_arm_ms_) < 400) {
        sequencer.Panic();
        transport_ccw_armed_ = 0;
      } else {
        sequencer.Stop();
        transport_ccw_armed_ = 1;
        transport_ccw_arm_ms_ = now;
      }
    } else {
      if (sequencer.global().transport == kSeqStopped) {
        sequencer.Play();
      } else {
        sequencer.Pause();
      }
      transport_ccw_armed_ = 0;
    }
    increment = 0;
  }

  // Hold-S7 + encoder turn = cycle Transport ↔ Mixer page.
  // SR-index 1 = SWITCH_7 (mapping: control = 7 - sr_index).
  if (switches_.low(1) && increment != 0) {
    inhibit_switch_ |= (1 << 1);
    UiPageNumber target =
        (active_page_ == PAGE_SEQ_MIXER) ? PAGE_MULTI : PAGE_SEQ_MIXER;
    display.Clear();
    ShowPage(target);
    increment = 0;
  }

  if (increment != 0) {
    uint8_t control_id = 0;
    if (switches_.low(0)) {
      increment *= 8;
      inhibit_switch_ |= 0x01;
    }
    // SR-index 6 = SWITCH_2: hold to jump by full page (×8 like SWITCH_8).
    if (switches_.low(6)) {
      increment *= 8;
      inhibit_switch_ |= 0x40;
    }
    if (switches_.low(7)) {
      ++control_id;
      inhibit_switch_ |= 0x80;
    }
    queue_.AddEvent(CONTROL_ENCODER, control_id, increment);
  }
  if (clicked) {
    queue_.AddEvent(CONTROL_ENCODER_CLICK, 0, 1);
  }
  
  if (!(cycle_ & 31)) {
    switches_.Read();
    uint16_t now_ms = static_cast<uint16_t>(avrlib::milliseconds());
    uint8_t mask = 1;
    for (uint8_t i = 0; i < 8; ++i) {
      if (switches_.lowered(i)) {
        switch_press_ms_[i] = now_ms;
        switch_last_hold_ms_[i] = 0;
      }
      if (switches_.raised(i)) {
        switch_last_hold_ms_[i] = now_ms - switch_press_ms_[i];
        if ((inhibit_switch_ & mask)) {
          inhibit_switch_ ^= mask;
        } else {
          uint8_t control = SWITCH_8;
          if (switches_.low(0)) {
            control = SWITCH_SHIFT_8;
            inhibit_switch_ = 0x01;
          }
          control -= i;
          queue_.AddEvent(CONTROL_SWITCH, control, 1);
        }
      }
      mask <<= 1;
    }
  }
  
  if (!(cycle_ & 7)) {
    display.BlinkCursor();
  }
  
  pots_.Read();
  uint8_t index = pots_.last_read();
  uint16_t value = pots_.value(index);
  if (value != pot_value_[index]) {
    pot_value_[index] = value;
    // Software correction for the neat (but incorrect) hardware layout.
    if (index < 4) {
      index = 3 - index;
    }
    queue_.AddEvent(CONTROL_POT, index, value);
  }
  
  if (voicecard_tx.sd_card_busy()) {
    display.ForceStatus(0xdb);
    leds.set_direct_pixel(LED_STATUS, 0x0f);
  }

  // O
  leds.Write();
  lcd.Tick();
}

/* static */
void Ui::ShowPageRelative(int8_t increment) {
  // Disable page scrolling for the system pages.
  if (page_info_.index >= PAGE_LIBRARY) {
    return;
  }

  int8_t current_page = page_info_.index;
  PageInfo candidate;
  uint8_t guard = PAGE_LIBRARY;
  do {
    current_page += increment;
    if (current_page < 0) {
      current_page = PAGE_MULTI_CLOCK;
    } else if (current_page > PAGE_MULTI_CLOCK) {
      current_page = 0;
    }
    ResourcesManager::Load(page_registry, current_page, &candidate);
    --guard;
  } while (guard &&
      candidate.data[0] == 0xff && candidate.data[1] == 0xff &&
      candidate.data[2] == 0xff && candidate.data[3] == 0xff &&
      candidate.data[4] == 0xff && candidate.data[5] == 0xff &&
      candidate.data[6] == 0xff && candidate.data[7] == 0xff);

  ShowPage(static_cast<UiPageNumber>(current_page));
  if (increment >= 0) {
    (*event_handlers_.SetActiveControl)(ACTIVE_CONTROL_FIRST);
  } else {
    (*event_handlers_.SetActiveControl)(ACTIVE_CONTROL_LAST);
  }
}

const prog_uint8_t part_leds_remap[] PROGMEM = { 0, 3, 1, 4, 2, 5 };

/* static */
void Ui::DoEvents() {
  display.Tick();
  
  uint8_t redraw = 0;
  while (queue_.available()) {
    Event e = queue_.PullEvent();
    queue_.Touch();
    redraw = 1;
    if (e.control_type == 0x3 && e.control_id == 0x3f && e.value == 0xff) {
      // Dummy event, continue
      continue;
    }
    switch (e.control_type) {
      case CONTROL_ENCODER_CLICK:
        (*event_handlers_.OnClick)();
        break;
        
      case CONTROL_ENCODER:
        if (e.control_id == 0) {
          (*event_handlers_.OnIncrement)(e.value);
        } else {
          int8_t new_part = state_.active_part + e.value;
          new_part = Clip(new_part, 0, kNumParts - 1);
          state_.active_part = new_part;
        }
        break;
        
      case CONTROL_SWITCH:
        if (!(*event_handlers_.OnKey)(e.control_id)) {
          // Cycle through the next page in the group.
          if (page_info_.group == e.control_id) {
            ShowPage(page_info_.next_page);
          } else {
            // Jump to the most recently visited page in the group.
            ShowPage(most_recent_page_in_group_[e.control_id]);
          }
        }
        break;
        
      case CONTROL_POT:
        (*event_handlers_.OnPot)(e.control_id, e.value);
        break;
    }
  }
  
  if (queue_.idle_time_ms() > 800) {
    queue_.Touch();
    if ((*event_handlers_.OnIdle)()) {
      redraw = 1;
    }
  }
  
  if (multi.flags() & FLAG_HAS_CHANGE) {
    redraw = 1;
    multi.ClearFlag(FLAG_HAS_CHANGE);
  }
  
  if (redraw) {
    display.Clear();
    // The status icon is displayed when there is blank space at the left/right
    // of the page. We don't want the icon to display the icon to show up on the
    // right side of the page, so we fill the last character of the first line
    // with an invisible, non-space character.
    display.line_buffer(0)[39] = '\xfe';
    
    display.set_cursor_position(kLcdNoCursor);
    (*event_handlers_.UpdateScreen)();
  }
  
  leds.Clear();
  leds.set_pixel(
      LED_PART_1 + pgm_read_byte(part_leds_remap + state_.active_part), 0xf0);
  for (uint8_t i = 0; i < kNumVoices; ++i) {
    uint8_t led_index = pgm_read_byte(part_leds_remap + i);
    uint8_t velocity = voicecard_tx.voice_status(i) >> 3;
    leds.set_pixel(
        LED_PART_1 + led_index,
        velocity | leds.pixel(LED_PART_1 + led_index));
  }
  (*event_handlers_.UpdateLeds)();
  if (system_settings.data().swap_leds_colors) {
    for (uint8_t i = 0; i < 15; ++i) {
      leds.set_pixel(i, U8Swap4(leds.pixel(i)));
    }
  }
  leds.Sync();
}

/* static */
void Ui::ShowPage(UiPageNumber page, uint8_t initialize) {
  // Flush the event queue.
  queue_.Flush();
  queue_.Touch();
  pots_.Lock(16);
  
  if (page <= PAGE_KNOB_ASSIGN) {
    most_recent_non_system_page_ = page;
  }
  active_page_ = page;

  // Load the page info structure in RAM.
  ResourcesManager::Load(page_registry, page, &page_info_);

  // Load the event handlers structure in RAM.
  ResourcesManager::Load(page_info_.event_handlers, 0, &event_handlers_);
  most_recent_page_in_group_[page_info_.group] = page;
  if (initialize) {
    (*event_handlers_.OnInit)(&page_info_);
  }
  (*event_handlers_.UpdateScreen)();
}

/* static */
void Ui::ShowDialogBox(uint8_t dialog_id, Dialog dialog, uint8_t choice) {
  // Flush the event queue.
  queue_.Flush();
  queue_.Touch();
  
  // Replace the current page by the dialog box handler.
  ResourcesManager::Load(&DialogBox::event_handlers_, 0, &event_handlers_);
  page_info_.dialog = dialog;
  page_info_.index = dialog_id;
  (*event_handlers_.OnInit)(&page_info_);
  DialogBox::set_choice(choice);
  (*event_handlers_.UpdateScreen)();
}

/* static */
void Ui::CloseDialogBox(uint8_t return_value) {
  // Return to the page that was active when the dialog was shown.
  uint8_t returning_from = page_info_.index;
  ShowPage(active_page_, 0);
  (*event_handlers_.OnDialogClosed)(returning_from, return_value);
}

}  // namespace ambika
