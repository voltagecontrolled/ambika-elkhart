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

#include <avr/pgmspace.h>
#include "avrlib/op.h"
#include "controller/resources.h"
#include "controller/sequencer.h"
#include "controller/voicecard_tx.h"
#include "midi/midi.h"

using namespace avrlib;

namespace ambika {

// Maps a Patch byte offset to a byte within the active SeqTrack.
// Returns NULL for offsets with no SeqTrack equivalent.
static uint8_t* PatchAddrToSeqField(SeqTrack& tr, uint8_t address) {
  switch (address) {
    // OSC page — page1 defaults
    case 0:  return &tr.defaults[kP1WAVE1];
    case 1:  return &tr.defaults[kP1PARA1];
    case 2:  return &tr.config[kCfgOSC1R];
    case 3:  return &tr.defaults[kP1FINE];
    case 4:  return &tr.defaults[kP1WAVE2];
    case 5:  return &tr.defaults[kP1PARA2];
    case 6:  return &tr.defaults[8 + kP2TUN2];   // lockable per-step
    case 7:  return &tr.defaults[8 + kP2FIN2];   // lockable per-step
    // Mixer
    case 8:  return &tr.defaults[kP1BLND];
    case 9:  return &tr.config[kCfgFMOP];
    case 10: return &tr.defaults[kP1RTIO];
    case 11: return &tr.config[kCfgWSUB];          // sub-osc shape (non-lockable)
    case 12: return &tr.defaults[8 + kP2SUB];
    case 13: return &tr.defaults[8 + kP2NOIS];
    case 14: return &tr.config[kCfgFUZZ];
    case 15: return &tr.config[kCfgBITS];
    // Filter
    case 16: return &tr.config[kCfgFREQ];
    case 17: return &tr.config[kCfgRES];
    case 18: return &tr.config[kCfgTYPE];
    case 22: return &tr.config[kCfgE2DEPT];        // filter_env = ENV2→VCF depth
    // Envelope attacks (voice-wide config)
    case 24: return &tr.config[kCfgE1ATK];
    case 32: return &tr.config[kCfgE2ATK];
    case 40: return &tr.config[kCfgE3ATK];
    // Envelope decays (per-step lockable)
    case 25: return &tr.defaults[8 + kP2E1DEC];
    case 33: return &tr.defaults[8 + kP2E2DEC];
    case 41: return &tr.defaults[8 + kP2E3DEC];
    // Envelope curves (voice-wide, sustain byte repurposed)
    case 26: return &tr.config[kCfgE1CRV];
    case 34: return &tr.config[kCfgE2CRV];
    case 42: return &tr.config[kCfgE3CRV];
    // Envelope releases — voicecard ignores these bytes (release_mod was
    // removed in 0x21). Only addr 43 still resolves so SetValue(43,…) has
    // a place to land for legacy MIDI/CC paths; the resolved byte is the
    // dead E3REL slot in defaults.page2.
    case 43: return &tr.defaults[8 + kP2E3REL];
    // LFO4 (voice_lfo on voicecard)
    case 48: return &tr.config[kCfgLSHP];          // voice_lfo_shape
    case 49: return &tr.config[kCfgLFOS];          // voice_lfo_rate
    // Configurable mod amounts (fixed routing slots)
    case 58: return &tr.config[kCfgE3DEPT];        // slot 2 amount: ENV3→pitch depth
    case 72: return &tr.config[kCfgLFO4D];         // slot 7 dest: LFO4 destination
    case 73: return &tr.config[kCfgLFO4A];         // slot 7 amount: LFO4 amount
    case 82: return &tr.config[kCfgE1DEPT];        // slot 10 amount: ENV1→VCA depth
    // Filter KB tracking
    case 105: return &tr.config[kCfgTRAK];
    // EG depth (virtual; indexed by active_env_lfo; 200=Amp/E1, 201=Filt/E2, 202=Pitch/E3)
    case 200: return &tr.config[kCfgE1DEPT];
    case 201: return &tr.config[kCfgE2DEPT];
    case 202: return &tr.config[kCfgE3DEPT];
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
  uint8_t tx_addr = address;
  if (address == 200) tx_addr = 82;
  else if (address == 201) tx_addr = 22;
  else if (address == 202) tx_addr = 58;
  voicecard_tx.WriteData(voice_id_, VOICECARD_DATA_PATCH, tx_addr, value);
  uint8_t* p = PatchAddrToSeqField(
      *sequencer.mutable_track(voice_id_), address);
  if (p) *p = value;
}

void Part::Touch() {
  // Send fixed mod routing base (42 bytes) from PROGMEM, then override amounts.
  for (uint8_t i = 0; i < 42; ++i) {
    voicecard_tx.WriteData(voice_id_, VOICECARD_DATA_PATCH, 50 + i,
        pgm_read_byte(&kDefaultMod[i]));
  }
  // All other patch addresses (configurable amounts override kDefaultMod slots).
  static const uint8_t kSyncAddresses[] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 22,
    24, 25, 26,
    32, 33, 34,
    40, 41, 42,
    48, 49,
    58, 72, 73, 82,
    105,
    200, 201, 202,
  };
  for (uint8_t i = 0; i < sizeof(kSyncAddresses); ++i) {
    uint8_t addr = kSyncAddresses[i];
    voicecard_tx.WriteData(voice_id_, VOICECARD_DATA_PATCH, addr, GetValue(addr));
  }
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
