// Copyright 2011 Emilie Gillet.
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

#include "controller/midi_dispatcher.h"
#include "controller/multi.h"
#include "controller/sequencer.h"
#include "controller/storage.h"

namespace ambika {

/* extern */
Multi multi;

/* <static> */
MultiData Multi::data_;
Part Multi::parts_[kNumParts];
uint16_t Multi::clock_counter_ = 0;
volatile uint8_t Multi::num_clock_events_;
uint16_t Multi::tick_duration_;
uint8_t Multi::tick_count_;
uint8_t Multi::step_count_;
uint8_t Multi::running_;
uint8_t Multi::flags_;
/* </static> */

static const prog_MultiData init_settings PROGMEM = {
  120,  // clock_bpm
  0,    // clock_groove_template
  0,    // clock_groove_amount
  4,    // clock_latch
};

/* static */
void Multi::Init(bool force_reset) {
  for (uint8_t i = 0; i < kNumParts; ++i) {
    parts_[i].Init(i);
  }
  sequencer.Init();
  if (force_reset) {
    InitSettings(INITIALIZATION_DEFAULT);
    Storage::WriteMultiToEeprom();
  } else {
    if (!Storage::LoadMultiFromEeprom()) {
      InitSettings(INITIALIZATION_DEFAULT);
      Storage::WriteMultiToEeprom();
    } else {
      Touch();
    }
  }
  running_ = 0;
}

/* static */
void Multi::InitSettings(InitializationMode mode) {
  ResourcesManager::Load(&init_settings, 0, &data_);
  for (uint8_t i = 0; i < kNumParts; ++i) {
    parts_[i].InitSettings(mode);
  }
  Touch();
}

/* static */
void Multi::Touch() {
  ComputeTickDuration();
  flags_ = FLAG_HAS_CHANGE;
}

const int32_t kTempoFactor = 392156L;

/* static */
void Multi::ComputeTickDuration() {
  STATIC_ASSERT(kTempoFactor == 392156L);
  int32_t rounding = 2 * static_cast<int32_t>(data_.clock_bpm);
  int32_t denominator = 4 * static_cast<int32_t>(data_.clock_bpm);
  tick_duration_ = static_cast<uint16_t>((kTempoFactor + rounding) / denominator);
}

/* static */
void Multi::NoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (!running_) {
    Start();
  }
  if (channel < kNumParts) {
    parts_[channel].NoteOn(note, velocity);
  } else if (channel == 9) {
    // MIDI ch10 drum map: notes 36-41 trigger voices 0-5 at middle C.
    if (note >= 36 && note < 36 + kNumParts) {
      parts_[note - 36].NoteOn(60, velocity);
    }
  }
}

/* static */
void Multi::NoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (channel < kNumParts) {
    parts_[channel].NoteOff(note);
  } else if (channel == 9) {
    if (note >= 36 && note < 36 + kNumParts) {
      parts_[note - 36].NoteOff(note);
    }
  }
}

/* static */
void Multi::Clock() {
  ++tick_count_;
  if (tick_count_ == kNumTicksPerStep) {
    tick_count_ = 0;
    ++step_count_;
    if (step_count_ >= 8) {
      step_count_ = 0;
    }
  }
  sequencer.Clock(1);
  if (running_) {
    midi_dispatcher.OnClock();
  }
}

/* static */
void Multi::Start() {
  midi_dispatcher.OnStart();
  tick_count_ = 0;
  step_count_ = 0;
  tick_duration_ = 0;  // ensure ComputeTickDuration result is fresh
  ComputeTickDuration();
  running_ = 1;
}

/* static */
void Multi::Stop() {
  midi_dispatcher.OnStop();
  for (uint8_t i = 0; i < kNumParts; ++i) {
    parts_[i].AllNotesOff();
  }
  running_ = 0;
}

/* static */
void Multi::UpdateClocks() {
  if (internal_clock()) {
    while (num_clock_events_) {
      Clock();
      --num_clock_events_;
    }
  } else {
    num_clock_events_ = 0;
  }
}

/* static */
void Multi::SetValue(uint8_t address, uint8_t value) {
  uint8_t* bytes = static_cast<uint8_t*>(static_cast<void*>(&data_));
  if (bytes[address] != value) {
    bytes[address] = value;
    flags_ |= FLAG_HAS_USER_CHANGE;
    if (address <= PRM_MULTI_CLOCK_GROOVE_AMOUNT) {
      ComputeTickDuration();
    }
  }
}

}  // namespace ambika
