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

#ifndef CONTROLLER_PART_H_
#define CONTROLLER_PART_H_

#include "avrlib/base.h"

#include "common/patch.h"
#include "controller/controller.h"

namespace ambika {

enum InitializationMode {
  INITIALIZATION_DEFAULT,
  INITIALIZATION_RANDOM
};

// Kept for parameter.cc / resources compat — not tied to live functionality.
enum ArpeggiatorDirection {
  ARPEGGIO_DIRECTION_UP = 0,
  ARPEGGIO_DIRECTION_DOWN,
  ARPEGGIO_DIRECTION_UP_DOWN,
  ARPEGGIO_DIRECTION_AS_PLAYED,
  ARPEGGIO_DIRECTION_RANDOM,
  ARPEGGIO_DIRECTION_CHORD,
  ARPEGGIO_DIRECTION_LAST
};

enum ArpSequencerMode {
  ARP_SEQUENCER_MODE_STEP,
  ARP_SEQUENCER_MODE_ARPEGGIATOR,
  ARP_SEQUENCER_MODE_NOTE,
  ARP_SEQUENCER_MODE_LAST
};

enum PolyphonyMode {
  MONO,
  POLY,
  UNISON_2X,
  CYCLIC,
  CHAIN,
  POLYPHONY_MODE_LAST
};

enum PartFlags {
  FLAG_HAS_CHANGE      = 1,
  FLAG_HAS_USER_CHANGE = 2,
};

// PartData struct retained for sizeof() references in storage.cc.
// No longer stored as a Part member — voice data lives in SeqTrack.
struct PartData {
  uint8_t volume;
  int8_t  octave;
  int8_t  tuning;
  uint8_t spread;
  uint8_t raga;
  uint8_t legato;
  uint8_t portamento_time;
  uint8_t arp_sequencer_mode;
  uint8_t arp_direction;
  uint8_t arp_octave;
  uint8_t arp_pattern;
  uint8_t arp_divider;
  uint8_t sequence_length[3];
  uint8_t polyphony_mode;
};

typedef PartData PROGMEM prog_PartData;

enum PartParameter {
  PRM_PART_VOLUME = sizeof(Patch),
  PRM_PART_OCTAVE,
  PRM_PART_TUNING,
  PRM_PART_TUNING_SPREAD,
  PRM_PART_RAGA,
  PRM_PART_LEGATO,
  PRM_PART_PORTAMENTO_TIME,
  PRM_PART_ARP_MODE,
  PRM_PART_ARP_DIRECTION,
  PRM_PART_ARP_OCTAVE,
  PRM_PART_ARP_PATTERN,
  PRM_PART_ARP_RESOLUTION,
  PRM_PART_SEQUENCE_LENGTH_1,
  PRM_PART_SEQUENCE_LENGTH_2,
  PRM_PART_SEQUENCE_LENGTH_3,
  PRM_PART_POLYPHONY_MODE
};

// Phase 3: Part is a minimal MIDI routing stub (2 bytes).
// Patch and PartData are no longer members — voice data lives in SeqTrack.
// GetValue/SetValue are stubs; parameter editor pages show 0 until Phase 7 rewires them.
class Part {
 public:
  Part() {}
  void Init(uint8_t voice_id) { voice_id_ = voice_id; flags_ = 0; }
  void InitSettings(InitializationMode) {}

  void NoteOn(uint8_t note, uint8_t velocity);
  void NoteOff(uint8_t note);
  void ControlChange(uint8_t controller, uint8_t value);
  void PitchBend(uint16_t pitch_bend);
  void Aftertouch(uint8_t velocity);
  void AllSoundOff();
  void AllNotesOff();
  void ResetAllControllers();
  void Reset();

  void SetValue(uint8_t address, uint8_t value, uint8_t user_initiated);
  uint8_t GetValue(uint8_t address) const;

  void TouchPatch() {}
  void Touch() {}

  // Stubs for storage.cc compatibility — return NULL; storage guards against null.
  const uint8_t* raw_patch_data() const { return NULL; }
  uint8_t* mutable_raw_patch_data() { return NULL; }
  const uint8_t* raw_data() const { return NULL; }
  uint8_t* mutable_raw_data() { return NULL; }
  const uint8_t* raw_sequence_data() const { return NULL; }
  uint8_t* mutable_raw_sequence_data() { return NULL; }

  uint8_t lfo_value(uint8_t) const { return 0; }

  uint8_t flags() const { return flags_; }
  void ClearFlag(uint8_t flag) { flags_ &= ~flag; }

 private:
  uint8_t voice_id_;
  uint8_t flags_;

  DISALLOW_COPY_AND_ASSIGN(Part);
};

}  // namespace ambika

#endif  // CONTROLLER_PART_H_
