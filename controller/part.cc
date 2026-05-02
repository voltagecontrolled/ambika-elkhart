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

// Phase 3: Part stripped to MIDI routing only — no Patch/PartData.
// Voice parameters live in SeqTrack; MIDI input triggers voicecard directly.

#include "controller/part.h"

#include "avrlib/op.h"
#include "controller/sequencer.h"
#include "controller/voicecard_tx.h"
#include "midi/midi.h"

using namespace avrlib;

namespace ambika {

// Maps a Patch byte offset to a byte within the active SeqTrack.
// Returns NULL for offsets with no SeqTrack equivalent.
static uint8_t* PatchAddrToSeqField(SeqTrack& tr, uint8_t address) {
  switch (address) {
    // OSC page — page1 (defaults[0..7])
    case 0:  return &tr.defaults[kP1WAVE1];  // osc1.shape
    case 1:  return &tr.defaults[kP1PARA1];  // osc1.parameter
    case 2:  return &tr.config[kCfgOSC1R];   // osc1.range
    case 3:  return &tr.defaults[kP1FINE];   // osc1.detune
    case 4:  return &tr.defaults[kP1WAVE2];  // osc2.shape
    case 5:  return &tr.defaults[kP1PARA2];  // osc2.parameter
    case 6:  return &tr.config[kCfgOSC2R];   // osc2.range
    case 7:  return &tr.config[kCfgOSC2D];   // osc2.detune
    // MIXER page — blend/ratio/sub (defaults[])
    case 8:  return &tr.defaults[kP1BLND];   // mix_balance
    case 9:  return &tr.config[kCfgFMOP];    // mix_operator (FM mode)
    case 10: return &tr.defaults[kP1RTIO];   // mix_parameter
    // MIXER page — page2 (defaults[8..15])
    case 11: return &tr.defaults[8 + kP2WSUB]; // mix_sub_osc_shape
    case 12: return &tr.defaults[8 + kP2SUB];  // mix_sub_osc level
    case 13: return &tr.defaults[8 + kP2NOIS]; // mix_noise
    case 14: return &tr.config[kCfgFUZZ];    // mix_fuzz
    case 25: return &tr.defaults[8 + kP2LPGD]; // env[0].decay  → LPG decay
    case 33: return &tr.defaults[8 + kP2LPGD]; // env[1].decay  → LPG decay
    case 41: return &tr.defaults[8 + kP2LPGD]; // env[2].decay  → LPG decay
    // MIXER/FILTER — config[]
    case 15: return &tr.config[kCfgBITS];  // mix_crush  → bit reduction
    case 16: return &tr.config[kCfgFREQ];  // filter cutoff
    case 17: return &tr.config[kCfgRES];   // filter resonance
    case 18: return &tr.config[kCfgTYPE];  // filter mode
    case 23: return &tr.config[kCfgLFOA];  // filter_lfo → LFO amount
    // ENV_LFO page — envelope attacks/sustains/releases + LFO params
    case 24: return &tr.config[kCfgE1ATK]; // env[0].attack
    case 26: return &tr.config[kCfgE1SUS]; // env[0].sustain
    case 27: return &tr.config[kCfgE1REL]; // env[0].release
    case 28: return &tr.config[kCfgLSHP];  // env[0].lfo_shape
    case 29: return &tr.config[kCfgLFOS];  // env[0].lfo_rate
    case 31: return &tr.config[kCfgLFOR];  // env[0].retrigger
    case 32: return &tr.config[kCfgE2ATK]; // env[1].attack
    case 34: return &tr.config[kCfgE2SUS]; // env[1].sustain
    case 35: return &tr.config[kCfgE2REL]; // env[1].release
    case 36: return &tr.config[kCfgLSHP];  // env[1].lfo_shape (shared)
    case 37: return &tr.config[kCfgLFOS];  // env[1].lfo_rate  (shared)
    case 39: return &tr.config[kCfgLFOR];  // env[1].retrigger (shared)
    case 40: return &tr.config[kCfgE3ATK]; // env[2].attack
    case 42: return &tr.config[kCfgE3SUS]; // env[2].sustain
    case 43: return &tr.config[kCfgE3REL]; // env[2].release
    case 44: return &tr.config[kCfgLSHP];  // env[2].lfo_shape (shared)
    case 45: return &tr.config[kCfgLFOS];  // env[2].lfo_rate  (shared)
    case 47: return &tr.config[kCfgLFOR];  // env[2].retrigger (shared)
    // VOICE_LFO page
    case 48: return &tr.config[kCfgLSHP];  // voice_lfo_shape
    case 49: return &tr.config[kCfgLFOS];  // voice_lfo_rate
    // FILTER KB tracking
    case 105: return &tr.config[kCfgTRAK]; // filter_kbt → pitch tracking
    default: return NULL;
  }
}

uint8_t Part::GetValue(uint8_t address) const {
  const SeqTrack& tr = sequencer.track(voice_id_);
  const uint8_t* p = PatchAddrToSeqField(
      const_cast<SeqTrack&>(tr), address);
  return p ? *p : 0;
}

void Part::SetValue(uint8_t address, uint8_t value, uint8_t) {
  voicecard_tx.WriteData(voice_id_, VOICECARD_DATA_PATCH, address, value);
  uint8_t* p = PatchAddrToSeqField(
      *sequencer.mutable_track(voice_id_), address);
  if (p) *p = value;
}

void Part::Touch() {
  static const uint8_t kSyncAddresses[] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 23,
    24, 25, 26, 27, 28, 29, 31,
    32, 33, 34, 35, 36, 37, 39,
    40, 41, 42, 43, 44, 45, 47,
    48, 49, 105,
  };
  for (uint8_t i = 0; i < sizeof(kSyncAddresses); ++i) {
    uint8_t addr = kSyncAddresses[i];
    voicecard_tx.WriteData(voice_id_, VOICECARD_DATA_PATCH, addr, GetValue(addr));
  }
  // VCA mod amount full cut: init_patch default (32) leaves VCA at 50% on release
  voicecard_tx.WriteData(voice_id_, VOICECARD_DATA_PATCH, 82, 63);
}

void Part::NoteOn(uint8_t note, uint8_t velocity) {
  if (velocity == 0) { NoteOff(note); return; }
  voicecard_tx.Trigger(voice_id_, static_cast<uint16_t>(note) << 7, velocity, 0);
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
