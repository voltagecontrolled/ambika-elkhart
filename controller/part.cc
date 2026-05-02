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

#include "controller/part.h"

#include "avrlib/op.h"
#include "avrlib/random.h"
#include "controller/midi_dispatcher.h"
#include "controller/resources.h"
#include "controller/voicecard_tx.h"

using namespace avrlib;

namespace ambika {

static const prog_Patch init_patch PROGMEM = {
  // Oscillators
  WAVEFORM_POLYBLEP_SAW, 0, 0, 0,
  WAVEFORM_POLYBLEP_PWM, 32, -12, 12,
  // Mixer
  32, OP_SUM, 31, WAVEFORM_SUB_OSC_SQUARE_1, 0, 0, 0, 0,
  // Filter
  96, 0, 0, 0, 0, 0, 24, 0,
  // ADSR
  0, 40, 20,  60, LFO_WAVEFORM_TRIANGLE, kNumSyncedLfoRates + 24, 0, 0,
  0, 40, 0,   40, LFO_WAVEFORM_TRIANGLE, kNumSyncedLfoRates + 32, 0, 0,
  0, 40, 100, 40, LFO_WAVEFORM_TRIANGLE, kNumSyncedLfoRates + 48, 0, 0,
  LFO_WAVEFORM_TRIANGLE, 72,
  // Routing
  MOD_SRC_ENV_1, MOD_DST_PARAMETER_1, 0,
  MOD_SRC_ENV_1, MOD_DST_PARAMETER_2, 0,
  MOD_SRC_LFO_1, MOD_DST_OSC_1, 0,
  MOD_SRC_LFO_1, MOD_DST_OSC_2, 0,
  MOD_SRC_LFO_2, MOD_DST_PARAMETER_1, 0,
  MOD_SRC_LFO_2, MOD_DST_PARAMETER_2, 0,
  MOD_SRC_LFO_3, MOD_DST_MIX_BALANCE, 0,
  MOD_SRC_LFO_4, MOD_DST_FILTER_CUTOFF, 0,
  MOD_SRC_SEQ_1, MOD_DST_FILTER_CUTOFF, 0,
  MOD_SRC_SEQ_2, MOD_DST_MIX_BALANCE, 0,
  MOD_SRC_ENV_3, MOD_DST_VCA, 63,
  MOD_SRC_VELOCITY, MOD_DST_VCA, 16,
  MOD_SRC_PITCH_BEND, MOD_DST_OSC_1_2_COARSE, 32,
  MOD_SRC_LFO_4, MOD_DST_OSC_1_2_COARSE, 16,
  // Modifiers
  MOD_SRC_LFO_1, MOD_SRC_LFO_2, 0,
  MOD_SRC_LFO_2, MOD_SRC_LFO_3, 0,
  MOD_SRC_LFO_3, MOD_SRC_SEQ_1, 0,
  MOD_SRC_SEQ_1, MOD_SRC_SEQ_2, 0,
  // Padding
  0, 0, 0, 0, 0, 0, 0, 0,
};

static const prog_PartData init_part PROGMEM = {
  120,         // volume
  0, 0, 0, 0,  // octave, tuning, spread, raga
  0, 0, 0,     // legato, portamento, arp_mode
  0, 1, 0, 10, // arp_direction, arp_octave, arp_pattern, arp_divider
  16, 16, 16,  // sequence_length[3]
  POLY,        // polyphony_mode
};

void Part::Init(uint8_t voice_id) {
  voice_id_ = voice_id;
  flags_ = 0;
}

void Part::InitPatch(InitializationMode mode) {
  if (mode == INITIALIZATION_DEFAULT) {
    ResourcesManager::Load(&init_patch, 0, &patch_);
  } else {
    uint8_t* bytes = static_cast<uint8_t*>(static_cast<void*>(&patch_));
    for (uint8_t i = 0; i < sizeof(Patch); ++i) {
      bytes[i] = Random::GetByte() & 0x7f;
    }
  }
  TouchPatch();
}

void Part::InitSettings(InitializationMode mode) {
  InitPatch(mode);
  if (mode == INITIALIZATION_DEFAULT) {
    ResourcesManager::Load(&init_part, 0, &data_);
  }
  Touch();
}

void Part::TouchPatch() {
  flags_ |= FLAG_HAS_CHANGE;
  voicecard_tx.PrepareForBlockWrite(voice_id_);
  ConstantDelay(5);
  const uint8_t* bytes = static_cast<const uint8_t*>(
      static_cast<const void*>(&patch_));
  voicecard_tx.WriteBlock(voice_id_, bytes, sizeof(Patch));
}

void Part::Touch() {
  flags_ |= FLAG_HAS_CHANGE;
  const uint8_t* bytes = static_cast<const uint8_t*>(
      static_cast<const void*>(&patch_));
  for (uint8_t address = PRM_PART_VOLUME;
       address <= PRM_PART_PORTAMENTO_TIME;
       ++address) {
    voicecard_tx.WriteData(
        voice_id_,
        VOICECARD_DATA_PART,
        address - sizeof(Patch),
        bytes[address]);
  }
}

void Part::SetValue(
    uint8_t address,
    uint8_t value,
    uint8_t user_initiated) {
  uint8_t* bytes = static_cast<uint8_t*>(static_cast<void*>(&patch_));
  bytes[address] = value;

  flags_ |= FLAG_HAS_CHANGE;
  if (user_initiated) {
    midi_dispatcher.OnEdit(this, address, value);
    flags_ |= FLAG_HAS_USER_CHANGE;
  }

  if (address < PRM_PART_VOLUME) {
    voicecard_tx.WriteData(voice_id_, VOICECARD_DATA_PATCH, address, value);
  } else if (address <= PRM_PART_PORTAMENTO_TIME) {
    voicecard_tx.WriteData(
        voice_id_, VOICECARD_DATA_PART, address - sizeof(Patch), value);
  }
}

void Part::NoteOn(uint8_t note, uint8_t velocity) {
  if (velocity == 0) { NoteOff(note); return; }
  int16_t n = static_cast<int16_t>(note) +
              static_cast<int16_t>(static_cast<int8_t>(data_.octave)) * 12;
  if (n < 0) n = 0;
  if (n > 127) n = 127;
  uint16_t tuned = static_cast<uint16_t>(n) << 7;
  tuned += static_cast<int8_t>(data_.tuning);
  midi_dispatcher.OnNote(this, note, velocity);
  voicecard_tx.Trigger(voice_id_, tuned, velocity, data_.legato);
}

void Part::NoteOff(uint8_t) {
  voicecard_tx.Release(voice_id_);
}

void Part::ControlChange(uint8_t controller, uint8_t value) {
  switch (controller) {
    case midi::kModulationWheelMsb:
      voicecard_tx.WriteData(
          voice_id_, VOICECARD_DATA_MODULATION, MOD_SRC_WHEEL, value << 1);
      break;
    case 0x78:  // All Sound Off
      AllSoundOff();
      break;
    case 0x7b:  // All Notes Off
      AllNotesOff();
      break;
  }
}

void Part::PitchBend(uint16_t pitch_bend) {
  voicecard_tx.WriteData(
      voice_id_, VOICECARD_DATA_MODULATION,
      MOD_SRC_PITCH_BEND, U14ShiftRight6(pitch_bend));
}

void Part::Aftertouch(uint8_t velocity) {
  voicecard_tx.WriteData(
      voice_id_, VOICECARD_DATA_MODULATION, MOD_SRC_AFTERTOUCH, velocity);
}

void Part::AllSoundOff() {
  voicecard_tx.Kill(voice_id_);
}

void Part::AllNotesOff() {
  voicecard_tx.Release(voice_id_);
}

void Part::ResetAllControllers() {
  voicecard_tx.ResetAllControllers(voice_id_);
}

void Part::Reset() {
  voicecard_tx.Reset(voice_id_);
}

}  // namespace ambika
