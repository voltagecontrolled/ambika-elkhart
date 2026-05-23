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
//
// Resources definitions.
//
// Automatically generated with:
// make resources


#ifndef VOICECARD_RESOURCES_H_
#define VOICECARD_RESOURCES_H_


#include "avrlib/base.h"

#include <avr/pgmspace.h>


#include "avrlib/resources_manager.h"

namespace ambika {

typedef uint8_t ResourceId;

extern const prog_char* const string_table[];

extern const prog_uint16_t* const lookup_table_table[];

extern const prog_uint8_t* const waveform_table[];

extern const prog_uint16_t lut_res_lfo_increments[] PROGMEM;
extern const prog_uint16_t lut_res_env_portamento_increments[] PROGMEM;
extern const prog_uint16_t lut_res_oscillator_increments[] PROGMEM;
extern const prog_uint16_t lut_res_fm_frequency_ratios[] PROGMEM;
extern const prog_uint16_t lut_res_vca_linearization[] PROGMEM;
extern const prog_uint8_t wav_res_formant_sine[] PROGMEM;
extern const prog_uint8_t wav_res_formant_square[] PROGMEM;
extern const prog_uint8_t wav_res_sine[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_saw_0[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_saw_1[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_saw_2[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_saw_3[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_saw_4[] PROGMEM;
extern const prog_uint8_t wav_res_bandlimited_saw_5[] PROGMEM;
extern const prog_uint8_t wav_res_vowel_data[] PROGMEM;
extern const prog_uint8_t wav_res_distortion[] PROGMEM;
extern const prog_uint8_t wav_res_fold[] PROGMEM;
extern const prog_uint8_t wav_res_lfo_waveforms[] PROGMEM;
extern const prog_uint8_t wav_res_env_expo[] PROGMEM;
extern const prog_uint8_t wav_res_division_table[] PROGMEM;
extern const prog_uint8_t wav_res_blep_table[] PROGMEM;
extern const prog_uint8_t wav_res_waves[] PROGMEM;
extern const prog_uint8_t wav_res_wavetables[] PROGMEM;
#define STR_RES_DUMMY 0  // dummy
#define LUT_RES_LFO_INCREMENTS 0
#define LUT_RES_LFO_INCREMENTS_SIZE 128
#define LUT_RES_ENV_PORTAMENTO_INCREMENTS 1
#define LUT_RES_ENV_PORTAMENTO_INCREMENTS_SIZE 128
#define LUT_RES_OSCILLATOR_INCREMENTS 2
#define LUT_RES_OSCILLATOR_INCREMENTS_SIZE 97
#define LUT_RES_FM_FREQUENCY_RATIOS 3
#define LUT_RES_FM_FREQUENCY_RATIOS_SIZE 25
#define LUT_RES_VCA_LINEARIZATION 4
#define LUT_RES_VCA_LINEARIZATION_SIZE 256
#define WAV_RES_FORMANT_SINE 0
#define WAV_RES_FORMANT_SINE_SIZE 256
#define WAV_RES_FORMANT_SQUARE 1
#define WAV_RES_FORMANT_SQUARE_SIZE 256
#define WAV_RES_SINE 2
#define WAV_RES_SINE_SIZE 257
#define WAV_RES_BANDLIMITED_SAW_0 3
#define WAV_RES_BANDLIMITED_SAW_0_SIZE 257
#define WAV_RES_BANDLIMITED_SAW_1 4
#define WAV_RES_BANDLIMITED_SAW_1_SIZE 257
#define WAV_RES_BANDLIMITED_SAW_2 5
#define WAV_RES_BANDLIMITED_SAW_2_SIZE 257
#define WAV_RES_BANDLIMITED_SAW_3 6
#define WAV_RES_BANDLIMITED_SAW_3_SIZE 257
#define WAV_RES_BANDLIMITED_SAW_4 7
#define WAV_RES_BANDLIMITED_SAW_4_SIZE 257
#define WAV_RES_BANDLIMITED_SAW_5 8
#define WAV_RES_BANDLIMITED_SAW_5_SIZE 257
#define WAV_RES_BANDLIMITED_SAW_6 9
#define WAV_RES_BANDLIMITED_SAW_6_SIZE 257
#define WAV_RES_VOWEL_DATA 10
#define WAV_RES_VOWEL_DATA_SIZE 63
#define WAV_RES_DISTORTION 11
#define WAV_RES_DISTORTION_SIZE 256
#define WAV_RES_FOLD 12
#define WAV_RES_FOLD_SIZE 256
#define WAV_RES_LFO_WAVEFORMS 13
#define WAV_RES_LFO_WAVEFORMS_SIZE 2
#define WAV_RES_ENV_EXPO 14
#define WAV_RES_ENV_EXPO_SIZE 257
#define WAV_RES_DIVISION_TABLE 15
#define WAV_RES_DIVISION_TABLE_SIZE 128
#define WAV_RES_BLEP_TABLE 16
#define WAV_RES_BLEP_TABLE_SIZE 128
#define WAV_RES_WAVES 17
#define WAV_RES_WAVES_SIZE 10320
#define WAV_RES_WAVETABLES 18
#define WAV_RES_WAVETABLES_SIZE 288
typedef avrlib::ResourcesManager<
    ResourceId,
    avrlib::ResourcesTables<
        string_table,
        lookup_table_table> > ResourcesManager; 

}  // namespace ambika

#endif  // VOICECARD_RESOURCES_H_
