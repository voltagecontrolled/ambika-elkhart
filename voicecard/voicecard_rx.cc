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

#include "voicecard/voicecard_rx.h"

namespace ambika {

/* static */
RingBuffer<InputBufferSpecs> VoicecardProtocolRx::buffer_;

/* static */
uint8_t VoicecardProtocolRx::command_;

/* static */
uint8_t VoicecardProtocolRx::state_;

/* static */
uint8_t VoicecardProtocolRx::data_size_;

/* static */
uint8_t* VoicecardProtocolRx::data_ptr_;

/* static */
uint8_t VoicecardProtocolRx::rx_led_counter_;

/* static */
uint8_t VoicecardProtocolRx::arguments_[43];  // v4.4-WS1: 40-byte snapshot + note H/L + vel

// v4.4-WS1: 40-byte snapshot. Layout matches controller sequencer.cc FireStep:
//   page1[0..7]  (li 0..7)   — NOTE/WAVE1/PARA1/BLND/RTIO/WAVE2/PARA2/RANG
//                              li 7 = osc1 RANGE (reclaimed; was osc1 detune addr 3)
//   page2[0..7]  (li 8..15)  — E1DEC/TUN2/E2DEC/FIN2/E3DEC/FOLD/NOIS/SUB
//   page3[0..3]  (li 24..27) — FREQ/FAMT/PAMT/WAVE
//   ext[0..19]   (li 28..47) — xmod/fuzz/crsh/reso/mode/E1rise/E1curv/E1dept/
//                              E2rise/E2curv/E3rise/E3curv/L4rate/L4wave/
//                              L4dest/L4dept/L5rate/L5wave/L5dest/L5dept
/* static */
const uint8_t VoicecardProtocolRx::kSnapshotAddrs[40] PROGMEM = {
  0xff, 0,    1,    8,    10,   4,    5,    2,        // page1 (li 7 → addr 2 osc1 range)
  25,   6,    33,   7,    41,   111,  13,   12,       // page2
  16,   22,   58,   11,                                // page3
  9,    14,   15,   17,   18,   24,   26,   82,       // ext: xmod/fuzz/crsh/reso/mode/E1{rise,curv,dept→82}
  32,   34,   40,   42,                                // ext: E2 rise/curv, E3 rise/curv
  49,   48,   72,   73,                                // ext: LFO4 rate/wave/dest/dept
  109,  108,  69,   70,                                // ext: LFO5 rate/wave/dest/dept
};

/* static */
uint8_t VoicecardProtocolRx::lights_out_;

/* extern */
VoicecardProtocolRx voicecard_rx;

}  // namespace ambika
