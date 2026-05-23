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
// A page which allows up to 8 parameters to be tweaked.

#include "controller/ui_pages/parameter_editor.h"

#include "avrlib/string.h"
#include "avrlib/time.h"

#include "controller/display.h"
#include "controller/leds.h"
#include "controller/multi.h"
#include "controller/parameter.h"
#include "controller/sequencer.h"
#include "controller/system_settings.h"
#include "controller/ui.h"

namespace ambika {

/* static */
const prog_EventHandlers ParameterEditor::event_handlers_ PROGMEM = {
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
ParameterEditor::SnapMask ParameterEditor::snapped_;
/* static */
uint16_t ParameterEditor::feedback_start_ms_;
/* static */
uint8_t ParameterEditor::feedback_cell_;
/* static */
uint8_t ParameterEditor::feedback_state_;

static const uint16_t kFeedbackDurationMs = 500;

/* static */
uint8_t ParameterEditor::parameter_index(uint8_t control_id) {
  return info_->data[control_id];
}

/* static */
uint8_t ParameterEditor::part_index(uint8_t control_id) {
  return ui.state().active_part;
}

/* static */
uint8_t ParameterEditor::instance_index(uint8_t control_id) {
  uint8_t parameter_id = info_->data[control_id];
  if (parameter_id == 0xff) {
    return 0xff;
  } else {
    const Parameter& parameter = parameter_manager.parameter(parameter_id);
    if (parameter.indexed_by != 0xff) {
      return static_cast<uint8_t*>(
          static_cast<void*>(ui.mutable_state()))[parameter.indexed_by];
    } else {
      return 0;
    }
  }
}

/* static */
void ParameterEditor::OnInit(PageInfo* info) {
  UiPage::OnInit(info);
  SetActiveControl(ACTIVE_CONTROL_FIRST);
  snapped_ = 0;
}

/* static */
void ParameterEditor::SetActiveControl(ActiveControl active_control) {
  if (active_control == ACTIVE_CONTROL_FIRST) {
    active_control_ = -1;
    OnIncrement(1);
  } else {
    active_control_ = 8;
    OnIncrement(-1);
  }
}

// Trigger-time reset toggle dispatch. Encoder click on the OSC1 SHAPE
// cell flips osc_phase_reset (patch byte 106); click on the VOICE LFO
// SHAPE cell flips lfo_retrigger (patch byte 107). Other cells: no-op
// (the legacy click-to-toggle-edit-mode behaviour is dropped — pots are
// the canonical value editor).
/* static */
uint8_t ParameterEditor::OnClick() {
  if (active_control_ < 0 || active_control_ >= kNumParametersPerPage) {
    return 1;
  }
  uint8_t parameter_id = info_->data[active_control_];
  uint8_t patch_addr;
  if (parameter_id == 0) {                // OSC1 SHAPE cell
    patch_addr = 106;                     // osc_phase_reset
  } else if (parameter_id == 33) {        // VOICE LFO 4 SHAPE cell
    patch_addr = 107;                     // lfo4_retrigger
  } else if (parameter_id == 74) {        // VOICE LFO 5 SHAPE cell (array idx)
    patch_addr = 110;                     // lfo5_retrigger
  } else {
    return 1;                             // no click action on this cell
  }
  uint8_t part = ui.state().active_part;
  uint8_t current = multi.part(part).GetValue(patch_addr);
  uint8_t next = current ? 0 : 1;
  multi.mutable_part(part)->SetValue(patch_addr, next, 1);
  uint16_t now = static_cast<uint16_t>(avrlib::milliseconds());
  if (now == 0) now = 1;                  // 0 reserved as "no feedback"
  feedback_start_ms_ = now;
  feedback_cell_ = active_control_;
  feedback_state_ = next;
  return 1;
}

/* static */
uint8_t ParameterEditor::OnIdle() {
  // Force a redraw while feedback is active so the overlay actually
  // clears once the duration elapses (UpdateScreen only runs on redraw).
  if (feedback_start_ms_) {
    uint16_t now = static_cast<uint16_t>(avrlib::milliseconds());
    if (static_cast<uint16_t>(now - feedback_start_ms_) >= kFeedbackDurationMs) {
      feedback_start_ms_ = 0;
      return 1;
    }
  }
  return UiPage::OnIdle();
}

/* static */
uint8_t ParameterEditor::OnIncrement(int8_t increment) {
  if (edit_mode_ != EDIT_IDLE) {
    parameter_manager.Increment(
        parameter_index(active_control_),
        part_index(active_control_),
        instance_index(active_control_),
        increment);
    edit_mode_ = EDIT_STARTED_BY_ENCODER;
  } else {
    int8_t new_control = active_control_ + increment;
    while (
        new_control >= 0 &&
        new_control < kNumParametersPerPage &&
        info_->data[new_control] == 0xff) {
      new_control += increment;
    }
    if (new_control < 0) {
      ui.ShowPageRelative(-1);
    } else if (new_control >= 8) {
      ui.ShowPageRelative(1);
    } else {
      active_control_ = new_control;
    }
  }
  return 1;
}

/* static */
uint8_t ParameterEditor::OnPot(uint8_t index, uint8_t value) {
  uint8_t parameter_id = parameter_index(index);
  if (parameter_id == 0xff) {
    return 1;
  }
  const Parameter& parameter = parameter_manager.parameter(parameter_id);
  if (system_settings.data().snap) {
    SnapMask mask = (1 << index);
    // If this pot has not reached the right position yet, test if the position
    // of the pot matches the value of the parameter.
    // Pots used to scroll among UI pages are not subject to snap.
    if (!(snapped_ & mask)) {
      if (parameter_manager.is_snapped(parameter,
              part_index(index),
              instance_index(index),
              value) || parameter.level == PARAMETER_LEVEL_UI) {
        snapped_ |= mask;
      } else {
        // Not there yet!
        return 1;
      }
    }
  }
  active_control_ = index;

  // Lock-edit: if a step button is held AND this parameter has a lock_index,
  // write the scaled byte to the sequencer lock pool instead of the patch.
  // Pool-full refuses silently (the gauge glyph gives advance warning).
  // Either way, a pot turn while a step is held inhibits the release-toggle
  // — the user intent is lock-edit, not a step on/off tap.
  uint8_t held_step = 0xff;
  for (uint8_t s = 0; s < 8; ++s) {
    if (ui.switch_held(s)) { held_step = 7 - s; break; }
  }
  if (held_step != 0xff) {
    ui.inhibit_switch(1 << (7 - held_step));
    if (parameter.level == PARAMETER_LEVEL_PATCH) {
      uint8_t lock_index = ParamIdToLockIndex(
          parameter_id, instance_index(index));
      if (lock_index != 0xff) {
        uint8_t scaled = parameter.Scale(value);
        uint8_t track  = part_index(active_control_);
        sequencer.SetStepLock(track, held_step, lock_index, scaled);
        return 1;
      }
    }
    // Step held but param not lockable on this cell — swallow the event
    // so we don't accidentally rewrite the patch default mid-lock-edit.
    return 1;
  }

  parameter_manager.Scale(
      parameter,
      part_index(active_control_),
      instance_index(active_control_),
      value);
  // Don't grab encoder focus on pot touch — the encoder should stay in
  // navigation mode. Pot is its own input.
  return 1;
}

/* static */
void ParameterEditor::UpdateScreen() {
  // If a step button is held, render the per-step locked value (lock pool or
  // intrinsic) so the user can see what the step will play. No step held →
  // render the live patch value.
  uint8_t held_step = 0xff;
  for (uint8_t s = 0; s < 8; ++s) {
    if (ui.switch_held(s)) { held_step = 7 - s; break; }
  }
  uint8_t paramLength = 0;
  for (uint8_t i = 0; i < kNumParametersPerPage; ++i) {
    uint8_t parameter_id = parameter_index(i);
    uint8_t line = i < 4 ? 0 : 1;
    uint8_t row = (i & 3) * 10;
    char* buffer = display.line_buffer(line) + row;
    if (row != 0) {
      buffer[0] = kDelimiter;
    }
    if ((row + 10) != kLcdWidth) {
      buffer[10] = kDelimiter;
    }
    if (parameter_id != 0xff) {
      const Parameter& parameter = parameter_manager.parameter(parameter_id);
      uint8_t value;
      uint8_t lock_index = (held_step != 0xff
                            && parameter.level == PARAMETER_LEVEL_PATCH)
          ? ParamIdToLockIndex(parameter_id, instance_index(i))
          : 0xff;
      if (lock_index != 0xff) {
        value = sequencer.StepLockedValue(part_index(i), held_step, lock_index);
      } else {
        value = parameter_manager.GetValue(
            parameter, part_index(i), instance_index(i));
      }
      if (parameter.level == PARAMETER_LEVEL_UI) {
        parameter.Print(value, &buffer[1], 6, 2);
        paramLength = 6;
      } else {
        parameter.Print(value, &buffer[1], 4, 4);
        paramLength = 4;
      }
      if (i == active_control_) {
        for (uint8_t c = 1; c < paramLength+1 ; ++c) {
          if (buffer[c] >= 'a' && buffer[c] <= 'z') {
            buffer[c] -= 0x20;
          }
        }
      }
    }
  }

  for (uint8_t i = 0; i < kNumParametersPerPage; ++i) {
    uint8_t parameter_id = info_->data[i];
    uint8_t line = i < 4 ? 0 : 1;
    uint8_t row = (i & 3) * 10;
    char* buffer = display.line_buffer(line) + row;
    if (parameter_id == 0xff) {
      buffer[0] = ' ';
      buffer[10] = ' ';
    }
  }

  // Trigger-time reset toggle feedback overlay. While within
  // kFeedbackDurationMs of the encoder click, replaces the cell contents
  // (between the kDelimiter separators) with "rst on " or "rst off".
  if (feedback_start_ms_ && feedback_cell_ < kNumParametersPerPage) {
    uint16_t now = static_cast<uint16_t>(avrlib::milliseconds());
    if (static_cast<uint16_t>(now - feedback_start_ms_) < kFeedbackDurationMs) {
      uint8_t line = feedback_cell_ < 4 ? 0 : 1;
      uint8_t row = (feedback_cell_ & 3) * 10;
      char* buffer = display.line_buffer(line) + row + 1;
      const char* label = feedback_state_ ? "rst on " : "rst off";
      for (uint8_t c = 0; c < 9; ++c) buffer[c] = ' ';
      for (uint8_t c = 0; label[c]; ++c) buffer[c] = label[c];
    } else {
      feedback_start_ms_ = 0;
    }
  }
}

/* static */
void ParameterEditor::UpdateLeds() {
  UiPage::UpdateLeds();
  if (multi.running() && (multi.step() & 3) == 0) {
    leds.set_pixel(LED_STATUS, 0xf0);
  }
}

}  // namespace ambika
