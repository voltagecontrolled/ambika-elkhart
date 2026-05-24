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
#define STR_RES_SN16 81  // sn16
#define STR_RES_PAD 82  // pad
#define STR_RES_FM 83  // fm
#define STR_RES_8BITS 84  // 8bits
#define STR_RES_PWM 85  // pwm
#define STR_RES_NOISE 86  // noise
#define STR_RES_VOWEL 87  // vowel
#define STR_RES_MALE 88  // male
#define STR_RES_FEMALE 89  // female
#define STR_RES_CHOIR 90  // choir
#define STR_RES_TAMPURA 91  // tampura
#define STR_RES_BOWED 92  // bowed
#define STR_RES_CELLO 93  // cello
#define STR_RES_VIBES 94  // vibes
#define STR_RES_SLAP 95  // slap
#define STR_RES_EPIANO 96  // epiano
#define STR_RES_ORGAN 97  // organ
#define STR_RES_WAVES 98  // waves
#define STR_RES_DIGITAL 99  // digital
#define STR_RES_DRONE_1 100  // drone 1
#define STR_RES_DRONE_2 101  // drone 2
#define STR_RES_METALLIC 102  // metallic
#define STR_RES_BELL 103  // bell
#define STR_RES_WAVQUENCE 104  // wavquence
#define STR_RES_OLDSAW 105  // oldsaw
#define STR_RES_QPWM 106  // qpwm
#define STR_RES_FMFB 107  // fmfb
#define STR_RES_CSAW 108  // csaw
#define STR_RES_VOWEL_2 109  // vowel 2
#define STR_RES_TRI 110  // tri
#define STR_RES_SQR 111  // sqr
#define STR_RES_S_H 112  // s&h
#define STR_RES_RAMP 113  // ramp
#define STR_RES_1EXP 114  // 1exp
#define STR_RES_1LIN 115  // 1lin
#define STR_RES_1TRI 116  // 1tri
#define STR_RES__SINE 117  // sine
#define STR_RES_HRM2 118  // hrm2
#define STR_RES_HRM3 119  // hrm3
#define STR_RES_HRM5 120  // hrm5
#define STR_RES_GRG1 121  // grg1
#define STR_RES_GRG2 122  // grg2
#define STR_RES_BAT1 123  // bat1
#define STR_RES_BAT2 124  // bat2
#define STR_RES_SPK1 125  // spk1
#define STR_RES_SPK2 126  // spk2
#define STR_RES_LSAW 127  // lsaw
#define STR_RES_LSQR 128  // lsqr
#define STR_RES_RSAW 129  // rsaw
#define STR_RES_RSQR 130  // rsqr
#define STR_RES_STP1 131  // stp1
#define STR_RES_STP2 132  // stp2
#define STR_RES___OFF 133  // off
#define STR_RES_SYNC 134  // sync
#define STR_RES_RINGMOD 135  // ringmod
#define STR_RES_XOR 136  // xor
#define STR_RES_FOLD 137  // fold
#define STR_RES_BITS 138  // bits
#define STR_RES_SQU1 139  // squ1
#define STR_RES_TRI1 140  // tri1
#define STR_RES_PUL1 141  // pul1
#define STR_RES_SQU2 142  // squ2
#define STR_RES_TRI2 143  // tri2
#define STR_RES_PUL2 144  // pul2
#define STR_RES_CLICK 145  // click
#define STR_RES_GLITCH 146  // glitch
#define STR_RES_BLOW 147  // blow
#define STR_RES_METAL 148  // metal
#define STR_RES_POP 149  // pop
#define STR_RES_ENV1 150  // env1
#define STR_RES_ENV2 151  // env2
#define STR_RES_ENV3 152  // env3
#define STR_RES_LFO_ 153  // lfo.
#define STR_RES_LFO5 154  // lfo5
#define STR_RES__LFO_ 155  // lfo.
#define STR_RES_LFO4 156  // lfo4
#define STR_RES_MOD1 157  // mod1
#define STR_RES_MOD2 158  // mod2
#define STR_RES_MOD3 159  // mod3
#define STR_RES_MOD4 160  // mod4
#define STR_RES_SEQ1 161  // seq1
#define STR_RES_SEQ2 162  // seq2
#define STR_RES_ARP 163  // arp
#define STR_RES_VELO 164  // velo
#define STR_RES_AFTR 165  // aftr
#define STR_RES_BEND 166  // bend
#define STR_RES_MWHL 167  // mwhl
#define STR_RES_WHL2 168  // whl2
#define STR_RES_PDAL 169  // pdal
#define STR_RES_NOTE 170  // note
#define STR_RES_GATE 171  // gate
#define STR_RES_NOIS 172  // nois
#define STR_RES_RAND 173  // rand
#define STR_RES_E256 174  // =256
#define STR_RES_E128 175  // =128
#define STR_RES_E64 176  // =64
#define STR_RES_E32 177  // =32
#define STR_RES_E16 178  // =16
#define STR_RES_E8 179  // =8
#define STR_RES_E4 180  // =4
#define STR_RES_PRM1 181  // prm1
#define STR_RES_PRM2 182  // prm2
#define STR_RES_OSC1 183  // osc1
#define STR_RES_OSC2 184  // osc2
#define STR_RES_31S2 185  // 1+2
#define STR_RES_VIBR 186  // vibr
#define STR_RES_MIX 187  // mix
#define STR_RES_XMOD 188  // xmod
#define STR_RES__NOIS 189  // nois
#define STR_RES_SUB 190  // sub
#define STR_RES_FUZZ 191  // fuzz
#define STR_RES_CRSH 192  // crsh
#define STR_RES_FREQ 193  // freq
#define STR_RES_RESO 194  // reso
#define STR_RES_ATTK 195  // attk
#define STR_RES_DECA 196  // deca
#define STR_RES_RELE 197  // rele
#define STR_RES__LFO4 198  // lfo4
#define STR_RES_VCA 199  // vca
#define STR_RES__FOLD 200  // fold
#define STR_RES_ENV_1 201  // env 1
#define STR_RES_ENV_2 202  // env 2
#define STR_RES_ENV_3 203  // env 3
#define STR_RES_LFO_1 204  // lfo 1
#define STR_RES_LFO_2 205  // lfo 2
#define STR_RES_LFO_3 206  // lfo 3
#define STR_RES_LFO_4 207  // lfo 4
#define STR_RES_MOD__1 208  // mod. 1
#define STR_RES_MOD__2 209  // mod. 2
#define STR_RES_MOD__3 210  // mod. 3
#define STR_RES_MOD__4 211  // mod. 4
#define STR_RES_SEQ__1 212  // seq. 1
#define STR_RES_SEQ__2 213  // seq. 2
#define STR_RES__ARP 214  // arp
#define STR_RES__VELO 215  // velo
#define STR_RES_AFTTCH 216  // afttch
#define STR_RES_BENDER 217  // bender
#define STR_RES_MWHEEL 218  // mwheel
#define STR_RES_WHEEL2 219  // wheel2
#define STR_RES_PEDAL 220  // pedal
#define STR_RES__NOTE 221  // note
#define STR_RES__GATE 222  // gate
#define STR_RES__NOISE 223  // noise
#define STR_RES_RANDOM 224  // random
#define STR_RES_E_256 225  // = 256
#define STR_RES_E_32 226  // = 32
#define STR_RES_E_16 227  // = 16
#define STR_RES_E_8 228  // = 8
#define STR_RES_E_4 229  // = 4
#define STR_RES_PARAM_1 230  // param 1
#define STR_RES_PARAM_2 231  // param 2
#define STR_RES_OSC_1 232  // osc 1
#define STR_RES_OSC_2 233  // osc 2
#define STR_RES_OSC_1S2 234  // osc 1+2
#define STR_RES_VIBRATO 235  // vibrato
#define STR_RES__MIX 236  // mix
#define STR_RES__XMOD 237  // xmod
#define STR_RES___NOISE 238  // noise
#define STR_RES_SUBOSC 239  // subosc
#define STR_RES__FUZZ 240  // fuzz
#define STR_RES_CRUSH 241  // crush
#define STR_RES_FREQUENCY 242  // frequency
#define STR_RES__RESO 243  // reso
#define STR_RES__ATTACK 244  // attack
#define STR_RES__DECAY 245  // decay
#define STR_RES__RELEASE 246  // release
#define STR_RES__LFO_4 247  // lfo 4
#define STR_RES__VCA 248  // vca
#define STR_RES___FOLD 249  // fold
#define STR_RES_LP 250  // lp
#define STR_RES_BP 251  // bp
#define STR_RES_HP 252  // hp
#define STR_RES_NT 253  // nt
#define STR_RES_FREE 254  // free
#define STR_RES_ENVTLFO 255  // env~lfo
#define STR_RES_LFOTENV 256  // lfo~env
#define STR_RES_STEP_SEQ 257  // step seq
#define STR_RES_ARPEGGIO 258  // arpeggio
#define STR_RES__PATTERN 259  // pattern
#define STR_RES__OFF 260  // off
#define STR_RES_ADD 261  // add
#define STR_RES_PROD 262  // prod
#define STR_RES_ATTN 263  // attn
#define STR_RES_MAX 264  // max
#define STR_RES_MIN 265  // min
#define STR_RES__XOR 266  // xor
#define STR_RES_GE 267  // >=
#define STR_RES_LE 268  // <=
#define STR_RES_QTZ 269  // qtz
#define STR_RES_LAG 270  // lag
#define STR_RES_MONO 271  // mono
#define STR_RES_POLY 272  // poly
#define STR_RES_2X_UNISON 273  // 2x unison
#define STR_RES_CYCLIC 274  // cyclic
#define STR_RES_CHAIN 275  // chain
#define STR_RES_UP 276  // up
#define STR_RES_DOWN 277  // down
#define STR_RES_UP_DOWN 278  // up&down
#define STR_RES_PLAYED 279  // played
#define STR_RES__RANDOM 280  // random
#define STR_RES_CHORD 281  // chord
#define STR_RES_1_96 282  // 1/96
#define STR_RES_1_48 283  // 1/48
#define STR_RES_1_32 284  // 1/32
#define STR_RES_1_24 285  // 1/24
#define STR_RES_1_16 286  // 1/16
#define STR_RES_1_12 287  // 1/12
#define STR_RES_1_8 288  // 1/8
#define STR_RES_1_6 289  // 1/6
#define STR_RES_1_4 290  // 1/4
#define STR_RES_1_3 291  // 1/3
#define STR_RES_3_8 292  // 3/8
#define STR_RES_1_2 293  // 1/2
#define STR_RES_2_3 294  // 2/3
#define STR_RES_3_4 295  // 3/4
#define STR_RES_1_1 296  // 1/1
#define STR_RES_THRU 297  // thru
#define STR_RES_SEQUENCER 298  // sequencer
#define STR_RES_CONTROLLR 299  // controllr
#define STR_RES__CHAIN 300  // chain
#define STR_RES_FULL 301  // full
#define STR_RES_____ 302  // ....
#define STR_RES____S 303  // ...s
#define STR_RES___P_ 304  // ..p.
#define STR_RES___PS 305  // ..ps
#define STR_RES__N__ 306  // .n..
#define STR_RES__N_S 307  // .n.s
#define STR_RES__NP_ 308  // .np.
#define STR_RES__NPS 309  // .nps
#define STR_RES_C___ 310  // c...
#define STR_RES_C__S 311  // c..s
#define STR_RES_C_P_ 312  // c.p.
#define STR_RES_C_PS 313  // c.ps
#define STR_RES_CN__ 314  // cn..
#define STR_RES_CN_S 315  // cn.s
#define STR_RES_CNP_ 316  // cnp.
#define STR_RES_CNPS 317  // cnps
#define STR_RES_SWING 318  // swing
#define STR_RES_SHUFFLE 319  // shuffle
#define STR_RES_PUSH 320  // push
#define STR_RES__LAG 321  // lag
#define STR_RES_HUMAN 322  // human
#define STR_RES_MONKEY 323  // monkey
#define STR_RES_OSCILLATOR_1 324  // oscillator 1
#define STR_RES_OSCILLATOR_2 325  // oscillator 2
#define STR_RES_MIXER 326  // mixer
#define STR_RES_LFO 327  // lfo
#define STR_RES_FILTER_1 328  // filter 1
#define STR_RES_FILTER_2 329  // filter 2
#define STR_RES_ENVELOPE 330  // envelope
#define STR_RES_ARPEGGIATOR 331  // arpeggiator
#define STR_RES_MULTI 332  // multi
#define STR_RES_CLOCK 333  // clock
#define STR_RES_PERFORMANCE 334  // performance
#define STR_RES_SYSTEM 335  // system
#define STR_RES_PT_X_PATCH 336  // pt X patch
#define STR_RES_PT_X_SEQUENCE 337  // pt X sequence
#define STR_RES_PT_X_PROGRAM 338  // pt X program
#define STR_RES_RANDOMIZE 339  // randomize
#define STR_RES_INIT 340  // init
#define STR_RES_PATCH 341  // PATCH
#define STR_RES_SEQUENCE 342  // SEQUENCE
#define STR_RES_PROGRAM 343  // PROGRAM
#define STR_RES__MULTI 344  // MULTI
#define STR_RES____ 345  // ___
#define STR_RES_EQUAL 346  // equal
#define STR_RES_EG 347  // eg
#define STR_RES_DEPT 348  // dept
#define STR_RES_DEPTH 349  // depth
#define STR_RES_AMP 350  // amp
#define STR_RES_FLT 351  // flt
#define STR_RES_PCH 352  // pch
#define STR_RES_RISE 353  // rise
#define STR_RES_FALL 354  // fall
#define STR_RES_CURV 355  // curv
#define STR_RES_DEST 356  // dest
#define STR_RES_SHAP 357  // shap
#define STR_RES_JUST 358  // just
#define STR_RES_PYTHAGOREAN 359  // pythagorean
#define STR_RES_1_4_EB 360  // 1/4 eb
#define STR_RES_1_4_E 361  // 1/4 e
#define STR_RES_1_4_EA 362  // 1/4 ea
#define STR_RES_BHAIRAV 363  // bhairav
#define STR_RES_GUNAKRI 364  // gunakri
#define STR_RES_MARWA 365  // marwa
#define STR_RES_SHREE 366  // shree
#define STR_RES_PURVI 367  // purvi
#define STR_RES_BILAWAL 368  // bilawal
#define STR_RES_YAMAN 369  // yaman
#define STR_RES_KAFI 370  // kafi
#define STR_RES_BHIMPALASREE 371  // bhimpalasree
#define STR_RES_DARBARI 372  // darbari
#define STR_RES_BAGESHREE 373  // bageshree
#define STR_RES_RAGESHREE 374  // rageshree
#define STR_RES_KHAMAJ 375  // khamaj
#define STR_RES_MIMAL 376  // mi'mal
#define STR_RES_PARAMESHWARI 377  // parameshwari
#define STR_RES_RANGESHWARI 378  // rangeshwari
#define STR_RES_GANGESHWARI 379  // gangeshwari
#define STR_RES_KAMESHWARI 380  // kameshwari
#define STR_RES_PA__KAFI 381  // pa. kafi
#define STR_RES_NATBHAIRAV 382  // natbhairav
#define STR_RES_M_KAUNS 383  // m.kauns
#define STR_RES_BAIRAGI 384  // bairagi
#define STR_RES_B_TODI 385  // b.todi
#define STR_RES_CHANDRADEEP 386  // chandradeep
#define STR_RES_KAUSHIK_TODI 387  // kaushik todi
#define STR_RES_JOGESHWARI 388  // jogeshwari
#define STR_RES_RASIA 389  // rasia
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
