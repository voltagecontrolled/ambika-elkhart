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
#include "controller/voicecard_tx.h"
#include "midi/midi.h"

using namespace avrlib;

namespace ambika {

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
