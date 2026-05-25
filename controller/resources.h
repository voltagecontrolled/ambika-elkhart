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
#define STR_RES_8BITS 81  // 8bits
#define STR_RES_PWM 82  // pwm
#define STR_RES_NOISE 83  // noise
#define STR_RES_VOWEL 84  // vowel
#define STR_RES_MALE 85  // male
#define STR_RES_FEMALE 86  // female
#define STR_RES_CHOIR 87  // choir
#define STR_RES_TAMPURA 88  // tampura
#define STR_RES_BOWED 89  // bowed
#define STR_RES_CELLO 90  // cello
#define STR_RES_VIBES 91  // vibes
#define STR_RES_SLAP 92  // slap
#define STR_RES_EPIANO 93  // epiano
#define STR_RES_ORGAN 94  // organ
#define STR_RES_WAVES 95  // waves
#define STR_RES_DIGITAL 96  // digital
#define STR_RES_DRONE_1 97  // drone 1
#define STR_RES_DRONE_2 98  // drone 2
#define STR_RES_METALLIC 99  // metallic
#define STR_RES_BELL 100  // bell
#define STR_RES_WAVQUENCE 101  // wavquence
#define STR_RES_OLDSAW 102  // oldsaw
#define STR_RES_QPWM 103  // qpwm
#define STR_RES_CSAW 104  // csaw
#define STR_RES_VOWEL_2 105  // vowel 2
#define STR_RES_TRI 106  // tri
#define STR_RES_SQR 107  // sqr
#define STR_RES_S_H 108  // s&h
#define STR_RES_RAMP 109  // ramp
#define STR_RES_1EXP 110  // 1exp
#define STR_RES_1LIN 111  // 1lin
#define STR_RES_1TRI 112  // 1tri
#define STR_RES__SINE 113  // sine
#define STR_RES_HRM2 114  // hrm2
#define STR_RES_HRM3 115  // hrm3
#define STR_RES_HRM5 116  // hrm5
#define STR_RES_GRG1 117  // grg1
#define STR_RES_GRG2 118  // grg2
#define STR_RES_BAT1 119  // bat1
#define STR_RES_BAT2 120  // bat2
#define STR_RES_SPK1 121  // spk1
#define STR_RES_SPK2 122  // spk2
#define STR_RES_LSAW 123  // lsaw
#define STR_RES_LSQR 124  // lsqr
#define STR_RES_RSAW 125  // rsaw
#define STR_RES_RSQR 126  // rsqr
#define STR_RES_STP1 127  // stp1
#define STR_RES_STP2 128  // stp2
#define STR_RES___OFF 129  // off
#define STR_RES_SYNC 130  // sync
#define STR_RES_RINGMOD 131  // ringmod
#define STR_RES_XOR 132  // xor
#define STR_RES_FOLD 133  // fold
#define STR_RES_BITS 134  // bits
#define STR_RES_SQU1 135  // squ1
#define STR_RES_TRI1 136  // tri1
#define STR_RES_PUL1 137  // pul1
#define STR_RES_SQU2 138  // squ2
#define STR_RES_TRI2 139  // tri2
#define STR_RES_PUL2 140  // pul2
#define STR_RES_CLICK 141  // click
#define STR_RES_GLITCH 142  // glitch
#define STR_RES_BLOW 143  // blow
#define STR_RES_METAL 144  // metal
#define STR_RES_POP 145  // pop
#define STR_RES_ENV1 146  // env1
#define STR_RES_ENV2 147  // env2
#define STR_RES_ENV3 148  // env3
#define STR_RES_LFO_ 149  // lfo.
#define STR_RES_LFO5 150  // lfo5
#define STR_RES__LFO_ 151  // lfo.
#define STR_RES_LFO4 152  // lfo4
#define STR_RES_MOD1 153  // mod1
#define STR_RES_MOD2 154  // mod2
#define STR_RES_MOD3 155  // mod3
#define STR_RES_MOD4 156  // mod4
#define STR_RES_SEQ1 157  // seq1
#define STR_RES_SEQ2 158  // seq2
#define STR_RES_ARP 159  // arp
#define STR_RES_VELO 160  // velo
#define STR_RES_AFTR 161  // aftr
#define STR_RES_BEND 162  // bend
#define STR_RES_MWHL 163  // mwhl
#define STR_RES_WHL2 164  // whl2
#define STR_RES_PDAL 165  // pdal
#define STR_RES_NOTE 166  // note
#define STR_RES_GATE 167  // gate
#define STR_RES_NOIS 168  // nois
#define STR_RES_RAND 169  // rand
#define STR_RES_E256 170  // =256
#define STR_RES_E128 171  // =128
#define STR_RES_E64 172  // =64
#define STR_RES_E32 173  // =32
#define STR_RES_E16 174  // =16
#define STR_RES_E8 175  // =8
#define STR_RES_E4 176  // =4
#define STR_RES_PRM1 177  // prm1
#define STR_RES_PRM2 178  // prm2
#define STR_RES_OSC1 179  // osc1
#define STR_RES_OSC2 180  // osc2
#define STR_RES_31S2 181  // 1+2
#define STR_RES_VIBR 182  // vibr
#define STR_RES_MIX 183  // mix
#define STR_RES_XMOD 184  // xmod
#define STR_RES__NOIS 185  // nois
#define STR_RES_SUB 186  // sub
#define STR_RES_FUZZ 187  // fuzz
#define STR_RES_CRSH 188  // crsh
#define STR_RES_FREQ 189  // freq
#define STR_RES_RESO 190  // reso
#define STR_RES_ATTK 191  // attk
#define STR_RES_DECA 192  // deca
#define STR_RES_RELE 193  // rele
#define STR_RES__LFO4 194  // lfo4
#define STR_RES_VCA 195  // vca
#define STR_RES__FOLD 196  // fold
#define STR_RES_ENV_1 197  // env 1
#define STR_RES_ENV_2 198  // env 2
#define STR_RES_ENV_3 199  // env 3
#define STR_RES_LFO__ 200  // lfo .
#define STR_RES_LFO_2 201  // lfo 2
#define STR_RES__LFO__ 202  // lfo .
#define STR_RES_LFO_4 203  // lfo 4
#define STR_RES_MOD__1 204  // mod. 1
#define STR_RES_MOD__2 205  // mod. 2
#define STR_RES_MOD__3 206  // mod. 3
#define STR_RES_MOD__4 207  // mod. 4
#define STR_RES_SEQ__1 208  // seq. 1
#define STR_RES_SEQ__2 209  // seq. 2
#define STR_RES__ARP 210  // arp
#define STR_RES__VELO 211  // velo
#define STR_RES_AFTTCH 212  // afttch
#define STR_RES_BENDER 213  // bender
#define STR_RES_MWHEEL 214  // mwheel
#define STR_RES_WHEEL2 215  // wheel2
#define STR_RES_PEDAL 216  // pedal
#define STR_RES__NOTE 217  // note
#define STR_RES__GATE 218  // gate
#define STR_RES__NOISE 219  // noise
#define STR_RES_RANDOM 220  // random
#define STR_RES_E_256 221  // = 256
#define STR_RES_E_32 222  // = 32
#define STR_RES_E_16 223  // = 16
#define STR_RES_E_8 224  // = 8
#define STR_RES_E_4 225  // = 4
#define STR_RES_PARAM_1 226  // param 1
#define STR_RES_PARAM_2 227  // param 2
#define STR_RES_OSC_1 228  // osc 1
#define STR_RES_OSC_2 229  // osc 2
#define STR_RES_OSC_1S2 230  // osc 1+2
#define STR_RES_VIBRATO 231  // vibrato
#define STR_RES__MIX 232  // mix
#define STR_RES__XMOD 233  // xmod
#define STR_RES___NOISE 234  // noise
#define STR_RES_SUBOSC 235  // subosc
#define STR_RES__FUZZ 236  // fuzz
#define STR_RES_CRUSH 237  // crush
#define STR_RES_FREQUENCY 238  // frequency
#define STR_RES__RESO 239  // reso
#define STR_RES__ATTACK 240  // attack
#define STR_RES__DECAY 241  // decay
#define STR_RES__RELEASE 242  // release
#define STR_RES__LFO_4 243  // lfo 4
#define STR_RES__VCA 244  // vca
#define STR_RES___FOLD 245  // fold
#define STR_RES_LP 246  // lp
#define STR_RES_BP 247  // bp
#define STR_RES_HP 248  // hp
#define STR_RES_NT 249  // nt
#define STR_RES_FREE 250  // free
#define STR_RES_ENVTLFO 251  // env~lfo
#define STR_RES_LFOTENV 252  // lfo~env
#define STR_RES_STEP_SEQ 253  // step seq
#define STR_RES_ARPEGGIO 254  // arpeggio
#define STR_RES__PATTERN 255  // pattern
#define STR_RES__OFF 256  // off
#define STR_RES_ADD 257  // add
#define STR_RES_PROD 258  // prod
#define STR_RES_ATTN 259  // attn
#define STR_RES_MAX 260  // max
#define STR_RES_MIN 261  // min
#define STR_RES__XOR 262  // xor
#define STR_RES_GE 263  // >=
#define STR_RES_LE 264  // <=
#define STR_RES_QTZ 265  // qtz
#define STR_RES_LAG 266  // lag
#define STR_RES_MONO 267  // mono
#define STR_RES_POLY 268  // poly
#define STR_RES_2X_UNISON 269  // 2x unison
#define STR_RES_CYCLIC 270  // cyclic
#define STR_RES_CHAIN 271  // chain
#define STR_RES_UP 272  // up
#define STR_RES_DOWN 273  // down
#define STR_RES_UP_DOWN 274  // up&down
#define STR_RES_PLAYED 275  // played
#define STR_RES__RANDOM 276  // random
#define STR_RES_CHORD 277  // chord
#define STR_RES_1_96 278  // 1/96
#define STR_RES_1_48 279  // 1/48
#define STR_RES_1_32 280  // 1/32
#define STR_RES_1_24 281  // 1/24
#define STR_RES_1_16 282  // 1/16
#define STR_RES_1_12 283  // 1/12
#define STR_RES_1_8 284  // 1/8
#define STR_RES_1_6 285  // 1/6
#define STR_RES_1_4 286  // 1/4
#define STR_RES_1_3 287  // 1/3
#define STR_RES_3_8 288  // 3/8
#define STR_RES_1_2 289  // 1/2
#define STR_RES_2_3 290  // 2/3
#define STR_RES_3_4 291  // 3/4
#define STR_RES_1_1 292  // 1/1
#define STR_RES_THRU 293  // thru
#define STR_RES_SEQUENCER 294  // sequencer
#define STR_RES_CONTROLLR 295  // controllr
#define STR_RES__CHAIN 296  // chain
#define STR_RES_FULL 297  // full
#define STR_RES_____ 298  // ....
#define STR_RES____S 299  // ...s
#define STR_RES___P_ 300  // ..p.
#define STR_RES___PS 301  // ..ps
#define STR_RES__N__ 302  // .n..
#define STR_RES__N_S 303  // .n.s
#define STR_RES__NP_ 304  // .np.
#define STR_RES__NPS 305  // .nps
#define STR_RES_C___ 306  // c...
#define STR_RES_C__S 307  // c..s
#define STR_RES_C_P_ 308  // c.p.
#define STR_RES_C_PS 309  // c.ps
#define STR_RES_CN__ 310  // cn..
#define STR_RES_CN_S 311  // cn.s
#define STR_RES_CNP_ 312  // cnp.
#define STR_RES_CNPS 313  // cnps
#define STR_RES_SWING 314  // swing
#define STR_RES_SHUFFLE 315  // shuffle
#define STR_RES_PUSH 316  // push
#define STR_RES__LAG 317  // lag
#define STR_RES_HUMAN 318  // human
#define STR_RES_MONKEY 319  // monkey
#define STR_RES_OSCILLATOR_1 320  // oscillator 1
#define STR_RES_OSCILLATOR_2 321  // oscillator 2
#define STR_RES_MIXER 322  // mixer
#define STR_RES_LFO 323  // lfo
#define STR_RES_FILTER_1 324  // filter 1
#define STR_RES_FILTER_2 325  // filter 2
#define STR_RES_ENVELOPE 326  // envelope
#define STR_RES_ARPEGGIATOR 327  // arpeggiator
#define STR_RES_MULTI 328  // multi
#define STR_RES_CLOCK 329  // clock
#define STR_RES_PERFORMANCE 330  // performance
#define STR_RES_SYSTEM 331  // system
#define STR_RES_PT_X_PATCH 332  // pt X patch
#define STR_RES_PT_X_SEQUENCE 333  // pt X sequence
#define STR_RES_PT_X_PROGRAM 334  // pt X program
#define STR_RES_RANDOMIZE 335  // randomize
#define STR_RES_INIT 336  // init
#define STR_RES_PATCH 337  // PATCH
#define STR_RES_SEQUENCE 338  // SEQUENCE
#define STR_RES_PROGRAM 339  // PROGRAM
#define STR_RES__MULTI 340  // MULTI
#define STR_RES____ 341  // ___
#define STR_RES_EQUAL 342  // equal
#define STR_RES_EG 343  // eg
#define STR_RES_DEPT 344  // dept
#define STR_RES_DEPTH 345  // depth
#define STR_RES_AMP 346  // amp
#define STR_RES_FLT 347  // flt
#define STR_RES_PCH 348  // pch
#define STR_RES_RISE 349  // rise
#define STR_RES_FALL 350  // fall
#define STR_RES_CURV 351  // curv
#define STR_RES_DEST 352  // dest
#define STR_RES_SHAP 353  // shap
#define STR_RES_JUST 354  // just
#define STR_RES_PYTHAGOREAN 355  // pythagorean
#define STR_RES_1_4_EB 356  // 1/4 eb
#define STR_RES_1_4_E 357  // 1/4 e
#define STR_RES_1_4_EA 358  // 1/4 ea
#define STR_RES_BHAIRAV 359  // bhairav
#define STR_RES_GUNAKRI 360  // gunakri
#define STR_RES_MARWA 361  // marwa
#define STR_RES_SHREE 362  // shree
#define STR_RES_PURVI 363  // purvi
#define STR_RES_BILAWAL 364  // bilawal
#define STR_RES_YAMAN 365  // yaman
#define STR_RES_KAFI 366  // kafi
#define STR_RES_BHIMPALASREE 367  // bhimpalasree
#define STR_RES_DARBARI 368  // darbari
#define STR_RES_BAGESHREE 369  // bageshree
#define STR_RES_RAGESHREE 370  // rageshree
#define STR_RES_KHAMAJ 371  // khamaj
#define STR_RES_MIMAL 372  // mi'mal
#define STR_RES_PARAMESHWARI 373  // parameshwari
#define STR_RES_RANGESHWARI 374  // rangeshwari
#define STR_RES_GANGESHWARI 375  // gangeshwari
#define STR_RES_KAMESHWARI 376  // kameshwari
#define STR_RES_PA__KAFI 377  // pa. kafi
#define STR_RES_NATBHAIRAV 378  // natbhairav
#define STR_RES_M_KAUNS 379  // m.kauns
#define STR_RES_BAIRAGI 380  // bairagi
#define STR_RES_B_TODI 381  // b.todi
#define STR_RES_CHANDRADEEP 382  // chandradeep
#define STR_RES_KAUSHIK_TODI 383  // kaushik todi
#define STR_RES_JOGESHWARI 384  // jogeshwari
#define STR_RES_RASIA 385  // rasia
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
