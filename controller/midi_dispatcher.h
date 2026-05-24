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

#ifndef CONTROLLER_MIDI_DISPATCHER_H_
#define CONTROLLER_MIDI_DISPATCHER_H_

#include "avrlib/base.h"
#include "avrlib/ring_buffer.h"

#include "controller/display.h"
#include "controller/multi.h"
#include "controller/storage.h"
#include "controller/system_settings.h"
#include "controller/ui.h"

#include "midi/midi.h"

namespace ambika {

const uint8_t kDataEntryResendRate = 32;

struct LowPriorityBufferSpecs {
  enum {
    buffer_size = 128,
    data_size = 8,
  };
  typedef avrlib::DataTypeForSize<data_size>::Type Value;
};

struct HighPriorityBufferSpecs {
  enum {
    buffer_size = 8,
    data_size = 8,
  };
  typedef avrlib::DataTypeForSize<data_size>::Type Value;
};

class MidiDispatcher : public midi::MidiDevice {
 public:
  typedef avrlib::RingBuffer<LowPriorityBufferSpecs> OutputBufferLowPriority;
  typedef avrlib::RingBuffer<HighPriorityBufferSpecs> OutputBufferHighPriority;

  MidiDispatcher() { }

  // ------ MIDI in handling ---------------------------------------------------

  // Forwarded to the controller.
  static inline void NoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    display.set_status('\x01');
    if (!ui.OnNote(note, velocity)) {
      multi.NoteOn(channel, note, velocity);
    }
  }
  static inline void NoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    multi.NoteOff(channel, note, velocity);
  }

  // Handled.
  static inline void ControlChange(
      uint8_t channel,
      uint8_t controller,
      uint8_t value) {
    if (controller == midi::kBankMsb) {
      current_bank_ = value;
    } else {
      display.set_status('\x03');
      multi.ControlChange(channel, controller, value);
    }
  }
  static void AllSoundOff(uint8_t channel) {
    multi.AllSoundOff(channel);
  }
  static void ResetAllControllers(uint8_t channel) {
    multi.ResetAllControllers(channel);
  }
  static void AllNotesOff(uint8_t channel) {
    multi.AllNotesOff(channel);
  }
  static void OmniModeOff(uint8_t channel) {
    multi.OmniModeOff(channel);
  }
  static void OmniModeOn(uint8_t channel) {
    multi.OmniModeOn(channel);
  }
  
  static void ProgramChange(uint8_t channel, uint8_t program) {
    // Program change via MIDI loads the patch for the voice on that channel.
    if (!system_settings.rx_program_change()) { return; }
    if (channel >= kNumParts) { return; }
    StorageLocation location;
    location.object = STORAGE_OBJECT_PATCH;
    location.name = NULL;
    location.part = channel;
    location.bank = current_bank_ < 26 ? current_bank_ : 0;
    location.slot = program;
    storage.Load(location);
  }
  
  static void Reset() { multi.Reset(); }
  static void Clock() { 
    if (!multi.internal_clock()) {
      multi.Clock();
    }
  }
  static void Start() { multi.Start(); }
  static void Stop() { multi.Stop(); }
  static void Continue() { multi.Continue(); }
  
  static void SysExStart() {
    ProcessSysEx(0xf0);
  }
  static void SysExByte(uint8_t sysex_byte) {
    ProcessSysEx(sysex_byte);
  }
  static void SysExEnd() {
    ProcessSysEx(0xf7);
    if (Storage::sysex_rx_state() == RECEPTION_OK) {
      display.set_status('+');
    } else {
      display.set_status('#');
    }
  }
  
  static uint8_t CheckChannel(uint8_t channel) {
    return 1;
  }
  
  static void RawMidiData(
      uint8_t status,
      uint8_t* data,
      uint8_t data_size,
      uint8_t accepted_channel) {
    if (mode() == MIDI_OUT_FULL) {
      Send(status, data, data_size);
    } else if (mode() == MIDI_OUT_CHAIN) {
      // In chain mode, forward everything... except notes.
      uint8_t hi = status & 0xf0;
      if (hi != 0x80 && hi != 0x90) {
        Send(status, data, data_size);
      }
    }
  }
  
  static void RawByte(uint8_t byte) {
    if (mode() == MIDI_OUT_THRU) {
      OutputBufferLowPriority::Overwrite(byte);
    }
  }
  
  static uint8_t readable_high_priority() {
    return OutputBufferHighPriority::readable();
  }
  
  static uint8_t readable_low_priority() {
    return OutputBufferLowPriority::readable();
  }

  static uint8_t ImmediateReadHighPriority() {
    return OutputBufferHighPriority::ImmediateRead();
  }
  
  static uint8_t ImmediateReadLowPriority() {
    return OutputBufferLowPriority::ImmediateRead();
  }
  
  
  // ------ Generation of MIDI out messages ------------------------------------
  // Sequencer note emission: caller supplies the wire channel (0..15) which
  // comes from multi.track_channel(t)-1. Tracking stays per-track so EXT-mode
  // channel changes between Trigger and Release still pair correctly. Also
  // remembers the channel last used so the note-off lands on the same wire
  // channel even if the user reassigned mch in between.
  static inline void SequencerNoteOn(uint8_t track, uint8_t channel,
                                     uint8_t note, uint8_t velocity) {
    if (sequencer_note_[track] != 0xff) {
      Send3(0x80 | (sequencer_channel_[track] & 0x0f),
            sequencer_note_[track], 0);
    }
    channel &= 0x0f;
    note &= 0x7f;
    Send3(0x90 | channel, note, velocity & 0x7f);
    sequencer_note_[track] = note;
    sequencer_channel_[track] = channel;
  }

  static inline void SequencerNoteOff(uint8_t track) {
    if (sequencer_note_[track] == 0xff) return;
    Send3(0x80 | (sequencer_channel_[track] & 0x0f),
          sequencer_note_[track], 0);
    sequencer_note_[track] = 0xff;
  }

  // Live CC out for sequencer-side knob tweaks and step-lock fires on EXT
  // tracks. Caller supplies channel and value (already in 0..127).
  static inline void SendCc(uint8_t channel, uint8_t cc, uint8_t value7bit) {
    Send3(0xb0 | (channel & 0x0f), cc & 0x7f, value7bit & 0x7f);
  }

  // Fire-time EXT emit helpers. We emit the resolved value (lock-or-default)
  // every step so external gear gets the same "snap back to default" behavior
  // the internal synth gets from a full snapshot push. No dedup state — the
  // 60 bytes of RAM that would buy is more valuable kept as stack headroom.
  static inline void SendSlotCc(uint8_t channel, uint8_t cc, uint8_t value7) {
    if (cc > 127) return;  // slot is off / unassigned
    Send3(0xb0 | (channel & 0x0f), cc, value7 & 0x7f);
  }
  static inline void SendVamtCc(uint8_t channel, uint8_t value7) {
    Send3(0xb0 | (channel & 0x0f), 1, value7 & 0x7f);
  }
  static inline void SendGlidCc(uint8_t channel, uint8_t native_byte) {
    // GLID on EXT is CC 5 (Portamento Time). Byte 0..127 → CC 0..127 directly;
    // default 0 = no glide is correct for both INT (no portamento) and EXT
    // (no remote portamento).
    Send3(0xb0 | (channel & 0x0f), 5, native_byte & 0x7f);
  }

  // Clock-out gate: 2=OUT, 3=THR both send. 0=INT, 1=EXT suppress.
  static inline uint8_t clock_sends_out() {
    uint8_t m = multi.midi_clock_mode();
    return m == 2 || m == 3;
  }

  static inline void OnStart() {
    if (clock_sends_out()) SendNow(0xfa);
  }

  static inline void OnStop() {
    if (clock_sends_out()) SendNow(0xfc);
  }

  static inline void OnClock() {
    if (clock_sends_out()) SendNow(0xf8);
  }
  
  static inline void OnProgramLoaded(
      uint8_t channel,
      uint8_t bank,
      uint8_t program) {
    if (mode() == MIDI_OUT_FULL) {
      Send3(0xb0 | channel, 0x20, bank & 0x7f);
      // We send a program change + an active sensing message that does
      // strictly nothing. This way, we can use the already unrolled
      // Send3 function.
      Send3(0xc0 | channel, program & 0x7f, 0xfe);
    }
  }
  
  static inline void OnEdit(Part* part, uint8_t address, uint8_t value) {
    if (mode() < MIDI_OUT_CONTROLLER) {
      return;
    }
    if (mode() == MIDI_OUT_CHAIN && address == 127 && value == 4) {
      // Remap poly mode = chain to poly mode = poly.
      value = 1;
    }
    uint8_t channel = multi.part_channel(part);
    ++data_entry_counter_;
    if (current_parameter_address_ != address || data_entry_counter_ >= 32) {
      Send3(0xb0 | channel, midi::kNrpnMsb, (address & 0x80) ? 1 : 0);
      Send3(0xb0 | channel, midi::kNrpnLsb, address & 0x7f);
      current_parameter_address_ = address;
      data_entry_counter_ = 0;
    }
    uint8_t msb = (value & 0x80) ? 1 : 0;
    Send3(0xb0 | channel, midi::kDataEntryMsb, msb);
    Send3(0xb0 | channel, midi::kDataEntryLsb, value & 0x7f);
  }
  
  static void Send3(uint8_t status, uint8_t a, uint8_t b);
  static void SendBlocking(uint8_t byte);
  
  static void Flush() {
    while (OutputBufferLowPriority::readable());
  }

 private:
  static void Send(uint8_t status, uint8_t* data, uint8_t size);
  static void SendNow(uint8_t byte);
  static uint8_t mode() { return system_settings.data().midi_out_mode; }
  static void ProcessSysEx(uint8_t byte) {
    if (mode() == MIDI_OUT_FULL || mode() == MIDI_OUT_CHAIN) {
      Send(byte, NULL, 0);
    }
    Storage::SysExReceive(byte);
  }
  
  static uint8_t current_bank_;
  static uint8_t data_entry_counter_;
  static uint8_t current_parameter_address_;
  static uint8_t sequencer_note_[kNumVoices];
  static uint8_t sequencer_channel_[kNumVoices];
  
  DISALLOW_COPY_AND_ASSIGN(MidiDispatcher);
};

extern MidiDispatcher midi_dispatcher;

}  // namespace ambika

#endif  // CONTROLLER_MIDI_DISPATCHER_H_
