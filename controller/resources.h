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
#define STR_RES_WAVEFORM 0  // waveform
#define STR_RES_PARAMETER 1  // parameter
#define STR_RES_RANGE 2  // range
#define STR_RES_TUNE 3  // tune
#define STR_RES_OSC_MIX 4  // osc mix
#define STR_RES_SUB_OSC_ 5  // sub osc.
#define STR_RES_CROSSMOD_ 6  // crossmod.
#define STR_RES_AMOUNT 7  // amount
#define STR_RES_RESONANCE 8  // resonance
#define STR_RES_MODE 9  // mode
#define STR_RES_ENV2TVCF 10  // env2~vcf
#define STR_RES_LFO2TVCF 11  // lfo2~vcf
#define STR_RES_VELOTVCF 12  // velo~vcf
#define STR_RES_KEYBTVCF 13  // keyb~vcf
#define STR_RES_ATTACK 14  // attack
#define STR_RES_DECAY 15  // decay
#define STR_RES_SUSTAIN 16  // sustain
#define STR_RES_RELEASE 17  // release
#define STR_RES_TRIGGER 18  // trigger
#define STR_RES_RATE 19  // rate
#define STR_RES_LFO_EG 20  // lfo/eg
#define STR_RES_VOICE_LFO 21  // voice lfo
#define STR_RES_SOURCE 22  // source
#define STR_RES_DESTINATION 23  // destination
#define STR_RES_VOLUME 24  // volume
#define STR_RES_OCTAVE 25  // octave
#define STR_RES_SPREAD 26  // spread
#define STR_RES_LEGATO 27  // legato
#define STR_RES_PORTAMENTO 28  // portamento
#define STR_RES_ARP_SEQ 29  // arp/seq
#define STR_RES_RAGA 30  // raga
#define STR_RES_DIRECTION 31  // direction
#define STR_RES_PATTERN 32  // pattern
#define STR_RES_CHANNEL 33  // channel
#define STR_RES_PART 34  // part
#define STR_RES_BPM 35  // bpm
#define STR_RES_LTCH 36  // ltch
#define STR_RES_LATCH 37  // latch
#define STR_RES_LOW 38  // low
#define STR_RES_HIGH 39  // high
#define STR_RES_GRID 40  // grid
#define STR_RES_SEQ1_LEN 41  // seq1 len
#define STR_RES_SEQ2_LEN 42  // seq2 len
#define STR_RES_PATT_LEN 43  // patt len
#define STR_RES_LEN1 44  // len1
#define STR_RES_LEN2 45  // len2
#define STR_RES_LENP 46  // lenp
#define STR_RES_GROOVE 47  // groove
#define STR_RES_MIDI 48  // midi
#define STR_RES_SNAP 49  // snap
#define STR_RES_HELP 50  // help
#define STR_RES_AUTO_BACKUP 51  // auto backup
#define STR_RES_LEDS 52  // leds
#define STR_RES_CARD_LEDS 53  // card leds
#define STR_RES_SWAP_COLORS 54  // swap colors
#define STR_RES_INPT_FILTER 55  // inpt filter
#define STR_RES_OUTP_MODE 56  // outp mode
#define STR_RES_EXT 57  // ext
#define STR_RES_OMNI 58  // omni
#define STR_RES_AMNT 59  // amnt
#define STR_RES_OCT 60  // oct
#define STR_RES_SPRD 61  // sprd
#define STR_RES_A_SQ 62  // a/sq
#define STR_RES_OCTV 63  // octv
#define STR_RES_OFF 64  // off
#define STR_RES_ON 65  // on
#define STR_RES_NONE 66  // none
#define STR_RES_SAW 67  // saw
#define STR_RES_SQUARE 68  // square
#define STR_RES_TRIANGLE 69  // triangle
#define STR_RES_SINE 70  // sine
#define STR_RES_ZSAW 71  // zsaw
#define STR_RES_SN16 72  // sn16
#define STR_RES_PAD 73  // pad
#define STR_RES_FM 74  // fm
#define STR_RES_8BITS 75  // 8bits
#define STR_RES_PWM 76  // pwm
#define STR_RES_NOISE 77  // noise
#define STR_RES_VOWEL 78  // vowel
#define STR_RES_MALE 79  // male
#define STR_RES_FEMALE 80  // female
#define STR_RES_CHOIR 81  // choir
#define STR_RES_TAMPURA 82  // tampura
#define STR_RES_BOWED 83  // bowed
#define STR_RES_CELLO 84  // cello
#define STR_RES_VIBES 85  // vibes
#define STR_RES_SLAP 86  // slap
#define STR_RES_EPIANO 87  // epiano
#define STR_RES_ORGAN 88  // organ
#define STR_RES_WAVES 89  // waves
#define STR_RES_DIGITAL 90  // digital
#define STR_RES_DRONE_1 91  // drone 1
#define STR_RES_DRONE_2 92  // drone 2
#define STR_RES_METALLIC 93  // metallic
#define STR_RES_BELL 94  // bell
#define STR_RES_WAVQUENCE 95  // wavquence
#define STR_RES_OLDSAW 96  // oldsaw
#define STR_RES_QPWM 97  // qpwm
#define STR_RES_FMFB 98  // fmfb
#define STR_RES_CSAW 99  // csaw
#define STR_RES_VOWEL_2 100  // vowel 2
#define STR_RES_TRI 101  // tri
#define STR_RES_SQR 102  // sqr
#define STR_RES_S_H 103  // s&h
#define STR_RES_RAMP 104  // ramp
#define STR_RES_1EXP 105  // 1exp
#define STR_RES_1LIN 106  // 1lin
#define STR_RES_1TRI 107  // 1tri
#define STR_RES__SINE 108  // sine
#define STR_RES_HRM2 109  // hrm2
#define STR_RES_HRM3 110  // hrm3
#define STR_RES_HRM5 111  // hrm5
#define STR_RES_GRG1 112  // grg1
#define STR_RES_GRG2 113  // grg2
#define STR_RES_BAT1 114  // bat1
#define STR_RES_BAT2 115  // bat2
#define STR_RES_SPK1 116  // spk1
#define STR_RES_SPK2 117  // spk2
#define STR_RES_LSAW 118  // lsaw
#define STR_RES_LSQR 119  // lsqr
#define STR_RES_RSAW 120  // rsaw
#define STR_RES_RSQR 121  // rsqr
#define STR_RES_STP1 122  // stp1
#define STR_RES_STP2 123  // stp2
#define STR_RES___OFF 124  // off
#define STR_RES_SYNC 125  // sync
#define STR_RES_RINGMOD 126  // ringmod
#define STR_RES_XOR 127  // xor
#define STR_RES_FOLD 128  // fold
#define STR_RES_BITS 129  // bits
#define STR_RES_SQU1 130  // squ1
#define STR_RES_TRI1 131  // tri1
#define STR_RES_PUL1 132  // pul1
#define STR_RES_SQU2 133  // squ2
#define STR_RES_TRI2 134  // tri2
#define STR_RES_PUL2 135  // pul2
#define STR_RES_CLICK 136  // click
#define STR_RES_GLITCH 137  // glitch
#define STR_RES_BLOW 138  // blow
#define STR_RES_METAL 139  // metal
#define STR_RES_POP 140  // pop
#define STR_RES_ENV1 141  // env1
#define STR_RES_ENV2 142  // env2
#define STR_RES_ENV3 143  // env3
#define STR_RES_LFO_ 144  // lfo.
#define STR_RES_LFO5 145  // lfo5
#define STR_RES__LFO_ 146  // lfo.
#define STR_RES_LFO4 147  // lfo4
#define STR_RES_MOD1 148  // mod1
#define STR_RES_MOD2 149  // mod2
#define STR_RES_MOD3 150  // mod3
#define STR_RES_MOD4 151  // mod4
#define STR_RES_SEQ1 152  // seq1
#define STR_RES_SEQ2 153  // seq2
#define STR_RES_ARP 154  // arp
#define STR_RES_VELO 155  // velo
#define STR_RES_AFTR 156  // aftr
#define STR_RES_BEND 157  // bend
#define STR_RES_MWHL 158  // mwhl
#define STR_RES_WHL2 159  // whl2
#define STR_RES_PDAL 160  // pdal
#define STR_RES_NOTE 161  // note
#define STR_RES_GATE 162  // gate
#define STR_RES_NOIS 163  // nois
#define STR_RES_RAND 164  // rand
#define STR_RES_E256 165  // =256
#define STR_RES_E128 166  // =128
#define STR_RES_E64 167  // =64
#define STR_RES_E32 168  // =32
#define STR_RES_E16 169  // =16
#define STR_RES_E8 170  // =8
#define STR_RES_E4 171  // =4
#define STR_RES_PRM1 172  // prm1
#define STR_RES_PRM2 173  // prm2
#define STR_RES_OSC1 174  // osc1
#define STR_RES_OSC2 175  // osc2
#define STR_RES_31S2 176  // 1+2
#define STR_RES_VIBR 177  // vibr
#define STR_RES_MIX 178  // mix
#define STR_RES_XMOD 179  // xmod
#define STR_RES__NOIS 180  // nois
#define STR_RES_SUB 181  // sub
#define STR_RES_FUZZ 182  // fuzz
#define STR_RES_CRSH 183  // crsh
#define STR_RES_FREQ 184  // freq
#define STR_RES_RESO 185  // reso
#define STR_RES_ATTK 186  // attk
#define STR_RES_DECA 187  // deca
#define STR_RES_RELE 188  // rele
#define STR_RES__LFO4 189  // lfo4
#define STR_RES_VCA 190  // vca
#define STR_RES__FOLD 191  // fold
#define STR_RES_ENV_1 192  // env 1
#define STR_RES_ENV_2 193  // env 2
#define STR_RES_ENV_3 194  // env 3
#define STR_RES_LFO__ 195  // lfo .
#define STR_RES_LFO_2 196  // lfo 2
#define STR_RES__LFO__ 197  // lfo .
#define STR_RES_LFO_4 198  // lfo 4
#define STR_RES_MOD__1 199  // mod. 1
#define STR_RES_MOD__2 200  // mod. 2
#define STR_RES_MOD__3 201  // mod. 3
#define STR_RES_MOD__4 202  // mod. 4
#define STR_RES_SEQ__1 203  // seq. 1
#define STR_RES_SEQ__2 204  // seq. 2
#define STR_RES__ARP 205  // arp
#define STR_RES__VELO 206  // velo
#define STR_RES_AFTTCH 207  // afttch
#define STR_RES_BENDER 208  // bender
#define STR_RES_MWHEEL 209  // mwheel
#define STR_RES_WHEEL2 210  // wheel2
#define STR_RES_PEDAL 211  // pedal
#define STR_RES__NOTE 212  // note
#define STR_RES__GATE 213  // gate
#define STR_RES__NOISE 214  // noise
#define STR_RES_RANDOM 215  // random
#define STR_RES_E_256 216  // = 256
#define STR_RES_E_32 217  // = 32
#define STR_RES_E_16 218  // = 16
#define STR_RES_E_8 219  // = 8
#define STR_RES_E_4 220  // = 4
#define STR_RES_PARAM_1 221  // param 1
#define STR_RES_PARAM_2 222  // param 2
#define STR_RES_OSC_1 223  // osc 1
#define STR_RES_OSC_2 224  // osc 2
#define STR_RES_OSC_1S2 225  // osc 1+2
#define STR_RES_VIBRATO 226  // vibrato
#define STR_RES__MIX 227  // mix
#define STR_RES__XMOD 228  // xmod
#define STR_RES___NOISE 229  // noise
#define STR_RES_SUBOSC 230  // subosc
#define STR_RES__FUZZ 231  // fuzz
#define STR_RES_CRUSH 232  // crush
#define STR_RES_FREQUENCY 233  // frequency
#define STR_RES__RESO 234  // reso
#define STR_RES__ATTACK 235  // attack
#define STR_RES__DECAY 236  // decay
#define STR_RES__RELEASE 237  // release
#define STR_RES__LFO_4 238  // lfo 4
#define STR_RES__VCA 239  // vca
#define STR_RES___FOLD 240  // fold
#define STR_RES_LP 241  // lp
#define STR_RES_BP 242  // bp
#define STR_RES_HP 243  // hp
#define STR_RES_NT 244  // nt
#define STR_RES_FREE 245  // free
#define STR_RES_ENVTLFO 246  // env~lfo
#define STR_RES_LFOTENV 247  // lfo~env
#define STR_RES_STEP_SEQ 248  // step seq
#define STR_RES_ARPEGGIO 249  // arpeggio
#define STR_RES__PATTERN 250  // pattern
#define STR_RES__OFF 251  // off
#define STR_RES_ADD 252  // add
#define STR_RES_PROD 253  // prod
#define STR_RES_ATTN 254  // attn
#define STR_RES_MAX 255  // max
#define STR_RES_MIN 256  // min
#define STR_RES__XOR 257  // xor
#define STR_RES_GE 258  // >=
#define STR_RES_LE 259  // <=
#define STR_RES_QTZ 260  // qtz
#define STR_RES_LAG 261  // lag
#define STR_RES_MONO 262  // mono
#define STR_RES_POLY 263  // poly
#define STR_RES_2X_UNISON 264  // 2x unison
#define STR_RES_CYCLIC 265  // cyclic
#define STR_RES_CHAIN 266  // chain
#define STR_RES_UP 267  // up
#define STR_RES_DOWN 268  // down
#define STR_RES_UP_DOWN 269  // up&down
#define STR_RES_PLAYED 270  // played
#define STR_RES__RANDOM 271  // random
#define STR_RES_CHORD 272  // chord
#define STR_RES_1_96 273  // 1/96
#define STR_RES_1_48 274  // 1/48
#define STR_RES_1_32 275  // 1/32
#define STR_RES_1_24 276  // 1/24
#define STR_RES_1_16 277  // 1/16
#define STR_RES_1_12 278  // 1/12
#define STR_RES_1_8 279  // 1/8
#define STR_RES_1_6 280  // 1/6
#define STR_RES_1_4 281  // 1/4
#define STR_RES_1_3 282  // 1/3
#define STR_RES_3_8 283  // 3/8
#define STR_RES_1_2 284  // 1/2
#define STR_RES_2_3 285  // 2/3
#define STR_RES_3_4 286  // 3/4
#define STR_RES_1_1 287  // 1/1
#define STR_RES_THRU 288  // thru
#define STR_RES_SEQUENCER 289  // sequencer
#define STR_RES_CONTROLLR 290  // controllr
#define STR_RES__CHAIN 291  // chain
#define STR_RES_FULL 292  // full
#define STR_RES_____ 293  // ....
#define STR_RES____S 294  // ...s
#define STR_RES___P_ 295  // ..p.
#define STR_RES___PS 296  // ..ps
#define STR_RES__N__ 297  // .n..
#define STR_RES__N_S 298  // .n.s
#define STR_RES__NP_ 299  // .np.
#define STR_RES__NPS 300  // .nps
#define STR_RES_C___ 301  // c...
#define STR_RES_C__S 302  // c..s
#define STR_RES_C_P_ 303  // c.p.
#define STR_RES_C_PS 304  // c.ps
#define STR_RES_CN__ 305  // cn..
#define STR_RES_CN_S 306  // cn.s
#define STR_RES_CNP_ 307  // cnp.
#define STR_RES_CNPS 308  // cnps
#define STR_RES_SWING 309  // swing
#define STR_RES_SHUFFLE 310  // shuffle
#define STR_RES_PUSH 311  // push
#define STR_RES__LAG 312  // lag
#define STR_RES_HUMAN 313  // human
#define STR_RES_MONKEY 314  // monkey
#define STR_RES_OSCILLATOR_1 315  // oscillator 1
#define STR_RES_OSCILLATOR_2 316  // oscillator 2
#define STR_RES_MIXER 317  // mixer
#define STR_RES_LFO 318  // lfo
#define STR_RES_FILTER_1 319  // filter 1
#define STR_RES_FILTER_2 320  // filter 2
#define STR_RES_ENVELOPE 321  // envelope
#define STR_RES_ARPEGGIATOR 322  // arpeggiator
#define STR_RES_MULTI 323  // multi
#define STR_RES_CLOCK 324  // clock
#define STR_RES_PERFORMANCE 325  // performance
#define STR_RES_SYSTEM 326  // system
#define STR_RES_PT_X_PATCH 327  // pt X patch
#define STR_RES_PT_X_SEQUENCE 328  // pt X sequence
#define STR_RES_PT_X_PROGRAM 329  // pt X program
#define STR_RES_RANDOMIZE 330  // randomize
#define STR_RES_INIT 331  // init
#define STR_RES_PATCH 332  // PATCH
#define STR_RES_SEQUENCE 333  // SEQUENCE
#define STR_RES_PROGRAM 334  // PROGRAM
#define STR_RES__MULTI 335  // MULTI
#define STR_RES____ 336  // ___
#define STR_RES_EQUAL 337  // equal
#define STR_RES_EG 338  // eg
#define STR_RES_DEPT 339  // dept
#define STR_RES_DEPTH 340  // depth
#define STR_RES_AMP 341  // amp
#define STR_RES_FLT 342  // flt
#define STR_RES_PCH 343  // pch
#define STR_RES_RISE 344  // rise
#define STR_RES_FALL 345  // fall
#define STR_RES_CURV 346  // curv
#define STR_RES_DEST 347  // dest
#define STR_RES_SHAP 348  // shap
#define STR_RES_JUST 349  // just
#define STR_RES_PYTHAGOREAN 350  // pythagorean
#define STR_RES_1_4_EB 351  // 1/4 eb
#define STR_RES_1_4_E 352  // 1/4 e
#define STR_RES_1_4_EA 353  // 1/4 ea
#define STR_RES_BHAIRAV 354  // bhairav
#define STR_RES_GUNAKRI 355  // gunakri
#define STR_RES_MARWA 356  // marwa
#define STR_RES_SHREE 357  // shree
#define STR_RES_PURVI 358  // purvi
#define STR_RES_BILAWAL 359  // bilawal
#define STR_RES_YAMAN 360  // yaman
#define STR_RES_KAFI 361  // kafi
#define STR_RES_BHIMPALASREE 362  // bhimpalasree
#define STR_RES_DARBARI 363  // darbari
#define STR_RES_BAGESHREE 364  // bageshree
#define STR_RES_RAGESHREE 365  // rageshree
#define STR_RES_KHAMAJ 366  // khamaj
#define STR_RES_MIMAL 367  // mi'mal
#define STR_RES_PARAMESHWARI 368  // parameshwari
#define STR_RES_RANGESHWARI 369  // rangeshwari
#define STR_RES_GANGESHWARI 370  // gangeshwari
#define STR_RES_KAMESHWARI 371  // kameshwari
#define STR_RES_PA__KAFI 372  // pa. kafi
#define STR_RES_NATBHAIRAV 373  // natbhairav
#define STR_RES_M_KAUNS 374  // m.kauns
#define STR_RES_BAIRAGI 375  // bairagi
#define STR_RES_B_TODI 376  // b.todi
#define STR_RES_CHANDRADEEP 377  // chandradeep
#define STR_RES_KAUSHIK_TODI 378  // kaushik todi
#define STR_RES_JOGESHWARI 379  // jogeshwari
#define STR_RES_RASIA 380  // rasia
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
typedef avrlib::ResourcesManager<
    ResourceId,
    avrlib::ResourcesTables<
        string_table,
        lookup_table_table> > ResourcesManager; 

}  // namespace ambika

#endif  // CONTROLLER_RESOURCES_H_
