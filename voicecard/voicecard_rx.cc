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
uint8_t VoicecardProtocolRx::arguments_[19];

// page1[]: NOTE skipped (sent as note bytes), WAVE1, PARA1, BLND, RTIO, WAVE2, PARA2, FINE.
// page2[]: E1DEC, TUN2 (osc2 coarse, addr 6), E2DEC, FIN2 (osc2 detune, addr 7),
//          E3DEC, E3REL dead, NOIS, SUB.
/* static */
const uint8_t VoicecardProtocolRx::kSnapshotAddrs[16] PROGMEM = {
  0xff, 0,    1,    8,    10,   4,    5,    3,
  25,   6,    33,   7,    41,   0xff, 13,   12,
};

/* static */
uint8_t VoicecardProtocolRx::lights_out_;

/* extern */
VoicecardProtocolRx voicecard_rx;

}  // namespace ambika
