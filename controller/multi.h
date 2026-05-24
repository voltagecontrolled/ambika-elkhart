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

#ifndef CONTROLLER_MULTI_H_
#define CONTROLLER_MULTI_H_

#include "avrlib/base.h"

#include "controller/controller.h"
#include "controller/part.h"

namespace ambika {

struct MultiData {
  uint8_t clock_bpm;
  uint8_t clock_groove_template;  // unused since transport-page swng removal
  uint8_t clock_groove_amount;    // unused since transport-page swng removal
  uint8_t clock_latch;
  // 0 = off (free-run), 1..127 = reset every (value + 1) undivided steps,
  // i.e. encoded value k maps to a period of (k + 1) steps for k >= 1.
  uint8_t master_reset_steps;
  // MIDI per-track config — appended for snapshot v2 (see snapshot.cc).
  uint8_t midi_channel[kNumVoices];      // 1..16, default i+1
  uint8_t midi_only_mask;                // bit N = track N is EXT (MIDI-only)
  uint8_t midi_clock_mode;               // 0=INT, 1=EXT, 2=OUT, 3=THR
  // EXT-mode CC# per track. 8 slots/track: idx 0..3 on S5b, idx 4..7 on S5c.
  // Each slot reuses the existing per-step lockable storage for its value;
  // this array only holds the user-assigned CC numbers.
  uint8_t midi_cc_map[kNumVoices][8];
};

typedef MultiData PROGMEM prog_MultiData;

enum MultiParameter {
  PRM_MULTI_CLOCK_BPM = 0,
  PRM_MULTI_CLOCK_GROOVE_TEMPLATE = 1,
  PRM_MULTI_CLOCK_GROOVE_AMOUNT = 2,
  PRM_MULTI_CLOCK_LATCH = 3,
  PRM_MULTI_MASTER_RESET = 4,
  // Legacy addresses kept for parameter table compat — not wired to active pages.
  PRM_MULTI_MIDI_CHANNEL = 0,
  PRM_MULTI_KEYRANGE_LOW = 1,
  PRM_MULTI_KEYRANGE_HIGH = 2,
  PRM_MULTI_VOICE_ALLOCATION = 3,
};

static const uint8_t kNumTicksPerStep = 6;

class Multi {
 public:
  Multi() {}
  static void Init(bool force_reset);
  static void InitSettings(InitializationMode mode);

  static void NoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
  static void NoteOff(uint8_t channel, uint8_t note, uint8_t velocity);

  static void ControlChange(uint8_t channel, uint8_t controller, uint8_t value) {
    if (channel < kNumParts) { parts_[channel].ControlChange(controller, value); }
  }
  static void AllSoundOff(uint8_t channel) {
    if (channel < kNumParts) { parts_[channel].AllSoundOff(); }
  }
  static void ResetAllControllers(uint8_t channel) {
    if (channel < kNumParts) { parts_[channel].ResetAllControllers(); }
  }
  static void AllNotesOff(uint8_t channel) {
    if (channel < kNumParts) { parts_[channel].AllNotesOff(); }
  }
  static void OmniModeOff(uint8_t) {}
  static void OmniModeOn(uint8_t) {}
  static void MonoModeOn(uint8_t, uint8_t) {}
  static void PolyModeOn(uint8_t) {}

  static void Reset() {
    for (uint8_t i = 0; i < kNumParts; ++i) { parts_[i].Reset(); }
  }

  static void Clock();
  static void Start();
  static void Stop();
  static void Continue() { Start(); }

  static void Tick() {
    ++clock_counter_;
    if (clock_counter_ >= tick_duration_) {
      ++num_clock_events_;
      clock_counter_ = 0;
    }
  }

  static void SetValue(uint8_t address, uint8_t value);
  static inline uint8_t GetValue(uint8_t address) {
    const uint8_t* bytes = static_cast<const uint8_t*>(
        static_cast<const void*>(&data_));
    return bytes[address];
  }

  static void UpdateClocks();

  static Part* mutable_part(uint8_t i) { return &parts_[i]; }
  static const Part& part(uint8_t i) { return parts_[i]; }

  static MultiData* mutable_data() { return &data_; }
  static const MultiData& data() { return data_; }
  static const uint8_t* raw_data() {
    return static_cast<const uint8_t*>(static_cast<const void*>(&data_));
  }
  static uint8_t* mutable_raw_data() {
    return static_cast<uint8_t*>(static_cast<void*>(&data_));
  }

  // True when the internal timer drives the sequencer. Modes 0 (INT) and 2
  // (OUT) use the internal clock; modes 1 (EXT) and 3 (THR) sync from MIDI
  // clock-in. BPM gate keeps the original "off if BPM < 40" semantics.
  static uint8_t internal_clock() {
    return data_.clock_bpm >= 40 && !(data_.midi_clock_mode & 1);
  }

  static uint8_t part_channel(Part* part) {
    return static_cast<uint8_t>(part - parts_);
  }

  // MIDI per-track accessors (added for snapshot v2). track_channel returns
  // the user-assigned MIDI channel in the 1..16 range; subtract 1 for wire.
  static inline uint8_t track_channel(uint8_t t) {
    return data_.midi_channel[t];
  }
  static inline uint8_t track_is_ext(uint8_t t) {
    return (data_.midi_only_mask >> t) & 1;
  }
  // EXT-slot CC# accessor. slot is 0..7 (first 4 on S5b, last 4 on S5c).
  static inline uint8_t cc_for_slot(uint8_t t, uint8_t slot) {
    return data_.midi_cc_map[t][slot];
  }
  static inline uint8_t midi_clock_mode() {
    return data_.midi_clock_mode;
  }

  static uint8_t step() { return step_count_; }
  static uint8_t running() { return running_; }
  static void Touch();

  static inline uint8_t flags() {
    uint8_t result = flags_;
    for (uint8_t i = 0; i < kNumParts; ++i) {
      result |= parts_[i].flags();
    }
    return result;
  }

  inline void ClearFlag(uint8_t flag) {
    flags_ &= ~flag;
    for (uint8_t i = 0; i < kNumParts; ++i) {
      parts_[i].ClearFlag(flag);
    }
  }

 private:
  static void ComputeTickDuration();

  static uint16_t clock_counter_;
  static volatile uint8_t num_clock_events_;
  static uint16_t tick_duration_;
  static uint8_t tick_count_;
  static uint8_t step_count_;
  static uint8_t running_;

  static MultiData data_;
  static Part parts_[kNumParts];
  static uint8_t flags_;

  DISALLOW_COPY_AND_ASSIGN(Multi);
};

extern Multi multi;

}  // namespace ambika

#endif  // CONTROLLER_MULTI_H_
