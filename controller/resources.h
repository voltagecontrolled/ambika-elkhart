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
// Resources definitions.
//
// Automatically generated with:
// make resources


#ifndef CONTROLLER_RESOURCES_H_
#define CONTROLLER_RESOURCES_H_


#include "avrlib/base.h"

#include <avr/pgmspace.h>


#include "avrlib/resources_manager.h"

namespace ambika {

typedef uint16_t ResourceId;

extern const prog_char* const string_table[];

extern const prog_uint16_t* const lookup_table_table[];

extern const prog_uint8_t* const character_table[];

extern const prog_uint8_t* const waveform_table[];

extern const prog_uint16_t lut_res_lfo_increments[] PROGMEM;
extern const prog_uint16_t lut_res_scale_just[] PROGMEM;
extern const prog_uint16_t lut_res_scale_pythagorean[] PROGMEM;
extern const prog_uint16_t lut_res_scale_1_4_eb[] PROGMEM;
extern const prog_uint16_t lut_res_scale_1_4_e[] PROGMEM;
extern const prog_uint16_t lut_res_scale_1_4_ea[] PROGMEM;
extern const prog_uint16_t lut_res_scale_bhairav[] PROGMEM;
extern const prog_uint16_t lut_res_scale_gunakri[] PROGMEM;
extern const prog_uint16_t lut_res_scale_marwa[] PROGMEM;
extern const prog_uint16_t lut_res_scale_shree[] PROGMEM;
extern const prog_uint16_t lut_res_scale_purvi[] PROGMEM;
extern const prog_uint16_t lut_res_scale_bilawal[] PROGMEM;
extern const prog_uint16_t lut_res_scale_yaman[] PROGMEM;
extern const prog_uint16_t lut_res_scale_kafi[] PROGMEM;
extern const prog_uint16_t lut_res_scale_bhimpalasree[] PROGMEM;
extern const prog_uint16_t lut_res_scale_darbari[] PROGMEM;
extern const prog_uint16_t lut_res_scale_rageshree[] PROGMEM;
extern const prog_uint16_t lut_res_scale_khamaj[] PROGMEM;
extern const prog_uint16_t lut_res_scale_mimal[] PROGMEM;
extern const prog_uint16_t lut_res_scale_parameshwari[] PROGMEM;
extern const prog_uint16_t lut_res_scale_rangeshwari[] PROGMEM;
extern const prog_uint16_t lut_res_scale_gangeshwari[] PROGMEM;
extern const prog_uint16_t lut_res_scale_kameshwari[] PROGMEM;
extern const prog_uint16_t lut_res_scale_pa__kafi[] PROGMEM;
extern const prog_uint16_t lut_res_scale_natbhairav[] PROGMEM;
extern const prog_uint16_t lut_res_scale_m_kauns[] PROGMEM;
extern const prog_uint16_t lut_res_scale_bairagi[] PROGMEM;
extern const prog_uint16_t lut_res_scale_b_todi[] PROGMEM;
extern const prog_uint16_t lut_res_scale_chandradeep[] PROGMEM;
extern const prog_uint16_t lut_res_scale_kaushik_todi[] PROGMEM;
extern const prog_uint16_t lut_res_scale_jogeshwari[] PROGMEM;
extern const prog_uint16_t lut_res_arpeggiator_patterns[] PROGMEM;
extern const prog_uint16_t lut_res_groove_swing[] PROGMEM;
extern const prog_uint16_t lut_res_groove_shuffle[] PROGMEM;
extern const prog_uint16_t lut_res_groove_push[] PROGMEM;
extern const prog_uint16_t lut_res_groove_lag[] PROGMEM;
extern const prog_uint16_t lut_res_groove_human[] PROGMEM;
extern const prog_uint16_t lut_res_groove_monkey[] PROGMEM;
extern const prog_uint8_t chr_res_special_characters[] PROGMEM;
extern const prog_uint8_t wav_res_lfo_waveforms[] PROGMEM;
#define STR_RES_WAVEFORM 0  // waveform
#define STR_RES_PARAMETER 1  // parameter
#define STR_RES_RANGE 2  // range
#define STR_RES_TUNE 3  // tune
#define STR_RES_OSC_MIX 4  // osc mix
#define STR_RES_SUB_OSC_ 5  // sub osc.
#define STR_RES_CROSSMOD_ 6  // crossmod.
#define STR_RES_OPERATOR 7  // operator
#define STR_RES_AMOUNT 8  // amount
#define STR_RES_RESONANCE 9  // resonance
#define STR_RES_MODE 10  // mode
#define STR_RES_ENV2TVCF 11  // env2~vcf
#define STR_RES_LFO2TVCF 12  // lfo2~vcf
#define STR_RES_VELOTVCF 13  // velo~vcf
#define STR_RES_KEYBTVCF 14  // keyb~vcf
#define STR_RES_ATTACK 15  // attack
#define STR_RES_DECAY 16  // decay
#define STR_RES_SUSTAIN 17  // sustain
#define STR_RES_RELEASE 18  // release
#define STR_RES_TRIGGER 19  // trigger
#define STR_RES_RATE 20  // rate
#define STR_RES_LFO_EG 21  // lfo/eg
#define STR_RES_VOICE_LFO 22  // voice lfo
#define STR_RES_SOURCE 23  // source
#define STR_RES_DESTINATION 24  // destination
#define STR_RES_IN1 25  // in1
#define STR_RES_IN2 26  // in2
#define STR_RES_MODULATION 27  // modulation
#define STR_RES_MODUL_ 28  // modul.
#define STR_RES__MODULATION 29  // modulation
#define STR_RES_MODIFIER 30  // modifier
#define STR_RES_MODIF_ 31  // modif.
#define STR_RES_VOLUME 32  // volume
#define STR_RES_OCTAVE 33  // octave
#define STR_RES_SPREAD 34  // spread
#define STR_RES_LEGATO 35  // legato
#define STR_RES_PORTAMENTO 36  // portamento
#define STR_RES_ARP_SEQ 37  // arp/seq
#define STR_RES_RAGA 38  // raga
#define STR_RES_DIRECTION 39  // direction
#define STR_RES_PATTERN 40  // pattern
#define STR_RES_CHANNEL 41  // channel
#define STR_RES_PART 42  // part
#define STR_RES_BPM 43  // bpm
#define STR_RES_LTCH 44  // ltch
#define STR_RES_LATCH 45  // latch
#define STR_RES_LOW 46  // low
#define STR_RES_HIGH 47  // high
#define STR_RES_GRID 48  // grid
#define STR_RES_SEQ1_LEN 49  // seq1 len
#define STR_RES_SEQ2_LEN 50  // seq2 len
#define STR_RES_PATT_LEN 51  // patt len
#define STR_RES_LEN1 52  // len1
#define STR_RES_LEN2 53  // len2
#define STR_RES_LENP 54  // lenp
#define STR_RES_GROOVE 55  // groove
#define STR_RES_MIDI 56  // midi
#define STR_RES_SNAP 57  // snap
#define STR_RES_HELP 58  // help
#define STR_RES_AUTO_BACKUP 59  // auto backup
#define STR_RES_LEDS 60  // leds
#define STR_RES_CARD_LEDS 61  // card leds
#define STR_RES_SWAP_COLORS 62  // swap colors
#define STR_RES_INPT_FILTER 63  // inpt filter
#define STR_RES_OUTP_MODE 64  // outp mode
#define STR_RES_EXT 65  // ext
#define STR_RES_OMNI 66  // omni
#define STR_RES_AMNT 67  // amnt
#define STR_RES_SRCE 68  // srce
#define STR_RES_OCT 69  // oct
#define STR_RES_SPRD 70  // sprd
#define STR_RES_A_SQ 71  // a/sq
#define STR_RES_OCTV 72  // octv
#define STR_RES_OFF 73  // off
#define STR_RES_ON 74  // on
#define STR_RES_NONE 75  // none
#define STR_RES_SAW 76  // saw
#define STR_RES_SQUARE 77  // square
#define STR_RES_TRIANGLE 78  // triangle
#define STR_RES_SINE 79  // sine
#define STR_RES_ZSAW 80  // zsaw
#define STR_RES_LPZSAW 81  // lpzsaw
#define STR_RES_PKZSAW 82  // pkzsaw
#define STR_RES_BPZSAW 83  // bpzsaw
#define STR_RES_HPZSAW 84  // hpzsaw
#define STR_RES_LPZPULSE 85  // lpzpulse
#define STR_RES_PKZPULSE 86  // pkzpulse
#define STR_RES_BPZPULSE 87  // bpzpulse
#define STR_RES_HPZPULSE 88  // hpzpulse
#define STR_RES_ZTRIANGLE 89  // ztriangle
#define STR_RES_PAD 90  // pad
#define STR_RES_FM 91  // fm
#define STR_RES_8BITS 92  // 8bits
#define STR_RES_PWM 93  // pwm
#define STR_RES_NOISE 94  // noise
#define STR_RES_VOWEL 95  // vowel
#define STR_RES_MALE 96  // male
#define STR_RES_FEMALE 97  // female
#define STR_RES_CHOIR 98  // choir
#define STR_RES_TAMPURA 99  // tampura
#define STR_RES_BOWED 100  // bowed
#define STR_RES_CELLO 101  // cello
#define STR_RES_VIBES 102  // vibes
#define STR_RES_SLAP 103  // slap
#define STR_RES_EPIANO 104  // epiano
#define STR_RES_ORGAN 105  // organ
#define STR_RES_WAVES 106  // waves
#define STR_RES_DIGITAL 107  // digital
#define STR_RES_DRONE_1 108  // drone 1
#define STR_RES_DRONE_2 109  // drone 2
#define STR_RES_METALLIC 110  // metallic
#define STR_RES_BELL 111  // bell
#define STR_RES_WAVQUENCE 112  // wavquence
#define STR_RES_OLDSAW 113  // oldsaw
#define STR_RES_QPWM 114  // qpwm
#define STR_RES_FMFB 115  // fmfb
#define STR_RES_CSAW 116  // csaw
#define STR_RES_VOWEL_2 117  // vowel 2
#define STR_RES_TRI 118  // tri
#define STR_RES_SQR 119  // sqr
#define STR_RES_S_H 120  // s&h
#define STR_RES_RAMP 121  // ramp
#define STR_RES_1EXP 122  // 1exp
#define STR_RES_1LIN 123  // 1lin
#define STR_RES_1TRI 124  // 1tri
#define STR_RES__SINE 125  // sine
#define STR_RES_HRM2 126  // hrm2
#define STR_RES_HRM3 127  // hrm3
#define STR_RES_HRM5 128  // hrm5
#define STR_RES_GRG1 129  // grg1
#define STR_RES_GRG2 130  // grg2
#define STR_RES_BAT1 131  // bat1
#define STR_RES_BAT2 132  // bat2
#define STR_RES_SPK1 133  // spk1
#define STR_RES_SPK2 134  // spk2
#define STR_RES_LSAW 135  // lsaw
#define STR_RES_LSQR 136  // lsqr
#define STR_RES_RSAW 137  // rsaw
#define STR_RES_RSQR 138  // rsqr
#define STR_RES_STP1 139  // stp1
#define STR_RES_STP2 140  // stp2
#define STR_RES___OFF 141  // off
#define STR_RES_SYNC 142  // sync
#define STR_RES_RINGMOD 143  // ringmod
#define STR_RES_XOR 144  // xor
#define STR_RES_FOLD 145  // fold
#define STR_RES_BITS 146  // bits
#define STR_RES_SQU1 147  // squ1
#define STR_RES_TRI1 148  // tri1
#define STR_RES_PUL1 149  // pul1
#define STR_RES_SQU2 150  // squ2
#define STR_RES_TRI2 151  // tri2
#define STR_RES_PUL2 152  // pul2
#define STR_RES_CLICK 153  // click
#define STR_RES_GLITCH 154  // glitch
#define STR_RES_BLOW 155  // blow
#define STR_RES_METAL 156  // metal
#define STR_RES_POP 157  // pop
#define STR_RES_ENV1 158  // env1
#define STR_RES_ENV2 159  // env2
#define STR_RES_ENV3 160  // env3
#define STR_RES_LFO_ 161  // lfo.
#define STR_RES_LFO5 162  // lfo5
#define STR_RES__LFO_ 163  // lfo.
#define STR_RES_LFO4 164  // lfo4
#define STR_RES_MOD1 165  // mod1
#define STR_RES_MOD2 166  // mod2
#define STR_RES_MOD3 167  // mod3
#define STR_RES_MOD4 168  // mod4
#define STR_RES_SEQ1 169  // seq1
#define STR_RES_SEQ2 170  // seq2
#define STR_RES_ARP 171  // arp
#define STR_RES_VELO 172  // velo
#define STR_RES_AFTR 173  // aftr
#define STR_RES_BEND 174  // bend
#define STR_RES_MWHL 175  // mwhl
#define STR_RES_WHL2 176  // whl2
#define STR_RES_PDAL 177  // pdal
#define STR_RES_NOTE 178  // note
#define STR_RES_GATE 179  // gate
#define STR_RES_NOIS 180  // nois
#define STR_RES_RAND 181  // rand
#define STR_RES_E256 182  // =256
#define STR_RES_E128 183  // =128
#define STR_RES_E64 184  // =64
#define STR_RES_E32 185  // =32
#define STR_RES_E16 186  // =16
#define STR_RES_E8 187  // =8
#define STR_RES_E4 188  // =4
#define STR_RES_PRM1 189  // prm1
#define STR_RES_PRM2 190  // prm2
#define STR_RES_OSC1 191  // osc1
#define STR_RES_OSC2 192  // osc2
#define STR_RES_31S2 193  // 1+2
#define STR_RES_VIBR 194  // vibr
#define STR_RES_MIX 195  // mix
#define STR_RES_XMOD 196  // xmod
#define STR_RES__NOIS 197  // nois
#define STR_RES_SUB 198  // sub
#define STR_RES_FUZZ 199  // fuzz
#define STR_RES_CRSH 200  // crsh
#define STR_RES_FREQ 201  // freq
#define STR_RES_RESO 202  // reso
#define STR_RES_ATTK 203  // attk
#define STR_RES_DECA 204  // deca
#define STR_RES_RELE 205  // rele
#define STR_RES__LFO4 206  // lfo4
#define STR_RES_VCA 207  // vca
#define STR_RES_ENV_1 208  // env 1
#define STR_RES_ENV_2 209  // env 2
#define STR_RES_ENV_3 210  // env 3
#define STR_RES_LFO_1 211  // lfo 1
#define STR_RES_LFO_2 212  // lfo 2
#define STR_RES_LFO_3 213  // lfo 3
#define STR_RES_LFO_4 214  // lfo 4
#define STR_RES_MOD__1 215  // mod. 1
#define STR_RES_MOD__2 216  // mod. 2
#define STR_RES_MOD__3 217  // mod. 3
#define STR_RES_MOD__4 218  // mod. 4
#define STR_RES_SEQ__1 219  // seq. 1
#define STR_RES_SEQ__2 220  // seq. 2
#define STR_RES__ARP 221  // arp
#define STR_RES__VELO 222  // velo
#define STR_RES_AFTTCH 223  // afttch
#define STR_RES_BENDER 224  // bender
#define STR_RES_MWHEEL 225  // mwheel
#define STR_RES_WHEEL2 226  // wheel2
#define STR_RES_PEDAL 227  // pedal
#define STR_RES__NOTE 228  // note
#define STR_RES__GATE 229  // gate
#define STR_RES__NOISE 230  // noise
#define STR_RES_RANDOM 231  // random
#define STR_RES_E_256 232  // = 256
#define STR_RES_E_32 233  // = 32
#define STR_RES_E_16 234  // = 16
#define STR_RES_E_8 235  // = 8
#define STR_RES_E_4 236  // = 4
#define STR_RES_PARAM_1 237  // param 1
#define STR_RES_PARAM_2 238  // param 2
#define STR_RES_OSC_1 239  // osc 1
#define STR_RES_OSC_2 240  // osc 2
#define STR_RES_OSC_1S2 241  // osc 1+2
#define STR_RES_VIBRATO 242  // vibrato
#define STR_RES__MIX 243  // mix
#define STR_RES__XMOD 244  // xmod
#define STR_RES___NOISE 245  // noise
#define STR_RES_SUBOSC 246  // subosc
#define STR_RES__FUZZ 247  // fuzz
#define STR_RES_CRUSH 248  // crush
#define STR_RES_FREQUENCY 249  // frequency
#define STR_RES__RESO 250  // reso
#define STR_RES__ATTACK 251  // attack
#define STR_RES__DECAY 252  // decay
#define STR_RES__RELEASE 253  // release
#define STR_RES__LFO_4 254  // lfo 4
#define STR_RES__VCA 255  // vca
#define STR_RES_LP 256  // lp
#define STR_RES_BP 257  // bp
#define STR_RES_HP 258  // hp
#define STR_RES_NT 259  // nt
#define STR_RES_FREE 260  // free
#define STR_RES_ENVTLFO 261  // env~lfo
#define STR_RES_LFOTENV 262  // lfo~env
#define STR_RES_STEP_SEQ 263  // step seq
#define STR_RES_ARPEGGIO 264  // arpeggio
#define STR_RES__PATTERN 265  // pattern
#define STR_RES__OFF 266  // off
#define STR_RES_ADD 267  // add
#define STR_RES_PROD 268  // prod
#define STR_RES_ATTN 269  // attn
#define STR_RES_MAX 270  // max
#define STR_RES_MIN 271  // min
#define STR_RES__XOR 272  // xor
#define STR_RES_GE 273  // >=
#define STR_RES_LE 274  // <=
#define STR_RES_QTZ 275  // qtz
#define STR_RES_LAG 276  // lag
#define STR_RES_MONO 277  // mono
#define STR_RES_POLY 278  // poly
#define STR_RES_2X_UNISON 279  // 2x unison
#define STR_RES_CYCLIC 280  // cyclic
#define STR_RES_CHAIN 281  // chain
#define STR_RES_UP 282  // up
#define STR_RES_DOWN 283  // down
#define STR_RES_UP_DOWN 284  // up&down
#define STR_RES_PLAYED 285  // played
#define STR_RES__RANDOM 286  // random
#define STR_RES_CHORD 287  // chord
#define STR_RES_1_96 288  // 1/96
#define STR_RES_1_48 289  // 1/48
#define STR_RES_1_32 290  // 1/32
#define STR_RES_1_24 291  // 1/24
#define STR_RES_1_16 292  // 1/16
#define STR_RES_1_12 293  // 1/12
#define STR_RES_1_8 294  // 1/8
#define STR_RES_1_6 295  // 1/6
#define STR_RES_1_4 296  // 1/4
#define STR_RES_1_3 297  // 1/3
#define STR_RES_3_8 298  // 3/8
#define STR_RES_1_2 299  // 1/2
#define STR_RES_2_3 300  // 2/3
#define STR_RES_3_4 301  // 3/4
#define STR_RES_1_1 302  // 1/1
#define STR_RES_THRU 303  // thru
#define STR_RES_SEQUENCER 304  // sequencer
#define STR_RES_CONTROLLR 305  // controllr
#define STR_RES__CHAIN 306  // chain
#define STR_RES_FULL 307  // full
#define STR_RES_____ 308  // ....
#define STR_RES____S 309  // ...s
#define STR_RES___P_ 310  // ..p.
#define STR_RES___PS 311  // ..ps
#define STR_RES__N__ 312  // .n..
#define STR_RES__N_S 313  // .n.s
#define STR_RES__NP_ 314  // .np.
#define STR_RES__NPS 315  // .nps
#define STR_RES_C___ 316  // c...
#define STR_RES_C__S 317  // c..s
#define STR_RES_C_P_ 318  // c.p.
#define STR_RES_C_PS 319  // c.ps
#define STR_RES_CN__ 320  // cn..
#define STR_RES_CN_S 321  // cn.s
#define STR_RES_CNP_ 322  // cnp.
#define STR_RES_CNPS 323  // cnps
#define STR_RES_SWING 324  // swing
#define STR_RES_SHUFFLE 325  // shuffle
#define STR_RES_PUSH 326  // push
#define STR_RES__LAG 327  // lag
#define STR_RES_HUMAN 328  // human
#define STR_RES_MONKEY 329  // monkey
#define STR_RES_OSCILLATOR_1 330  // oscillator 1
#define STR_RES_OSCILLATOR_2 331  // oscillator 2
#define STR_RES_MIXER 332  // mixer
#define STR_RES_LFO 333  // lfo
#define STR_RES_FILTER_1 334  // filter 1
#define STR_RES_FILTER_2 335  // filter 2
#define STR_RES_ENVELOPE 336  // envelope
#define STR_RES_ARPEGGIATOR 337  // arpeggiator
#define STR_RES_MULTI 338  // multi
#define STR_RES_CLOCK 339  // clock
#define STR_RES_PERFORMANCE 340  // performance
#define STR_RES_SYSTEM 341  // system
#define STR_RES_PT_X_PATCH 342  // pt X patch
#define STR_RES_PT_X_SEQUENCE 343  // pt X sequence
#define STR_RES_PT_X_PROGRAM 344  // pt X program
#define STR_RES_RANDOMIZE 345  // randomize
#define STR_RES_INIT 346  // init
#define STR_RES_PATCH 347  // PATCH
#define STR_RES_SEQUENCE 348  // SEQUENCE
#define STR_RES_PROGRAM 349  // PROGRAM
#define STR_RES__MULTI 350  // MULTI
#define STR_RES____ 351  // ___
#define STR_RES_EQUAL 352  // equal
#define STR_RES_EG 353  // eg
#define STR_RES_DEPT 354  // dept
#define STR_RES_DEPTH 355  // depth
#define STR_RES_AMP 356  // amp
#define STR_RES_FLT 357  // flt
#define STR_RES_PCH 358  // pch
#define STR_RES_RISE 359  // rise
#define STR_RES_FALL 360  // fall
#define STR_RES_CURV 361  // curv
#define STR_RES_DEST 362  // dest
#define STR_RES_SHAP 363  // shap
#define STR_RES_JUST 364  // just
#define STR_RES_PYTHAGOREAN 365  // pythagorean
#define STR_RES_1_4_EB 366  // 1/4 eb
#define STR_RES_1_4_E 367  // 1/4 e
#define STR_RES_1_4_EA 368  // 1/4 ea
#define STR_RES_BHAIRAV 369  // bhairav
#define STR_RES_GUNAKRI 370  // gunakri
#define STR_RES_MARWA 371  // marwa
#define STR_RES_SHREE 372  // shree
#define STR_RES_PURVI 373  // purvi
#define STR_RES_BILAWAL 374  // bilawal
#define STR_RES_YAMAN 375  // yaman
#define STR_RES_KAFI 376  // kafi
#define STR_RES_BHIMPALASREE 377  // bhimpalasree
#define STR_RES_DARBARI 378  // darbari
#define STR_RES_BAGESHREE 379  // bageshree
#define STR_RES_RAGESHREE 380  // rageshree
#define STR_RES_KHAMAJ 381  // khamaj
#define STR_RES_MIMAL 382  // mi'mal
#define STR_RES_PARAMESHWARI 383  // parameshwari
#define STR_RES_RANGESHWARI 384  // rangeshwari
#define STR_RES_GANGESHWARI 385  // gangeshwari
#define STR_RES_KAMESHWARI 386  // kameshwari
#define STR_RES_PA__KAFI 387  // pa. kafi
#define STR_RES_NATBHAIRAV 388  // natbhairav
#define STR_RES_M_KAUNS 389  // m.kauns
#define STR_RES_BAIRAGI 390  // bairagi
#define STR_RES_B_TODI 391  // b.todi
#define STR_RES_CHANDRADEEP 392  // chandradeep
#define STR_RES_KAUSHIK_TODI 393  // kaushik todi
#define STR_RES_JOGESHWARI 394  // jogeshwari
#define STR_RES_RASIA 395  // rasia
#define LUT_RES_LFO_INCREMENTS 0
#define LUT_RES_LFO_INCREMENTS_SIZE 128
#define LUT_RES_SCALE_JUST 1
#define LUT_RES_SCALE_JUST_SIZE 12
#define LUT_RES_SCALE_PYTHAGOREAN 2
#define LUT_RES_SCALE_PYTHAGOREAN_SIZE 12
#define LUT_RES_SCALE_1_4_EB 3
#define LUT_RES_SCALE_1_4_EB_SIZE 12
#define LUT_RES_SCALE_1_4_E 4
#define LUT_RES_SCALE_1_4_E_SIZE 12
#define LUT_RES_SCALE_1_4_EA 5
#define LUT_RES_SCALE_1_4_EA_SIZE 12
#define LUT_RES_SCALE_BHAIRAV 6
#define LUT_RES_SCALE_BHAIRAV_SIZE 12
#define LUT_RES_SCALE_GUNAKRI 7
#define LUT_RES_SCALE_GUNAKRI_SIZE 12
#define LUT_RES_SCALE_MARWA 8
#define LUT_RES_SCALE_MARWA_SIZE 12
#define LUT_RES_SCALE_SHREE 9
#define LUT_RES_SCALE_SHREE_SIZE 12
#define LUT_RES_SCALE_PURVI 10
#define LUT_RES_SCALE_PURVI_SIZE 12
#define LUT_RES_SCALE_BILAWAL 11
#define LUT_RES_SCALE_BILAWAL_SIZE 12
#define LUT_RES_SCALE_YAMAN 12
#define LUT_RES_SCALE_YAMAN_SIZE 12
#define LUT_RES_SCALE_KAFI 13
#define LUT_RES_SCALE_KAFI_SIZE 12
#define LUT_RES_SCALE_BHIMPALASREE 14
#define LUT_RES_SCALE_BHIMPALASREE_SIZE 12
#define LUT_RES_SCALE_DARBARI 15
#define LUT_RES_SCALE_DARBARI_SIZE 12
#define LUT_RES_SCALE_BAGESHREE 16
#define LUT_RES_SCALE_BAGESHREE_SIZE 12
#define LUT_RES_SCALE_RAGESHREE 17
#define LUT_RES_SCALE_RAGESHREE_SIZE 12
#define LUT_RES_SCALE_KHAMAJ 18
#define LUT_RES_SCALE_KHAMAJ_SIZE 12
#define LUT_RES_SCALE_MIMAL 19
#define LUT_RES_SCALE_MIMAL_SIZE 12
#define LUT_RES_SCALE_PARAMESHWARI 20
#define LUT_RES_SCALE_PARAMESHWARI_SIZE 12
#define LUT_RES_SCALE_RANGESHWARI 21
#define LUT_RES_SCALE_RANGESHWARI_SIZE 12
#define LUT_RES_SCALE_GANGESHWARI 22
#define LUT_RES_SCALE_GANGESHWARI_SIZE 12
#define LUT_RES_SCALE_KAMESHWARI 23
#define LUT_RES_SCALE_KAMESHWARI_SIZE 12
#define LUT_RES_SCALE_PA__KAFI 24
#define LUT_RES_SCALE_PA__KAFI_SIZE 12
#define LUT_RES_SCALE_NATBHAIRAV 25
#define LUT_RES_SCALE_NATBHAIRAV_SIZE 12
#define LUT_RES_SCALE_M_KAUNS 26
#define LUT_RES_SCALE_M_KAUNS_SIZE 12
#define LUT_RES_SCALE_BAIRAGI 27
#define LUT_RES_SCALE_BAIRAGI_SIZE 12
#define LUT_RES_SCALE_B_TODI 28
#define LUT_RES_SCALE_B_TODI_SIZE 12
#define LUT_RES_SCALE_CHANDRADEEP 29
#define LUT_RES_SCALE_CHANDRADEEP_SIZE 12
#define LUT_RES_SCALE_KAUSHIK_TODI 30
#define LUT_RES_SCALE_KAUSHIK_TODI_SIZE 12
#define LUT_RES_SCALE_JOGESHWARI 31
#define LUT_RES_SCALE_JOGESHWARI_SIZE 12
#define LUT_RES_SCALE_RASIA 32
#define LUT_RES_SCALE_RASIA_SIZE 12
#define LUT_RES_ARPEGGIATOR_PATTERNS 33
#define LUT_RES_ARPEGGIATOR_PATTERNS_SIZE 22
#define LUT_RES_GROOVE_SWING 34
#define LUT_RES_GROOVE_SWING_SIZE 16
#define LUT_RES_GROOVE_SHUFFLE 35
#define LUT_RES_GROOVE_SHUFFLE_SIZE 16
#define LUT_RES_GROOVE_PUSH 36
#define LUT_RES_GROOVE_PUSH_SIZE 16
#define LUT_RES_GROOVE_LAG 37
#define LUT_RES_GROOVE_LAG_SIZE 16
#define LUT_RES_GROOVE_HUMAN 38
#define LUT_RES_GROOVE_HUMAN_SIZE 16
#define LUT_RES_GROOVE_MONKEY 39
#define LUT_RES_GROOVE_MONKEY_SIZE 16
#define CHR_RES_SPECIAL_CHARACTERS 0
#define CHR_RES_SPECIAL_CHARACTERS_SIZE 56
#define WAV_RES_LFO_WAVEFORMS 0
#define WAV_RES_LFO_WAVEFORMS_SIZE 2064
typedef avrlib::ResourcesManager<
    ResourceId,
    avrlib::ResourcesTables<
        string_table,
        lookup_table_table> > ResourcesManager; 

}  // namespace ambika

#endif  // CONTROLLER_RESOURCES_H_
