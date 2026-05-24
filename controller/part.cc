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
    case 2:  return &tr.defaults[kP1FINE];         // v4.4-WS1: slot 7 reclaimed for OSC1 RANGE (now lockable)
    case 3:  return NULL;                          // osc1 detune — WS0 soft-dropped, no storage
    case 4:  return &tr.defaults[kP1WAVE2];
    case 5:  return &tr.defaults[kP1PARA2];
    case 6:  return &tr.defaults[8 + kP2TUN2];   // lockable per-step
    case 7:  return &tr.defaults[8 + kP2FIN2];   // lockable per-step
    // Mixer
    case 8:  return &tr.defaults[kP1BLND];
    case 9:  return &tr.defaults[28];              // WS1: xmod (mix_op), lock 28
    case 10: return &tr.defaults[kP1RTIO];
    case 11: return &tr.defaults[24 + kP3WAVE];    // sub-osc shape (now lockable)
    case 12: return &tr.defaults[8 + kP2SUB];
    case 13: return &tr.defaults[8 + kP2NOIS];
    case 14: return &tr.defaults[29];              // WS1: fuzz, lock 29
    case 15: return &tr.defaults[30];              // WS1: crsh, lock 30
    // Filter
    case 16: return &tr.defaults[24 + kP3FREQ];    // filter cutoff (now lockable)
    case 17: return &tr.defaults[31];              // WS1: reso, lock 31
    case 18: return &tr.defaults[32];              // WS1: mode, lock 32
    case 22: return &tr.defaults[24 + kP3FAMT];    // ENV2→VCF depth (now lockable)
    // Envelopes — all rise/fall/curv/depth now lockable (WS1)
    case 24: return &tr.defaults[33];              // WS1: E1 rise, lock 33
    case 25: return &tr.defaults[8 + kP2E1DEC];    // E1 fall, lock 8
    case 26: return &tr.defaults[34];              // WS1: E1 curv, lock 34
    case 32: return &tr.defaults[36];              // WS1: E2 rise, lock 36
    case 33: return &tr.defaults[8 + kP2E2DEC];    // E2 fall, lock 10
    case 34: return &tr.defaults[37];              // WS1: E2 curv, lock 37
    case 40: return &tr.defaults[38];              // WS1: E3 rise, lock 38
    case 41: return &tr.defaults[8 + kP2E3DEC];    // E3 fall, lock 12
    case 42: return &tr.defaults[39];              // WS1: E3 curv, lock 39
    // LFO4 (voice_lfo on voicecard) — all four cells lockable (WS1)
    case 48: return &tr.defaults[41];              // WS1: LFO4 wave (voice_lfo_shape), lock 41
    case 49: return &tr.defaults[40];              // WS1: LFO4 rate (voice_lfo_rate), lock 40
    // Configurable mod amounts (fixed routing slots)
    case 58: return &tr.defaults[24 + kP3PAMT];    // ENV3→pitch depth (PAMT, lock 26)
    case 72: return &tr.defaults[42];              // WS1: LFO4 dest (mod slot 7), lock 42
    case 73: return &tr.defaults[43];              // WS1: LFO4 dept (mod slot 7), lock 43
    case 82: return &tr.defaults[35];              // WS1: E1 depth (ENV1→VCA mod slot 10 amt), lock 35
    case 85:  return &tr.config[kCfgVELAMT];        // slot 11 amount: vel→VCA depth
    case 203: return &tr.config[kCfgSMTH];          // portamento (VOICECARD_DATA_PART)
    // Filter KB tracking
    case 105: return &tr.config[kCfgTRAK];
    // Trigger-time reset toggles (encoder click toggles them on OSC1 SHAPE,
    // LFO4 SHAPE, LFO5 SHAPE cells respectively).
    case 106: return &tr.config[kCfgPHSE];        // osc 1 phase reset
    case 107: return &tr.config[kCfgLFOR];        // LFO4 retrigger
    // LFO5 — WS1 lockable; storage migrated from config[] to defaults[].
    case 69:  return &tr.defaults[46];              // WS1: LFO5 dest (mod slot 6), lock 46
    case 70:  return &tr.defaults[47];              // WS1: LFO5 dept (mod slot 6), lock 47
    case 108: return &tr.defaults[45];              // WS1: LFO5 wave, lock 45
    case 109: return &tr.defaults[44];              // WS1: LFO5 rate, lock 44
    case 110: return &tr.config[kCfgL5RT];          // LFO5 retrigger (toggle only)
    case 111: return &tr.defaults[8 + kP2FOLD];     // pre-filter wavefolder (lockable)
    // EG depth (virtual; indexed by active_env_lfo; 200=Amp/E1, 201=Filt/E2, 202=Pitch/E3)
    case 200: return &tr.defaults[35];              // WS1: E1 depth, same slot as case 82
    case 201: return &tr.defaults[24 + kP3FAMT];    // E2 depth = FAMT, lock 25
    case 202: return &tr.defaults[24 + kP3PAMT];    // E3 depth = PAMT, lock 26
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
  if (address == 203) {
    // Portamento lives in the voicecard Part struct, not the Patch.
    voicecard_tx.WriteData(voice_id_, VOICECARD_DATA_PART, 6, value);
    sequencer.mutable_track(voice_id_)->config[kCfgSMTH] = value;
    return;
  }
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
    58, 69, 70, 72, 73, 82, 85,
    105, 106, 107, 108, 109, 110,
  };
  for (uint8_t i = 0; i < sizeof(kSyncAddresses); ++i) {
    uint8_t addr = kSyncAddresses[i];
    voicecard_tx.WriteData(voice_id_, VOICECARD_DATA_PATCH, addr, GetValue(addr));
  }
  // Portamento lives in the voicecard Part struct (not Patch).
  voicecard_tx.WriteData(voice_id_, VOICECARD_DATA_PART, 6,
      sequencer.track(voice_id_).config[kCfgSMTH]);
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
