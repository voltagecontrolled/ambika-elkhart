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
#define STR_RES_FMFB 75  // fmfb
#define STR_RES_FMCA 76  // fmca
#define STR_RES_FMCB 77  // fmcb
#define STR_RES_FMCC 78  // fmcc
#define STR_RES_FMCD 79  // fmcd
#define STR_RES_FMCE 80  // fmce
#define STR_RES_HRM1 81  // hrm1
#define STR_RES_HRM2 82  // hrm2
#define STR_RES_8BITS 83  // 8bits
#define STR_RES_PWM 84  // pwm
#define STR_RES_NOISE 85  // noise
#define STR_RES_VOWEL 86  // vowel
#define STR_RES_MALE 87  // male
#define STR_RES_FEMALE 88  // female
#define STR_RES_CHOIR 89  // choir
#define STR_RES_TAMPURA 90  // tampura
#define STR_RES_BOWED 91  // bowed
#define STR_RES_CELLO 92  // cello
#define STR_RES_VIBES 93  // vibes
#define STR_RES_SLAP 94  // slap
#define STR_RES_EPIANO 95  // epiano
#define STR_RES_ORGAN 96  // organ
#define STR_RES_WAVES 97  // waves
#define STR_RES_DIGITAL 98  // digital
#define STR_RES_DRONE_1 99  // drone 1
#define STR_RES_DRONE_2 100  // drone 2
#define STR_RES_METALLIC 101  // metallic
#define STR_RES_BELL 102  // bell
#define STR_RES_WAVQUENCE 103  // wavquence
#define STR_RES_OLDSAW 104  // oldsaw
#define STR_RES_QPWM 105  // qpwm
#define STR_RES_CSAW 106  // csaw
#define STR_RES_VOWEL_2 107  // vowel 2
#define STR_RES_TRI 108  // tri
#define STR_RES_SQR 109  // sqr
#define STR_RES_S_H 110  // s&h
#define STR_RES_RAMP 111  // ramp
#define STR_RES_1EXP 112  // 1exp
#define STR_RES_1LIN 113  // 1lin
#define STR_RES_1TRI 114  // 1tri
#define STR_RES__SINE 115  // sine
#define STR_RES__HRM2 116  // hrm2
#define STR_RES_HRM3 117  // hrm3
#define STR_RES_HRM5 118  // hrm5
#define STR_RES_GRG1 119  // grg1
#define STR_RES_GRG2 120  // grg2
#define STR_RES_BAT1 121  // bat1
#define STR_RES_BAT2 122  // bat2
#define STR_RES_SPK1 123  // spk1
#define STR_RES_SPK2 124  // spk2
#define STR_RES_LSAW 125  // lsaw
#define STR_RES_LSQR 126  // lsqr
#define STR_RES_RSAW 127  // rsaw
#define STR_RES_RSQR 128  // rsqr
#define STR_RES_STP1 129  // stp1
#define STR_RES_STP2 130  // stp2
#define STR_RES___OFF 131  // off
#define STR_RES_SYNC 132  // sync
#define STR_RES_RINGMOD 133  // ringmod
#define STR_RES_XOR 134  // xor
#define STR_RES_FOLD 135  // fold
#define STR_RES_BITS 136  // bits
#define STR_RES_SQU1 137  // squ1
#define STR_RES_TRI1 138  // tri1
#define STR_RES_PUL1 139  // pul1
#define STR_RES_SQU2 140  // squ2
#define STR_RES_TRI2 141  // tri2
#define STR_RES_PUL2 142  // pul2
#define STR_RES_CLICK 143  // click
#define STR_RES_GLITCH 144  // glitch
#define STR_RES_BLOW 145  // blow
#define STR_RES_METAL 146  // metal
#define STR_RES_POP 147  // pop
#define STR_RES_ENV1 148  // env1
#define STR_RES_ENV2 149  // env2
#define STR_RES_ENV3 150  // env3
#define STR_RES_LFO_ 151  // lfo.
#define STR_RES_LFO5 152  // lfo5
#define STR_RES__LFO_ 153  // lfo.
#define STR_RES_LFO4 154  // lfo4
#define STR_RES_MOD1 155  // mod1
#define STR_RES_MOD2 156  // mod2
#define STR_RES_MOD3 157  // mod3
#define STR_RES_MOD4 158  // mod4
#define STR_RES_SEQ1 159  // seq1
#define STR_RES_SEQ2 160  // seq2
#define STR_RES_ARP 161  // arp
#define STR_RES_VELO 162  // velo
#define STR_RES_AFTR 163  // aftr
#define STR_RES_BEND 164  // bend
#define STR_RES_MWHL 165  // mwhl
#define STR_RES_WHL2 166  // whl2
#define STR_RES_PDAL 167  // pdal
#define STR_RES_NOTE 168  // note
#define STR_RES_GATE 169  // gate
#define STR_RES_NOIS 170  // nois
#define STR_RES_RAND 171  // rand
#define STR_RES_E256 172  // =256
#define STR_RES_E128 173  // =128
#define STR_RES_E64 174  // =64
#define STR_RES_E32 175  // =32
#define STR_RES_E16 176  // =16
#define STR_RES_E8 177  // =8
#define STR_RES_E4 178  // =4
#define STR_RES_PRM1 179  // prm1
#define STR_RES_PRM2 180  // prm2
#define STR_RES_OSC1 181  // osc1
#define STR_RES_OSC2 182  // osc2
#define STR_RES_31S2 183  // 1+2
#define STR_RES_VIBR 184  // vibr
#define STR_RES_MIX 185  // mix
#define STR_RES_XMOD 186  // xmod
#define STR_RES__NOIS 187  // nois
#define STR_RES_SUB 188  // sub
#define STR_RES_FUZZ 189  // fuzz
#define STR_RES_CRSH 190  // crsh
#define STR_RES_FREQ 191  // freq
#define STR_RES_RESO 192  // reso
#define STR_RES_ATTK 193  // attk
#define STR_RES_DECA 194  // deca
#define STR_RES_RELE 195  // rele
#define STR_RES__LFO4 196  // lfo4
#define STR_RES_VCA 197  // vca
#define STR_RES__FOLD 198  // fold
#define STR_RES_ENV_1 199  // env 1
#define STR_RES_ENV_2 200  // env 2
#define STR_RES_ENV_3 201  // env 3
#define STR_RES_LFO__ 202  // lfo .
#define STR_RES_LFO_2 203  // lfo 2
#define STR_RES__LFO__ 204  // lfo .
#define STR_RES_LFO_4 205  // lfo 4
#define STR_RES_MOD__1 206  // mod. 1
#define STR_RES_MOD__2 207  // mod. 2
#define STR_RES_MOD__3 208  // mod. 3
#define STR_RES_MOD__4 209  // mod. 4
#define STR_RES_SEQ__1 210  // seq. 1
#define STR_RES_SEQ__2 211  // seq. 2
#define STR_RES__ARP 212  // arp
#define STR_RES__VELO 213  // velo
#define STR_RES_AFTTCH 214  // afttch
#define STR_RES_BENDER 215  // bender
#define STR_RES_MWHEEL 216  // mwheel
#define STR_RES_WHEEL2 217  // wheel2
#define STR_RES_PEDAL 218  // pedal
#define STR_RES__NOTE 219  // note
#define STR_RES__GATE 220  // gate
#define STR_RES__NOISE 221  // noise
#define STR_RES_RANDOM 222  // random
#define STR_RES_E_256 223  // = 256
#define STR_RES_E_32 224  // = 32
#define STR_RES_E_16 225  // = 16
#define STR_RES_E_8 226  // = 8
#define STR_RES_E_4 227  // = 4
#define STR_RES_PARAM_1 228  // param 1
#define STR_RES_PARAM_2 229  // param 2
#define STR_RES_OSC_1 230  // osc 1
#define STR_RES_OSC_2 231  // osc 2
#define STR_RES_OSC_1S2 232  // osc 1+2
#define STR_RES_VIBRATO 233  // vibrato
#define STR_RES__MIX 234  // mix
#define STR_RES__XMOD 235  // xmod
#define STR_RES___NOISE 236  // noise
#define STR_RES_SUBOSC 237  // subosc
#define STR_RES__FUZZ 238  // fuzz
#define STR_RES_CRUSH 239  // crush
#define STR_RES_FREQUENCY 240  // frequency
#define STR_RES__RESO 241  // reso
#define STR_RES__ATTACK 242  // attack
#define STR_RES__DECAY 243  // decay
#define STR_RES__RELEASE 244  // release
#define STR_RES__LFO_4 245  // lfo 4
#define STR_RES__VCA 246  // vca
#define STR_RES___FOLD 247  // fold
#define STR_RES_LP 248  // lp
#define STR_RES_BP 249  // bp
#define STR_RES_HP 250  // hp
#define STR_RES_NT 251  // nt
#define STR_RES_FREE 252  // free
#define STR_RES_ENVTLFO 253  // env~lfo
#define STR_RES_LFOTENV 254  // lfo~env
#define STR_RES_STEP_SEQ 255  // step seq
#define STR_RES_ARPEGGIO 256  // arpeggio
#define STR_RES__PATTERN 257  // pattern
#define STR_RES__OFF 258  // off
#define STR_RES_ADD 259  // add
#define STR_RES_PROD 260  // prod
#define STR_RES_ATTN 261  // attn
#define STR_RES_MAX 262  // max
#define STR_RES_MIN 263  // min
#define STR_RES__XOR 264  // xor
#define STR_RES_GE 265  // >=
#define STR_RES_LE 266  // <=
#define STR_RES_QTZ 267  // qtz
#define STR_RES_LAG 268  // lag
#define STR_RES_MONO 269  // mono
#define STR_RES_POLY 270  // poly
#define STR_RES_2X_UNISON 271  // 2x unison
#define STR_RES_CYCLIC 272  // cyclic
#define STR_RES_CHAIN 273  // chain
#define STR_RES_UP 274  // up
#define STR_RES_DOWN 275  // down
#define STR_RES_UP_DOWN 276  // up&down
#define STR_RES_PLAYED 277  // played
#define STR_RES__RANDOM 278  // random
#define STR_RES_CHORD 279  // chord
#define STR_RES_1_96 280  // 1/96
#define STR_RES_1_48 281  // 1/48
#define STR_RES_1_32 282  // 1/32
#define STR_RES_1_24 283  // 1/24
#define STR_RES_1_16 284  // 1/16
#define STR_RES_1_12 285  // 1/12
#define STR_RES_1_8 286  // 1/8
#define STR_RES_1_6 287  // 1/6
#define STR_RES_1_4 288  // 1/4
#define STR_RES_1_3 289  // 1/3
#define STR_RES_3_8 290  // 3/8
#define STR_RES_1_2 291  // 1/2
#define STR_RES_2_3 292  // 2/3
#define STR_RES_3_4 293  // 3/4
#define STR_RES_1_1 294  // 1/1
#define STR_RES_THRU 295  // thru
#define STR_RES_SEQUENCER 296  // sequencer
#define STR_RES_CONTROLLR 297  // controllr
#define STR_RES__CHAIN 298  // chain
#define STR_RES_FULL 299  // full
#define STR_RES_____ 300  // ....
#define STR_RES____S 301  // ...s
#define STR_RES___P_ 302  // ..p.
#define STR_RES___PS 303  // ..ps
#define STR_RES__N__ 304  // .n..
#define STR_RES__N_S 305  // .n.s
#define STR_RES__NP_ 306  // .np.
#define STR_RES__NPS 307  // .nps
#define STR_RES_C___ 308  // c...
#define STR_RES_C__S 309  // c..s
#define STR_RES_C_P_ 310  // c.p.
#define STR_RES_C_PS 311  // c.ps
#define STR_RES_CN__ 312  // cn..
#define STR_RES_CN_S 313  // cn.s
#define STR_RES_CNP_ 314  // cnp.
#define STR_RES_CNPS 315  // cnps
#define STR_RES_SWING 316  // swing
#define STR_RES_SHUFFLE 317  // shuffle
#define STR_RES_PUSH 318  // push
#define STR_RES__LAG 319  // lag
#define STR_RES_HUMAN 320  // human
#define STR_RES_MONKEY 321  // monkey
#define STR_RES_OSCILLATOR_1 322  // oscillator 1
#define STR_RES_OSCILLATOR_2 323  // oscillator 2
#define STR_RES_MIXER 324  // mixer
#define STR_RES_LFO 325  // lfo
#define STR_RES_FILTER_1 326  // filter 1
#define STR_RES_FILTER_2 327  // filter 2
#define STR_RES_ENVELOPE 328  // envelope
#define STR_RES_ARPEGGIATOR 329  // arpeggiator
#define STR_RES_MULTI 330  // multi
#define STR_RES_CLOCK 331  // clock
#define STR_RES_PERFORMANCE 332  // performance
#define STR_RES_SYSTEM 333  // system
#define STR_RES_PT_X_PATCH 334  // pt X patch
#define STR_RES_PT_X_SEQUENCE 335  // pt X sequence
#define STR_RES_PT_X_PROGRAM 336  // pt X program
#define STR_RES_RANDOMIZE 337  // randomize
#define STR_RES_INIT 338  // init
#define STR_RES_PATCH 339  // PATCH
#define STR_RES_SEQUENCE 340  // SEQUENCE
#define STR_RES_PROGRAM 341  // PROGRAM
#define STR_RES__MULTI 342  // MULTI
#define STR_RES____ 343  // ___
#define STR_RES_EQUAL 344  // equal
#define STR_RES_EG 345  // eg
#define STR_RES_DEPT 346  // dept
#define STR_RES_DEPTH 347  // depth
#define STR_RES_AMP 348  // amp
#define STR_RES_FLT 349  // flt
#define STR_RES_PCH 350  // pch
#define STR_RES_RISE 351  // rise
#define STR_RES_FALL 352  // fall
#define STR_RES_CURV 353  // curv
#define STR_RES_DEST 354  // dest
#define STR_RES_SHAP 355  // shap
#define STR_RES_JUST 356  // just
#define STR_RES_PYTHAGOREAN 357  // pythagorean
#define STR_RES_1_4_EB 358  // 1/4 eb
#define STR_RES_1_4_E 359  // 1/4 e
#define STR_RES_1_4_EA 360  // 1/4 ea
#define STR_RES_BHAIRAV 361  // bhairav
#define STR_RES_GUNAKRI 362  // gunakri
#define STR_RES_MARWA 363  // marwa
#define STR_RES_SHREE 364  // shree
#define STR_RES_PURVI 365  // purvi
#define STR_RES_BILAWAL 366  // bilawal
#define STR_RES_YAMAN 367  // yaman
#define STR_RES_KAFI 368  // kafi
#define STR_RES_BHIMPALASREE 369  // bhimpalasree
#define STR_RES_DARBARI 370  // darbari
#define STR_RES_BAGESHREE 371  // bageshree
#define STR_RES_RAGESHREE 372  // rageshree
#define STR_RES_KHAMAJ 373  // khamaj
#define STR_RES_MIMAL 374  // mi'mal
#define STR_RES_PARAMESHWARI 375  // parameshwari
#define STR_RES_RANGESHWARI 376  // rangeshwari
#define STR_RES_GANGESHWARI 377  // gangeshwari
#define STR_RES_KAMESHWARI 378  // kameshwari
#define STR_RES_PA__KAFI 379  // pa. kafi
#define STR_RES_NATBHAIRAV 380  // natbhairav
#define STR_RES_M_KAUNS 381  // m.kauns
#define STR_RES_BAIRAGI 382  // bairagi
#define STR_RES_B_TODI 383  // b.todi
#define STR_RES_CHANDRADEEP 384  // chandradeep
#define STR_RES_KAUSHIK_TODI 385  // kaushik todi
#define STR_RES_JOGESHWARI 386  // jogeshwari
#define STR_RES_RASIA 387  // rasia
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
