// Performance mixer (S6b). See seq_mixer_page.h for behaviour.

#include "controller/ui_pages/seq_mixer_page.h"

#include "avrlib/string.h"
#include "controller/display.h"
#include "controller/leds.h"
#include "controller/sequencer.h"
#include "controller/ui.h"
#include "controller/voicecard_tx.h"

namespace ambika {

static const uint8_t kModeSmut = 0;
static const uint8_t kModeAmut = 1;
static const uint8_t kModeSolo = 2;
static const uint8_t kNumMixerModes = 3;

static const uint8_t kPotIdle = 0xff;

// Cell layout (matches seq_track_page convention: 4 cells/row, 10 chars/cell):
//   row 0: cell 0=v1, 1=v2, 2=v3, 3=mode
//   row 1: cell 4=v4, 5=v5, 6=v6, 7=clr-hint
// Cursor walks 0..7. Cells 3 and 7 are display-only.
static const uint8_t kCellMode = 3;
static const uint8_t kCellHint = 7;

// SR-index mapping (Ui::Poll: control = SWITCH_8 - sr_index):
//   SR 0 = S8, SR 1 = S7, SR 2 = S6, ..., SR 7 = S1.
static const uint8_t kSrS7 = 1;

static uint8_t smut_bits_ = 0;       // sequencer mute: skip future fires only
static uint8_t amut_bits_ = 0;       // audio mute: skip + Release on toggle
static uint8_t solo_bits_ = 0;       // any bit set → only solo'd are audible
static uint8_t mode_ = kModeSmut;
static uint8_t pending_toggle_ = 0;  // queued during S7 hold
static uint8_t pot_caught_ = 0;      // bit i = pot for voice i is caught
static uint8_t pot_entry_[6];        // pot value at first sight per voice

/* static */ uint8_t SeqMixerPage::cursor_ = 0;

// pot index → voice slot, or 0xff for unused.
static inline uint8_t pot_to_voice(uint8_t i) {
  if (i < 3) return i;
  if (i >= 4 && i < 7) return i - 1;
  return 0xff;
}

static inline uint8_t audible_mask(uint8_t smut, uint8_t amut, uint8_t solo) {
  uint8_t silenced = smut | amut;
  if (solo) silenced |= ~solo;
  return (~silenced) & 0x3f;
}

// SMUT toggle never triggers a Release — current note rings out naturally.
static void ApplySmutChange(uint8_t new_smut) {
  smut_bits_ = new_smut;
}

static void ApplyAmutChange(uint8_t new_amut) {
  uint8_t was = audible_mask(smut_bits_, amut_bits_, solo_bits_);
  uint8_t now = audible_mask(smut_bits_, new_amut, solo_bits_);
  uint8_t lost = was & ~now;
  amut_bits_ = new_amut;
  for (uint8_t v = 0; v < kNumVoices; ++v) {
    if (lost & (1 << v)) voicecard_tx.Kill(v);
  }
}

static void ApplySoloChange(uint8_t new_solo) {
  uint8_t was = audible_mask(smut_bits_, amut_bits_, solo_bits_);
  uint8_t now = audible_mask(smut_bits_, amut_bits_, new_solo);
  uint8_t lost = was & ~now;
  solo_bits_ = new_solo;
  for (uint8_t v = 0; v < kNumVoices; ++v) {
    if (lost & (1 << v)) voicecard_tx.Kill(v);
  }
}

static void ApplyToggleMask(uint8_t mask) {
  if (mode_ == kModeSmut) {
    ApplySmutChange(smut_bits_ ^ mask);
  } else if (mode_ == kModeAmut) {
    ApplyAmutChange(amut_bits_ ^ mask);
  } else {
    ApplySoloChange(solo_bits_ ^ mask);
  }
}

static void UnmuteAll() {
  smut_bits_ = 0;
  if (amut_bits_) ApplyAmutChange(0);
  if (solo_bits_) ApplySoloChange(0);
}

/* static */
const prog_EventHandlers SeqMixerPage::event_handlers_ PROGMEM = {
  OnInit,
  SetActiveControl,
  OnIncrement,
  OnClick,
  OnPot,
  OnKey,
  NULL,
  OnIdle,
  UpdateScreen,
  UpdateLeds,
  OnDialogClosed,
};

/* static */
void SeqMixerPage::OnInit(PageInfo* info) {
  UiPage::OnInit(info);
  pot_caught_ = 0;
  for (uint8_t i = 0; i < 6; ++i) pot_entry_[i] = kPotIdle;
  pending_toggle_ = 0;
}

/* static */
uint8_t SeqMixerPage::OnIncrement(int8_t increment) {
  int8_t next = static_cast<int8_t>(cursor_) + increment;
  if (next < 0) {
    cursor_ = 0;
    ui.ShowPageRelative(-1);
    return 1;
  }
  if (next >= 8) {
    cursor_ = 7;
    ui.ShowPageRelative(1);
    return 1;
  }
  cursor_ = next;
  return 1;
}

/* static */
uint8_t SeqMixerPage::OnPot(uint8_t index, uint8_t value) {
  if (index >= 8) return 0;
  cursor_ = index;
  uint8_t voice = pot_to_voice(index);
  if (voice >= kNumVoices) return 1;

  SeqTrack* tr = sequencer.mutable_track(voice);
  uint8_t stored = tr->pattern[kPatVOL] >> 1;
  uint8_t bit = 1 << voice;

  if (!(pot_caught_ & bit)) {
    if (pot_entry_[voice] == kPotIdle) {
      pot_entry_[voice] = value;
      return 1;
    }
    uint8_t entry = pot_entry_[voice];
    uint8_t crossed = (entry <= stored && value >= stored) ||
                      (entry >= stored && value <= stored);
    if (!crossed) return 1;
    pot_caught_ |= bit;
  }

  tr->pattern[kPatVOL] = (value < 127) ? (value << 1) : 254;
  return 1;
}

/* static */
uint8_t SeqMixerPage::OnKey(uint8_t key) {
  switch (key) {
    case SWITCH_1: case SWITCH_2: case SWITCH_3:
    case SWITCH_4: case SWITCH_5: case SWITCH_6: {
      uint8_t voice = key - SWITCH_1;
      if (ui.switch_held(kSrS7)) {
        pending_toggle_ ^= (1 << voice);
      } else {
        ApplyToggleMask(1 << voice);
      }
      return 1;
    }
    case SWITCH_7:
      if (pending_toggle_) {
        ApplyToggleMask(pending_toggle_);
        pending_toggle_ = 0;
      } else {
        mode_ = (mode_ + 1) % kNumMixerModes;
      }
      return 1;
    case SWITCH_8:
      UnmuteAll();
      return 1;
  }
  return 0;
}

static inline void DrawCellSeparators(char* buffer, uint8_t row) {
  if (row != 0)                buffer[0]  = kDelimiter;
  if ((row + 10) != kLcdWidth) buffer[10] = kDelimiter;
}

static void DrawVoiceCell(char* buffer, uint8_t voice, uint8_t cursor_on) {
  // 4-char abbr at +1 ("v1  "), value at +5 (3 chars right-aligned).
  char tag = '1' + voice;
  buffer[1] = cursor_on ? 'V' : 'v';
  buffer[2] = tag;
  buffer[3] = ' ';
  buffer[4] = ' ';
  buffer[5] = ' ';
  uint8_t v = sequencer.track(voice).pattern[kPatVOL] >> 1;
  UnsafeItoa<uint8_t>(v, 3, &buffer[6]);
  AlignRight(&buffer[6], 3);
  buffer[9] = ' ';
}

static void DrawModeCell(char* buffer, uint8_t cursor_on) {
  // "mode SMUT" — abbr at +1, mode label at +5 (4 chars). Mode label is
  // always uppercase to indicate it's the active mode.
  buffer[1] = cursor_on ? 'M' : 'm';
  buffer[2] = cursor_on ? 'O' : 'o';
  buffer[3] = cursor_on ? 'D' : 'd';
  buffer[4] = cursor_on ? 'E' : 'e';
  const char* label =
      (mode_ == kModeSmut) ? "MT-S" :
      (mode_ == kModeAmut) ? "MT-A" : "SOLO";
  for (uint8_t i = 0; i < 4; ++i) buffer[5 + i] = label[i];
  buffer[9] = ' ';
}

static void DrawHintCell(char* buffer, uint8_t cursor_on) {
  // "clr  unmt" — S8 = unmute all hint.
  buffer[1] = cursor_on ? 'C' : 'c';
  buffer[2] = cursor_on ? 'L' : 'l';
  buffer[3] = cursor_on ? 'R' : 'r';
  buffer[4] = ' ';
  buffer[5] = 'u';
  buffer[6] = 'n';
  buffer[7] = 'm';
  buffer[8] = 't';
  buffer[9] = ' ';
}

/* static */
void SeqMixerPage::UpdateScreen() {
  for (uint8_t i = 0; i < 8; ++i) {
    uint8_t line = i < 4 ? 0 : 1;
    uint8_t row = (i & 3) * 10;
    char* buffer = display.line_buffer(line) + row;
    DrawCellSeparators(buffer, row);
    uint8_t cursor_on = (i == cursor_) ? 1 : 0;
    if (i == kCellMode) {
      DrawModeCell(buffer, cursor_on);
    } else if (i == kCellHint) {
      DrawHintCell(buffer, cursor_on);
    } else {
      uint8_t voice = (i < 3) ? i : (i - 1);  // 0,1,2, _, 3,4,5, _
      DrawVoiceCell(buffer, voice, cursor_on);
    }
  }
}

/* static */
void SeqMixerPage::UpdateLeds() {
  UiPage::UpdateLeds();
  uint8_t bits = (mode_ == kModeAmut) ? amut_bits_ :
                 (mode_ == kModeSolo) ? solo_bits_ : smut_bits_;
  // SMUT/AMUT: lit = audible (bit clear). SOLO: lit = solo'd.
  uint8_t lit = (mode_ == kModeSolo) ? bits : (~bits & 0x3f);
  // Pending shift-gate toggles preview: blink the queued voices red.
  uint8_t queued = ui.switch_held(kSrS7) ? pending_toggle_ : 0;
  for (uint8_t v = 0; v < 6; ++v) {
    uint8_t mask = 1 << v;
    if (queued & mask) {
      leds.set_pixel(LED_1 + v, 0x0f);
    } else if (lit & mask) {
      leds.set_pixel(LED_1 + v, 0xf0);
    }
  }
  // S7 LED: dim red in AMUT, bright red in SOLO; off in SMUT.
  if (mode_ == kModeAmut) leds.set_pixel(LED_7, 0x03);
  if (mode_ == kModeSolo) leds.set_pixel(LED_7, 0x0f);
}

/* static */
uint8_t SeqMixerPage::skip_mask() {
  uint8_t silenced = smut_bits_ | amut_bits_;
  if (solo_bits_) silenced |= (~solo_bits_) & 0x3f;
  return silenced & 0x3f;
}

}  // namespace ambika
