// Copyright 2012 Emilie Gillet.
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

#include "voicecard/oscillator.h"

#include "voicecard/voicecard.h"

namespace ambika {

#define UPDATE_PHASE \
  if (*sync_input_++) { \
    phase.integral = 0; \
    phase.fractional = 0; \
  } \
  phase = U24AddC(phase, phase_increment_int); \
  *sync_output_++ = phase.carry; \

// This variant has a larger register width, but yields faster code.
#define UPDATE_PHASE_MORE_REGISTERS \
  if (*sync_input++) { \
    phase.integral = 0; \
    phase.fractional = 0; \
  } \
  phase = U24AddC(phase, phase_increment_int); \
  *sync_output++ = phase.carry; \

#define BEGIN_SAMPLE_LOOP \
  uint24c_t phase; \
  uint24_t phase_increment_int; \
  phase_increment_int.integral = phase_increment_.integral; \
  phase_increment_int.fractional = phase_increment_.fractional; \
  phase.integral = phase_.integral; \
  phase.fractional = phase_.fractional; \
  uint8_t size = kAudioBlockSize; \
  uint8_t* sync_input = sync_input_; \
  uint8_t* sync_output = sync_output_; \
  while (size--) {
  
#define END_SAMPLE_LOOP \
  } \
  phase_.integral = phase.integral; \
  phase_.fractional = phase.fractional;

#define CALCULATE_DIVISION_FACTOR(divisor, result_quotient, result_quotient_shifts) \
  uint16_t div_table_index = divisor; \
  int8_t result_quotient_shifts = 0; \
  while (div_table_index > 255) { \
    div_table_index >>= 1; \
    --result_quotient_shifts; \
  } \
  while (div_table_index < 128) { \
    div_table_index <<= 1; \
    ++result_quotient_shifts; \
  } \
  div_table_index -= 128; \
  uint8_t result_quotient = ResourcesManager::Lookup<uint8_t, uint8_t>( \
    wav_res_division_table, div_table_index);
  
#define CALCULATE_BLEP_INDEX(increment, quotient, quotient_shifts, result_blep_index) \
uint16_t result_blep_index = increment; \
int8_t shifts = quotient_shifts; \
while (shifts < 0) { \
  blep_index >>= 1; \
  ++shifts; \
} \
while (shifts > 0) { \
  blep_index <<= 1; \
  --shifts; \
} \
result_blep_index = U16U8MulShift8(result_blep_index, quotient);
  
// ------- Silence (useful when processing external signals) -----------------
void Oscillator::RenderSilence(uint8_t* buffer) {
  uint8_t size = kAudioBlockSize;
  while (size--) {
    *buffer++ = 128;
  }
}

// ------- Interpolation between two waveforms from two wavetables -----------
// The position is determined by the note pitch, to prevent aliasing.
void Oscillator::RenderSimpleWavetable(uint8_t* buffer) {
  uint8_t balance_index = U8Swap4(note_);
  uint8_t gain_2 = balance_index & 0xf0;
  uint8_t gain_1 = ~gain_2;
  uint8_t wave_1_index, wave_2_index;
  if (shape_ != WAVEFORM_SINE) {
    uint8_t wave_index = balance_index & 0xf;
    uint8_t base_resource_id = WAV_RES_BANDLIMITED_SAW_0;
    wave_1_index = base_resource_id + wave_index;
    wave_index = U8AddClip(wave_index, 1, kNumZonesFullSampleRate);
    wave_2_index = base_resource_id + wave_index;
  } else {
    wave_1_index = WAV_RES_SINE;
    wave_2_index = WAV_RES_SINE;
  }
  const prog_uint8_t* wave_1 = waveform_table[wave_1_index];
  const prog_uint8_t* wave_2 = waveform_table[wave_2_index];

  BEGIN_SAMPLE_LOOP
    UPDATE_PHASE_MORE_REGISTERS
    uint8_t sample = InterpolateTwoTables(
        wave_1, wave_2,
        phase.integral, gain_1, gain_2);
    if (sample < parameter_) {
      sample += parameter_ >> 1;
    }
    *buffer++ = sample;
  END_SAMPLE_LOOP
}

// ------- Casio CZ-like synthesis -------------------------------------------
void Oscillator::RenderCzSaw(uint8_t* buffer) {
  BEGIN_SAMPLE_LOOP
    UPDATE_PHASE
    uint8_t phi = phase.integral >> 8;
    uint8_t clipped_phi = phase.integral < 0x2000 ? phase.integral >> 5 : 0xff;
    // Interpolation causes more aliasing here.
    *buffer++ = ReadSample(wav_res_sine,
        U8MixU16(phi, clipped_phi, parameter_ << 1));
  END_SAMPLE_LOOP
}

// Windowed/shaped sine bank. PARA selects one of 8 variants:
//   0 = full sine
//   1 = half-wave rectified (positive half only, silence on negative)
//   2 = absolute / full-wave rectified
//   3 = quarter sine (rising quarter then silence)
//   4 = alternating sine (positive half at 2x speed, silence)
//   5 = camel sine (two abs-sine bumps per cycle)
//   6 = square (sign of sine)
//   7 = log saw (crude approximation via half-period sine ramp)
// Stepped select (PARA >> 4). Crossfading adjacent shapes was tried but
// compounded the wavetable aliasing — discontinuities from two shapes
// summed produced pitch-sensitive beating on high notes. Stepped is the
// cleaner-sounding option on this non-bandlimited bank.
void Oscillator::RenderSin16Bit(uint8_t* buffer) {
  uint8_t shape = parameter_ >> 4;
  BEGIN_SAMPLE_LOOP
    UPDATE_PHASE
    uint16_t p = phase.integral;
    uint8_t sample;
    switch (shape) {
      case 0:
        sample = ReadSample(wav_res_sine, p);
        break;
      case 1:
        sample = (p < 0x8000) ? ReadSample(wav_res_sine, p) : 128;
        break;
      case 2:
        sample = (p < 0x8000)
            ? ReadSample(wav_res_sine, p)
            : ReadSample(wav_res_sine, p - 0x8000);
        break;
      case 3:
        sample = (p < 0x4000) ? ReadSample(wav_res_sine, p) : 128;
        break;
      case 4:
        sample = (p < 0x8000) ? ReadSample(wav_res_sine, p << 1) : 128;
        break;
      case 5:
        if (p < 0x4000) {
          sample = ReadSample(wav_res_sine, p << 2);
        } else if (p < 0x8000) {
          sample = 128;
        } else if (p < 0xC000) {
          sample = ReadSample(wav_res_sine, (p - 0x8000) << 2);
        } else {
          sample = 128;
        }
        break;
      case 6:
        sample = (p < 0x8000) ? 255 : 0;
        break;
      default:  // 7
        sample = ReadSample(wav_res_sine, p >> 1);
        break;
    }
    *buffer++ = sample;
  END_SAMPLE_LOOP
}

// ------- FM ----------------------------------------------------------------
void Oscillator::RenderFm(uint8_t* buffer) {
  uint8_t offset = fm_parameter_ < 24 ? 0 : 
    (fm_parameter_ > 48 ? 24 : fm_parameter_-24);
  uint16_t multiplier = ResourcesManager::Lookup<uint16_t, uint8_t>(
      lut_res_fm_frequency_ratios, offset);
  uint16_t increment = (
      static_cast<int32_t>(phase_increment_.integral) * multiplier) >> 8;
  parameter_ <<= 1;
  
  uint16_t phase_2 = data_.secondary_phase;
  uint8_t last_output = data_.output_sample;
  uint8_t fb_phase_mod = 
    (shape_ == WAVEFORM_FM || parameter_ < 128) ? 0 : parameter_ - 128;
  BEGIN_SAMPLE_LOOP
    UPDATE_PHASE
    phase_2 += increment;
    uint8_t modulator = ReadSample(wav_res_sine,
        phase_2 + fb_phase_mod*last_output);
    uint16_t modulation = modulator * parameter_;
    last_output = InterpolateSample(wav_res_sine,
        phase.integral + modulation);
    *buffer++ = last_output;
  END_SAMPLE_LOOP
  data_.secondary_phase = phase_2;
  data_.output_sample = last_output;
}

// ------- 8-bit land --------------------------------------------------------
void Oscillator::Render8BitLand(uint8_t* buffer) {
  BEGIN_SAMPLE_LOOP
    UPDATE_PHASE
    uint8_t x = parameter_;
    *buffer++ = (((phase.integral >> 8) ^ (x << 1)) & (~x)) + (x >> 1);
  END_SAMPLE_LOOP
}

void Oscillator::RenderVowel(uint8_t* buffer) {
  data_.vw.update = (data_.vw.update + 1) & 3;
  if (!data_.vw.update) {
    uint8_t offset_1 = U8ShiftRight4(parameter_);
    offset_1 = U8U8Mul(offset_1, 7);
    uint8_t offset_2 = offset_1 + 7;
    uint8_t balance = parameter_ & 15;
    
    // Interpolate formant frequencies.
    for (uint8_t i = 0; i < 3; ++i) {
      data_.vw.formant_increment[i] = U8U4MixU12(
          ResourcesManager::Lookup<uint8_t, uint8_t>(
              wav_res_vowel_data, offset_1 + i),
          ResourcesManager::Lookup<uint8_t, uint8_t>(
              wav_res_vowel_data, offset_2 + i),
          balance);
      data_.vw.formant_increment[i] <<= 3;
    }
    
    // Interpolate formant amplitudes.
    for (uint8_t i = 0; i < 4; ++i) {
      uint8_t amplitude_a = ResourcesManager::Lookup<uint8_t, uint8_t>(
          wav_res_vowel_data,
          offset_1 + 3 + i);
      uint8_t amplitude_b = ResourcesManager::Lookup<uint8_t, uint8_t>(
          wav_res_vowel_data,
          offset_2 + 3 + i);
      data_.vw.formant_amplitude[i] = U8U4MixU8(
          amplitude_a,
          amplitude_b, balance);
    }
  }
  
  int16_t phase_noise = (shape_ == WAVEFORM_VOWEL) ? 
    S8S8Mul(Random::state_msb(), data_.vw.noise_modulation) : 0;
  BEGIN_SAMPLE_LOOP
    int8_t result = 0;
    uint8_t phaselet;
    
    data_.vw.formant_phase[0] += data_.vw.formant_increment[0];
    phaselet = (data_.vw.formant_phase[0] >> 8) & 0xf0;
    result = ResourcesManager::Lookup<uint8_t, uint8_t>(
        wav_res_formant_sine,
        phaselet | data_.vw.formant_amplitude[0]);

    data_.vw.formant_phase[1] += data_.vw.formant_increment[1];
    phaselet = (data_.vw.formant_phase[1] >> 8) & 0xf0;
    result += ResourcesManager::Lookup<uint8_t, uint8_t>(
        wav_res_formant_sine,
        phaselet | data_.vw.formant_amplitude[1]);
    
    data_.vw.formant_phase[2] += data_.vw.formant_increment[2];
    phaselet = (data_.vw.formant_phase[2] >> 8) & 0xf0;
    result += ResourcesManager::Lookup<uint8_t, uint8_t>(
        wav_res_formant_square,
        phaselet | data_.vw.formant_amplitude[2]);
    
    result = S8U8MulShift8(result, phase.integral >> 8);
    phase.integral -= phase_increment_int.integral;
    if ((phase.integral + phase_noise) < phase_increment_int.integral) {
      data_.vw.formant_phase[0] = 0;
      data_.vw.formant_phase[1] = 0;
      data_.vw.formant_phase[2] = 0;
    }
    uint8_t x = S16ClipS8(4 * result) + 128;
    *buffer++ = x;
    *buffer++ = x;
    size--;
  END_SAMPLE_LOOP
}

// ------- New Triangle (Non-band-limited and with different waveshaping) ----
void Oscillator::RenderNewTriangle(uint8_t* buffer) {
  BEGIN_SAMPLE_LOOP
    UPDATE_PHASE
    uint8_t tri = phase.integral >> 7;
    uint8_t v = phase.integral & 0x8000 ? tri : ~tri;
    if (v < parameter_) { // fold triangle
      v = (parameter_ << 1) - v;
    }
    *buffer++ = v;
  END_SAMPLE_LOOP
}

// ------- Dirty Pwm (kills kittens) -----------------------------------------
void Oscillator::RenderDirtyPwm(uint8_t* buffer) {
  BEGIN_SAMPLE_LOOP
    UPDATE_PHASE
    *buffer++ = (phase.integral >> 8) < 127 + parameter_ ? 0 : 255;
  END_SAMPLE_LOOP
}

// ------- Polyblep Saw ------------------------------------------------------
// Heavily inspired by Emilies experimental implementation for STM but
// dumbed down and much less generic (does not do polyblep for sync etc)
void Oscillator::RenderPolyBlepSaw(uint8_t* buffer) {
  
  // calculate (1/increment) for later multiplication with current phase
  CALCULATE_DIVISION_FACTOR(phase_increment_.integral, quotient, quotient_shifts)

  // Revert to pure saw (=single blep) to avoid cpu overload for high notes
  uint8_t mod_parameter = note_ > 107 ? 0 : parameter_;
  uint8_t high = phase_.integral >= 0x8000;

  uint8_t next_sample = data_.output_sample;
  BEGIN_SAMPLE_LOOP
    UPDATE_PHASE
    uint8_t this_sample = next_sample;

    // Compute naive waveform
    next_sample = (phase.integral < 0x8000) ?
      (phase.integral >> 8) :
      (phase.integral >> 8) - mod_parameter;

    if (phase.carry) {
      high = false;
      CALCULATE_BLEP_INDEX(phase.integral, quotient, quotient_shifts, blep_index)
      this_sample -= U8U8MulShift8(
        ResourcesManager::Lookup<uint8_t, uint8_t>(wav_res_blep_table, blep_index),
        255-mod_parameter /* scale blep to size of edge */);
      next_sample += U8U8MulShift8(
        ResourcesManager::Lookup<uint8_t, uint8_t>(wav_res_blep_table, 127-blep_index),
        255-mod_parameter /* scale blep to size of edge */);
    }
    else if (mod_parameter && !high && phase.integral >= 0x8000) {
      high = true;
      CALCULATE_BLEP_INDEX(phase.integral-0x8000, quotient, quotient_shifts, blep_index)
      this_sample -= U8U8MulShift8(
        ResourcesManager::Lookup<uint8_t, uint8_t>(wav_res_blep_table, blep_index),
        mod_parameter /* scale blep to size of edge */);
      next_sample += U8U8MulShift8(
        ResourcesManager::Lookup<uint8_t, uint8_t>(wav_res_blep_table, 127-blep_index),
        mod_parameter /* scale blep to size of edge */);
    }

    *buffer++ = this_sample;
  END_SAMPLE_LOOP

  data_.output_sample = next_sample;
}

// ------- Polyblep CS-80 Saw ------------------------------------------------
// Heavily inspired by Emilies experimental implementation for STM but
// dumbed down and much less generic (does not do polyblep for sync etc)
void Oscillator::RenderPolyBlepCSaw(uint8_t* buffer) {

  // calculate (1/increment) for later multiplication with current phase
  CALCULATE_DIVISION_FACTOR(phase_increment_.integral, quotient, quotient_shifts)

  // Revert to pure saw (=single blep) to avoid cpu overload for high notes
  uint8_t revert_to_saw = note_ > 107;

  // PWM modulation (constrained to extend over at least one increment)
  uint8_t pwm_limit = phase_increment_.integral >> 8;
  uint16_t pwm_phase =
    (parameter_ > 0 && parameter_ < pwm_limit) ?
    static_cast<uint16_t>(pwm_limit) << 8 :
    static_cast<uint16_t>(parameter_) << 8;
  uint8_t high = phase_.integral >= pwm_phase;

  uint8_t next_sample = data_.output_sample;
  BEGIN_SAMPLE_LOOP
    UPDATE_PHASE
    uint8_t this_sample = next_sample;

    // Compute naive waveform
    next_sample = (revert_to_saw || phase.integral >= pwm_phase) ?
      (phase.integral >> 8) : 0;

    if (phase.carry) {
      high = false;
      CALCULATE_BLEP_INDEX(phase.integral, quotient, quotient_shifts, blep_index)
      this_sample -= ResourcesManager::Lookup<uint8_t, uint8_t>(
        wav_res_blep_table, blep_index);
      next_sample += ResourcesManager::Lookup<uint8_t, uint8_t>(
        wav_res_blep_table, 127-blep_index);
    }
    else if (!revert_to_saw && /* no positive edge for pure saw */
      phase.integral >= pwm_phase && !high) {
      high = true;
      CALCULATE_BLEP_INDEX(phase.integral-pwm_phase, quotient, quotient_shifts, blep_index)
      this_sample += U8U8MulShift8(
        ResourcesManager::Lookup<uint8_t, uint8_t>(wav_res_blep_table, blep_index),
        parameter_ /* scale blep to size of edge */);
      next_sample -= U8U8MulShift8(
        ResourcesManager::Lookup<uint8_t, uint8_t>(wav_res_blep_table, 127-blep_index),
        parameter_ /* scale blep to size of edge */);
    }

    *buffer++ = this_sample;
  END_SAMPLE_LOOP

  data_.output_sample = next_sample;
}

// ------- Polyblep Pwm ------------------------------------------------------
// Heavily inspired by Emilies experimental implementation for STM but
// dumbed down and much less generic (does not do polyblep for sync etc)
void Oscillator::RenderPolyBlepPwm(uint8_t* buffer) {

  // calculate (1/increment) for later multiplication with current phase
  CALCULATE_DIVISION_FACTOR(phase_increment_.integral, quotient, quotient_shifts)

  // Revert to pure saw (=single blep) to avoid cpu overload for high notes
  uint8_t revert_to_saw = note_ > 107;
     
  // PWM modulation (constrained to extend over at least one increment) 
  uint8_t pwm_limit = 127 - (phase_increment_.integral >> 8);
  uint16_t pwm_phase = 
    (parameter_ < pwm_limit) ? /* prevent dual bleps at same increment */
    static_cast<uint16_t>(127 + parameter_) << 8 :
    static_cast<uint16_t>(127 + pwm_limit) << 8;
  uint8_t high = phase_.integral >= pwm_phase;
  
  uint8_t next_sample = data_.output_sample;
  BEGIN_SAMPLE_LOOP
    UPDATE_PHASE
    uint8_t this_sample = next_sample;

    // Compute naive waveform
    next_sample = revert_to_saw ? 
      (phase.integral >> 8) : (phase.integral < pwm_phase ? 0 : 255);

    if (phase.carry) {
      high = false;
      CALCULATE_BLEP_INDEX(phase.integral, quotient, quotient_shifts, blep_index)
      this_sample -= ResourcesManager::Lookup<uint8_t, uint8_t>(
        wav_res_blep_table, blep_index);
      next_sample += ResourcesManager::Lookup<uint8_t, uint8_t>(
        wav_res_blep_table, 127-blep_index);
    }
    else if (!revert_to_saw && /* no positive edge for pure saw */
      phase.integral >= pwm_phase && !high) {
      high = true;
      CALCULATE_BLEP_INDEX(phase.integral-pwm_phase, quotient, quotient_shifts, blep_index)
      this_sample += ResourcesManager::Lookup<uint8_t, uint8_t>(
        wav_res_blep_table, blep_index);
      next_sample -= ResourcesManager::Lookup<uint8_t, uint8_t>(
        wav_res_blep_table, 127-blep_index);
    }

    *buffer++ = this_sample;
  END_SAMPLE_LOOP

  data_.output_sample = next_sample;
}

// ------- Quad saw or pwm (mit aliasing) ------------------------------------
void Oscillator::RenderQuad(uint8_t* buffer) {
  uint16_t phase_spread = (
      static_cast<uint32_t>(phase_increment_.integral) * parameter_) >> 13;
  ++phase_spread;
  uint16_t phase_increment = phase_increment_.integral;
  uint16_t increments[3];
  for (uint8_t i = 0; i < 3; ++i) {
    phase_increment += phase_spread;
    increments[i] = phase_increment;
  }
  
  if (shape_ == WAVEFORM_QUAD_SAW_PAD) {
    BEGIN_SAMPLE_LOOP
      UPDATE_PHASE
      data_.qs.phase[0] += increments[0];
      data_.qs.phase[1] += increments[1];
      data_.qs.phase[2] += increments[2];
      uint8_t value = (phase.integral >> 10);
      value += (data_.qs.phase[0] >> 10);
      value += (data_.qs.phase[1] >> 10);
      value += (data_.qs.phase[2] >> 10);
      *buffer++ = value;
    END_SAMPLE_LOOP
  }
  else { // WAVEFORM_QUAD_PWM
    uint8_t pwm_phase = 127 + parameter_;
    BEGIN_SAMPLE_LOOP
      UPDATE_PHASE
      data_.qs.phase[0] += increments[0];
      data_.qs.phase[1] += increments[1];
      data_.qs.phase[2] += increments[2];
      uint8_t value = phase.integral < (pwm_phase << 8) ? 0 : 63;
      if (data_.qsbytes[1] >= pwm_phase) value += 63;
      if (data_.qsbytes[3] >= pwm_phase) value += 63;
      if (data_.qsbytes[5] >= pwm_phase) value += 63;
      *buffer++ = value;
    END_SAMPLE_LOOP
  }
}

// ------- Low-passed, then high-passed white noise --------------------------
void Oscillator::RenderFilteredNoise(uint8_t* buffer) {
  uint16_t rng_state = data_.no.rng_state;
  if (rng_state == 0) {
    ++rng_state;
  }
  uint8_t filter_coefficient = parameter_ << 2;
  if (filter_coefficient <= 4) {
    filter_coefficient = 4;
  }
  BEGIN_SAMPLE_LOOP
    if (*sync_input_++) {
      rng_state = data_.no.rng_reset_value;
    }
    rng_state = (rng_state >> 1) ^ (-(rng_state & 1) & 0xb400);
    uint8_t noise_sample = rng_state >> 8;
    // This trick is used to avoid having a DC component (no innovation) when
    // the parameter is set to its minimal or maximal value.
    data_.no.lp_noise_sample = U8Mix(
        data_.no.lp_noise_sample,
        noise_sample,
        filter_coefficient);
    if (parameter_ >= 64) {
      *buffer++ = noise_sample - data_.no.lp_noise_sample - 128;
    } else {
      *buffer++ = data_.no.lp_noise_sample;
    }
  END_SAMPLE_LOOP
  data_.no.rng_state = rng_state;
}

// The position is freely determined by the parameter
void Oscillator::RenderInterpolatedWavetable(uint8_t* buffer) {
  // Which wavetable should we play?.
  const prog_uint8_t* wavetable_definition = 
      wav_res_wavetables + U8U8Mul(
          shape_ - WAVEFORM_WAVETABLE_1,
          18);
  // Get a 8:8 value with the wave index in the first byte, and the
  // balance amount in the second byte.
  uint8_t num_steps = ResourcesManager::Lookup<uint8_t, uint8_t>(
      wavetable_definition,
      0);
  uint16_t pointer = U8U8Mul(parameter_ << 1, num_steps);
  uint16_t wave_index_1 = ResourcesManager::Lookup<uint8_t, uint8_t>(
      wavetable_definition,
      1 + (pointer >> 8));
  uint16_t wave_index_2 = ResourcesManager::Lookup<uint8_t, uint8_t>(
      wavetable_definition,
      2 + (pointer >> 8));
  uint8_t gain = pointer & 0xff;
  const prog_uint8_t* wave_1 = wav_res_waves + U8U8Mul(
      wave_index_1,
      129);
  const prog_uint8_t* wave_2 = wav_res_waves + U8U8Mul(
      wave_index_2,
      129);
  BEGIN_SAMPLE_LOOP
    UPDATE_PHASE_MORE_REGISTERS
    *buffer++ = InterpolateTwoTables(
        wave_1,
        wave_2,
        phase.integral >> 1,
        ~gain,
        gain);
  END_SAMPLE_LOOP
}

// The position is freely determined by the parameter
void Oscillator::RenderWavequence(uint8_t* buffer) {
  const prog_uint8_t* wave = wav_res_waves + U8U8Mul(
      parameter_,
      129);
  BEGIN_SAMPLE_LOOP
    UPDATE_PHASE
    *buffer++ = InterpolateSample(wave, phase.integral >> 1);
  END_SAMPLE_LOOP
}

/* static */
const Oscillator::RenderFn Oscillator::fn_table_[] PROGMEM = {
  &Oscillator::RenderSilence,

  &Oscillator::RenderPolyBlepSaw,
  &Oscillator::RenderPolyBlepPwm,
  &Oscillator::RenderNewTriangle,
  &Oscillator::RenderSimpleWavetable,

  &Oscillator::RenderCzSaw,
  &Oscillator::RenderSin16Bit,

  &Oscillator::RenderQuad,
  
  &Oscillator::RenderFm,
  
  &Oscillator::Render8BitLand,
  &Oscillator::RenderDirtyPwm,
  &Oscillator::RenderFilteredNoise,
  &Oscillator::RenderVowel,
  
  &Oscillator::RenderInterpolatedWavetable,
  
  &Oscillator::RenderSimpleWavetable,
  &Oscillator::RenderQuad,
  &Oscillator::RenderFm,
  &Oscillator::RenderPolyBlepCSaw,
  &Oscillator::RenderVowel
};

}  // namespace