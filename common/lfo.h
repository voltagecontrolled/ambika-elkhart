// Copyright 2009 Emilie Gillet.
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
//
// LFO (cheap oscillator).
//
// Contrary to oscillators which are templatized "static singletons", to
// generate the fastest, most specialized code, LFOs are less
// performance-sensitive and are thus implemented as a traditional class.

#ifndef COMMON_LFO_H_
#define COMMON_LFO_H_

#include "avrlib/base.h"
#include "avrlib/op.h"
#include "avrlib/random.h"
#include "common/patch.h"

using avrlib::InterpolateSample;
using avrlib::Random;

namespace ambika {

extern const prog_uint8_t wav_res_lfo_waveforms[] PROGMEM;
extern const prog_uint8_t wav_res_env_expo[] PROGMEM;

class Lfo {
 public:
  Lfo() { }

  uint8_t Render(uint8_t shape) {
    // Sync mode (phase_increment_ == 0): tick() owns phase advance and the
    // looped_ flag. Free-run mode: advance here. We must not clobber looped_
    // when in sync mode — S&H and one-shot shapes rely on tick's rollover
    // signal to know when a cycle completed.
    if (phase_increment_) {
      phase_ += phase_increment_;
      looped_ = phase_ < phase_increment_;
    }

    // One-shot shapes: hold at 0 once the cycle has completed. Termination
    // fires on any rollover (free-run or sync-tick) — the looped_ flag
    // unifies both sources.
    if (shape >= LFO_WAVEFORM_ONE_SHOT_EXP && shape <= LFO_WAVEFORM_ONE_SHOT_TRI) {
      if (one_shot_done_) {
        return 0;
      }
      if (looped_) {
        one_shot_done_ = 1;
        phase_ = 0xffff;
      }
      switch (shape) {
        case LFO_WAVEFORM_ONE_SHOT_EXP:
          // wav_res_env_expo is an exponential approach curve (1 - e^(-4t)):
          // rises fast at first then asymptotes. We want the *complementary*
          // exp decay (255 → 0, fast then slow), so subtract from 255.
          return 255 - pgm_read_byte(wav_res_env_expo + (phase_ >> 8));
        case LFO_WAVEFORM_ONE_SHOT_LIN:
          return 255 - (phase_ >> 8);
        case LFO_WAVEFORM_ONE_SHOT_TRI:
        default:
          return (phase_ & 0x8000) ?
              ~static_cast<uint8_t>(phase_ >> 7) :
              phase_ >> 7;
      }
    }

    // Compute the LFO value.
    uint8_t value;
    switch (shape) {
      case LFO_WAVEFORM_RAMP:
        value = phase_ >> 8;
        break;

      case LFO_WAVEFORM_S_H:
        if (looped_) {
          value_ = Random::GetByte();
        }
        value = value_;
        break;

      case LFO_WAVEFORM_TRIANGLE:
        value = (phase_ & 0x8000) ?
            phase_ >> 7 :
            ~static_cast<uint8_t>(phase_ >> 7);
        break;

      case LFO_WAVEFORM_SQUARE:
        value = (phase_ & 0x8000) ? 255 : 0;
        break;

#ifndef DISABLE_WAVETABLE_LFOS
      default:
        {
          uint16_t offset = avrlib::U8U8Mul(shape - LFO_WAVEFORM_WAVE_1, 129);
          const prog_uint8_t* wave = wav_res_lfo_waveforms + offset;
          if (shape < LFO_WAVEFORM_WAVE_15) {
            value = InterpolateSample(wave, phase_ >> 1);
          }
          else { // hack to avoid interpolation of the 'steppy' lfo waveforms
            value = pgm_read_byte(wave + (phase_ >> 9));
          }
        }
        break;
#endif  // DISABLE_WAVETABLE_LFOS
    }
    return value;
  }

  void set_phase(uint16_t phase) {
    looped_ = phase <= phase_;
    phase_ = phase;
    if (phase == 0) {
      one_shot_done_ = 0;
    }
  }


  void set_phase_increment(uint16_t phase_increment) {
    phase_increment_ = phase_increment;
  }

  // Advance phase by a tick increment without clearing one-shot state.
  // Used by MIDI clock sync — the controller pushes one tick per clock
  // and the voicecard advances synced LFOs by the configured per-tick
  // phase increment.
  void tick(uint16_t increment) {
    uint16_t new_phase = phase_ + increment;
    looped_ = new_phase < phase_;
    phase_ = new_phase;
  }

  uint8_t looped() const { return looped_; }

 private:
  // Phase increment.
  uint16_t phase_increment_;

  // Current phase of the lfo.
  uint16_t phase_;
  uint8_t looped_;

  // Current value of S&H.
  uint8_t value_;
  uint8_t step_;

  // Latched at end of one-shot cycle; cleared by set_phase(0).
  uint8_t one_shot_done_;

  DISALLOW_COPY_AND_ASSIGN(Lfo);
};

}  // namespace ambika

#endif  // COMMON_LFO_H_
