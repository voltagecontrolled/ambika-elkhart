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
#define STR_RES__SINE 122  // sine
#define STR_RES_HRM2 123  // hrm2
#define STR_RES_HRM3 124  // hrm3
#define STR_RES_HRM5 125  // hrm5
#define STR_RES_GRG1 126  // grg1
#define STR_RES_GRG2 127  // grg2
#define STR_RES_BAT1 128  // bat1
#define STR_RES_BAT2 129  // bat2
#define STR_RES_SPK1 130  // spk1
#define STR_RES_SPK2 131  // spk2
#define STR_RES_LSAW 132  // lsaw
#define STR_RES_LSQR 133  // lsqr
#define STR_RES_RSAW 134  // rsaw
#define STR_RES_RSQR 135  // rsqr
#define STR_RES_STP1 136  // stp1
#define STR_RES_STP2 137  // stp2
#define STR_RES___OFF 138  // off
#define STR_RES_SYNC 139  // sync
#define STR_RES_RINGMOD 140  // ringmod
#define STR_RES_XOR 141  // xor
#define STR_RES_FOLD 142  // fold
#define STR_RES_BITS 143  // bits
#define STR_RES_SQU1 144  // squ1
#define STR_RES_TRI1 145  // tri1
#define STR_RES_PUL1 146  // pul1
#define STR_RES_SQU2 147  // squ2
#define STR_RES_TRI2 148  // tri2
#define STR_RES_PUL2 149  // pul2
#define STR_RES_CLICK 150  // click
#define STR_RES_GLITCH 151  // glitch
#define STR_RES_BLOW 152  // blow
#define STR_RES_METAL 153  // metal
#define STR_RES_POP 154  // pop
#define STR_RES_ENV1 155  // env1
#define STR_RES_ENV2 156  // env2
#define STR_RES_ENV3 157  // env3
#define STR_RES_LFO1 158  // lfo1
#define STR_RES_LFO2 159  // lfo2
#define STR_RES_LFO3 160  // lfo3
#define STR_RES_LFO4 161  // lfo4
#define STR_RES_MOD1 162  // mod1
#define STR_RES_MOD2 163  // mod2
#define STR_RES_MOD3 164  // mod3
#define STR_RES_MOD4 165  // mod4
#define STR_RES_SEQ1 166  // seq1
#define STR_RES_SEQ2 167  // seq2
#define STR_RES_ARP 168  // arp
#define STR_RES_VELO 169  // velo
#define STR_RES_AFTR 170  // aftr
#define STR_RES_BEND 171  // bend
#define STR_RES_MWHL 172  // mwhl
#define STR_RES_WHL2 173  // whl2
#define STR_RES_PDAL 174  // pdal
#define STR_RES_NOTE 175  // note
#define STR_RES_GATE 176  // gate
#define STR_RES_NOIS 177  // nois
#define STR_RES_RAND 178  // rand
#define STR_RES_E256 179  // =256
#define STR_RES_E128 180  // =128
#define STR_RES_E64 181  // =64
#define STR_RES_E32 182  // =32
#define STR_RES_E16 183  // =16
#define STR_RES_E8 184  // =8
#define STR_RES_E4 185  // =4
#define STR_RES_PRM1 186  // prm1
#define STR_RES_PRM2 187  // prm2
#define STR_RES_OSC1 188  // osc1
#define STR_RES_OSC2 189  // osc2
#define STR_RES_31S2 190  // 1+2
#define STR_RES_VIBR 191  // vibr
#define STR_RES_MIX 192  // mix
#define STR_RES_XMOD 193  // xmod
#define STR_RES__NOIS 194  // nois
#define STR_RES_SUB 195  // sub
#define STR_RES_FUZZ 196  // fuzz
#define STR_RES_CRSH 197  // crsh
#define STR_RES_FREQ 198  // freq
#define STR_RES_RESO 199  // reso
#define STR_RES_ATTK 200  // attk
#define STR_RES_DECA 201  // deca
#define STR_RES_RELE 202  // rele
#define STR_RES__LFO4 203  // lfo4
#define STR_RES_VCA 204  // vca
#define STR_RES_ENV_1 205  // env 1
#define STR_RES_ENV_2 206  // env 2
#define STR_RES_ENV_3 207  // env 3
#define STR_RES_LFO_1 208  // lfo 1
#define STR_RES_LFO_2 209  // lfo 2
#define STR_RES_LFO_3 210  // lfo 3
#define STR_RES_LFO_4 211  // lfo 4
#define STR_RES_MOD__1 212  // mod. 1
#define STR_RES_MOD__2 213  // mod. 2
#define STR_RES_MOD__3 214  // mod. 3
#define STR_RES_MOD__4 215  // mod. 4
#define STR_RES_SEQ__1 216  // seq. 1
#define STR_RES_SEQ__2 217  // seq. 2
#define STR_RES__ARP 218  // arp
#define STR_RES__VELO 219  // velo
#define STR_RES_AFTTCH 220  // afttch
#define STR_RES_BENDER 221  // bender
#define STR_RES_MWHEEL 222  // mwheel
#define STR_RES_WHEEL2 223  // wheel2
#define STR_RES_PEDAL 224  // pedal
#define STR_RES__NOTE 225  // note
#define STR_RES__GATE 226  // gate
#define STR_RES__NOISE 227  // noise
#define STR_RES_RANDOM 228  // random
#define STR_RES_E_256 229  // = 256
#define STR_RES_E_32 230  // = 32
#define STR_RES_E_16 231  // = 16
#define STR_RES_E_8 232  // = 8
#define STR_RES_E_4 233  // = 4
#define STR_RES_PARAM_1 234  // param 1
#define STR_RES_PARAM_2 235  // param 2
#define STR_RES_OSC_1 236  // osc 1
#define STR_RES_OSC_2 237  // osc 2
#define STR_RES_OSC_1S2 238  // osc 1+2
#define STR_RES_VIBRATO 239  // vibrato
#define STR_RES__MIX 240  // mix
#define STR_RES__XMOD 241  // xmod
#define STR_RES___NOISE 242  // noise
#define STR_RES_SUBOSC 243  // subosc
#define STR_RES__FUZZ 244  // fuzz
#define STR_RES_CRUSH 245  // crush
#define STR_RES_FREQUENCY 246  // frequency
#define STR_RES__RESO 247  // reso
#define STR_RES__ATTACK 248  // attack
#define STR_RES__DECAY 249  // decay
#define STR_RES__RELEASE 250  // release
#define STR_RES__LFO_4 251  // lfo 4
#define STR_RES__VCA 252  // vca
#define STR_RES_LP 253  // lp
#define STR_RES_BP 254  // bp
#define STR_RES_HP 255  // hp
#define STR_RES_NT 256  // nt
#define STR_RES_FREE 257  // free
#define STR_RES_ENVTLFO 258  // env~lfo
#define STR_RES_LFOTENV 259  // lfo~env
#define STR_RES_STEP_SEQ 260  // step seq
#define STR_RES_ARPEGGIO 261  // arpeggio
#define STR_RES__PATTERN 262  // pattern
#define STR_RES__OFF 263  // off
#define STR_RES_ADD 264  // add
#define STR_RES_PROD 265  // prod
#define STR_RES_ATTN 266  // attn
#define STR_RES_MAX 267  // max
#define STR_RES_MIN 268  // min
#define STR_RES__XOR 269  // xor
#define STR_RES_GE 270  // >=
#define STR_RES_LE 271  // <=
#define STR_RES_QTZ 272  // qtz
#define STR_RES_LAG 273  // lag
#define STR_RES_MONO 274  // mono
#define STR_RES_POLY 275  // poly
#define STR_RES_2X_UNISON 276  // 2x unison
#define STR_RES_CYCLIC 277  // cyclic
#define STR_RES_CHAIN 278  // chain
#define STR_RES_UP 279  // up
#define STR_RES_DOWN 280  // down
#define STR_RES_UP_DOWN 281  // up&down
#define STR_RES_PLAYED 282  // played
#define STR_RES__RANDOM 283  // random
#define STR_RES_CHORD 284  // chord
#define STR_RES_1_1 285  // 1/1
#define STR_RES_3_4 286  // 3/4
#define STR_RES_2_3 287  // 2/3
#define STR_RES_1_2 288  // 1/2
#define STR_RES_3_8 289  // 3/8
#define STR_RES_1_3 290  // 1/3
#define STR_RES_1_4 291  // 1/4
#define STR_RES_1_6 292  // 1/6
#define STR_RES_1_8 293  // 1/8
#define STR_RES_1_12 294  // 1/12
#define STR_RES_1_16 295  // 1/16
#define STR_RES_1_24 296  // 1/24
#define STR_RES_1_32 297  // 1/32
#define STR_RES_1_48 298  // 1/48
#define STR_RES_1_96 299  // 1/96
#define STR_RES_THRU 300  // thru
#define STR_RES_SEQUENCER 301  // sequencer
#define STR_RES_CONTROLLR 302  // controllr
#define STR_RES__CHAIN 303  // chain
#define STR_RES_FULL 304  // full
#define STR_RES_____ 305  // ....
#define STR_RES____S 306  // ...s
#define STR_RES___P_ 307  // ..p.
#define STR_RES___PS 308  // ..ps
#define STR_RES__N__ 309  // .n..
#define STR_RES__N_S 310  // .n.s
#define STR_RES__NP_ 311  // .np.
#define STR_RES__NPS 312  // .nps
#define STR_RES_C___ 313  // c...
#define STR_RES_C__S 314  // c..s
#define STR_RES_C_P_ 315  // c.p.
#define STR_RES_C_PS 316  // c.ps
#define STR_RES_CN__ 317  // cn..
#define STR_RES_CN_S 318  // cn.s
#define STR_RES_CNP_ 319  // cnp.
#define STR_RES_CNPS 320  // cnps
#define STR_RES_SWING 321  // swing
#define STR_RES_SHUFFLE 322  // shuffle
#define STR_RES_PUSH 323  // push
#define STR_RES__LAG 324  // lag
#define STR_RES_HUMAN 325  // human
#define STR_RES_MONKEY 326  // monkey
#define STR_RES_OSCILLATOR_1 327  // oscillator 1
#define STR_RES_OSCILLATOR_2 328  // oscillator 2
#define STR_RES_MIXER 329  // mixer
#define STR_RES_LFO 330  // lfo
#define STR_RES_FILTER_1 331  // filter 1
#define STR_RES_FILTER_2 332  // filter 2
#define STR_RES_ENVELOPE 333  // envelope
#define STR_RES_ARPEGGIATOR 334  // arpeggiator
#define STR_RES_MULTI 335  // multi
#define STR_RES_CLOCK 336  // clock
#define STR_RES_PERFORMANCE 337  // performance
#define STR_RES_SYSTEM 338  // system
#define STR_RES_PT_X_PATCH 339  // pt X patch
#define STR_RES_PT_X_SEQUENCE 340  // pt X sequence
#define STR_RES_PT_X_PROGRAM 341  // pt X program
#define STR_RES_RANDOMIZE 342  // randomize
#define STR_RES_INIT 343  // init
#define STR_RES_PATCH 344  // PATCH
#define STR_RES_SEQUENCE 345  // SEQUENCE
#define STR_RES_PROGRAM 346  // PROGRAM
#define STR_RES__MULTI 347  // MULTI
#define STR_RES____ 348  // ___
#define STR_RES_EQUAL 349  // equal
#define STR_RES_JUST 350  // just
#define STR_RES_PYTHAGOREAN 351  // pythagorean
#define STR_RES_1_4_EB 352  // 1/4 eb
#define STR_RES_1_4_E 353  // 1/4 e
#define STR_RES_1_4_EA 354  // 1/4 ea
#define STR_RES_BHAIRAV 355  // bhairav
#define STR_RES_GUNAKRI 356  // gunakri
#define STR_RES_MARWA 357  // marwa
#define STR_RES_SHREE 358  // shree
#define STR_RES_PURVI 359  // purvi
#define STR_RES_BILAWAL 360  // bilawal
#define STR_RES_YAMAN 361  // yaman
#define STR_RES_KAFI 362  // kafi
#define STR_RES_BHIMPALASREE 363  // bhimpalasree
#define STR_RES_DARBARI 364  // darbari
#define STR_RES_BAGESHREE 365  // bageshree
#define STR_RES_RAGESHREE 366  // rageshree
#define STR_RES_KHAMAJ 367  // khamaj
#define STR_RES_MIMAL 368  // mi'mal
#define STR_RES_PARAMESHWARI 369  // parameshwari
#define STR_RES_RANGESHWARI 370  // rangeshwari
#define STR_RES_GANGESHWARI 371  // gangeshwari
#define STR_RES_KAMESHWARI 372  // kameshwari
#define STR_RES_PA__KAFI 373  // pa. kafi
#define STR_RES_NATBHAIRAV 374  // natbhairav
#define STR_RES_M_KAUNS 375  // m.kauns
#define STR_RES_BAIRAGI 376  // bairagi
#define STR_RES_B_TODI 377  // b.todi
#define STR_RES_CHANDRADEEP 378  // chandradeep
#define STR_RES_KAUSHIK_TODI 379  // kaushik todi
#define STR_RES_JOGESHWARI 380  // jogeshwari
#define STR_RES_RASIA 381  // rasia
#define STR_RES_EG 382  // eg
#define STR_RES_DEPT 383  // dept
#define STR_RES_DEPTH 384  // depth
#define STR_RES_AMP 385  // amp
#define STR_RES_FLT 386  // flt
#define STR_RES_PCH 387  // pitc
#define STR_RES_RISE 388  // rise
#define STR_RES_FALL 389  // fall
#define STR_RES_CURV 390  // curv
#define STR_RES_DEST 391  // dest
#define STR_RES_SHAP 392  // shap
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
