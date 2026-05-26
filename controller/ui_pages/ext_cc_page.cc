// EXT-track MIDI CC editing page (issue #32).
//
// 4 CC slots per track. Top row = CC# (with per-slot enable bit); bot row =
// CC value. Encoder turn walks cursor 0..7 across the 4 cells × 2 rows.
// Encoder click toggles the slot enable bit (bit 7 of cc_map byte) for the
// cursor's slot. Pots 0..3 dial CC#; pots 4..7 dial value. Value pots
// write to LockPool (key kCcSlotLockBase+slot) when a step is held, else to
// midi_cc_values track default. Live MIDI CC emit on every value-pot turn
// so external gear sweeps with the knob even while the sequencer is silent.
//
// Reachable only when multi.track_is_ext(active_part). The nav layer
// (ui.cc) is responsible for swapping this page in place of the synth
// pages on EXT tracks; this page itself assumes EXT and renders 8 cells
// of CC editing.

#include "controller/ui_pages/ext_cc_page.h"

#include "avrlib/string.h"
#include "avrlib/time.h"
#include "controller/display.h"
#include "controller/leds.h"
#include "controller/midi_dispatcher.h"
#include "controller/multi.h"
#include "controller/sequencer.h"
#include "controller/ui.h"

namespace ambika {

// Step-button gesture thresholds — match SeqStepsPage so behavior across
// pages is consistent. Hold ≥ kStepLongPressMs = peek (no toggle on release).
// Two short taps within kStepDoubleTapMs = clear all locks for that step.
static const uint16_t kStepLongPressMs = 250;
static const uint16_t kStepDoubleTapMs = 300;

/* static */ uint8_t  ExtCcPage::cursor_         = 0;
/* static */ uint8_t  ExtCcPage::last_tap_step_  = 0xff;
/* static */ uint16_t ExtCcPage::last_tap_ms_    = 0;

/* static */
const prog_EventHandlers ExtCcPage::event_handlers_ PROGMEM = {
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
uint8_t ExtCcPage::OnIncrement(int8_t increment) {
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
uint8_t ExtCcPage::OnClick() {
  // Toggle enable bit for the cursor's slot. CC# (top) and VAL (bot) for the
  // same slot share an enable — clicking on either row flips the slot.
  uint8_t track = ui.state().active_part;
  uint8_t slot = cursor_ & 3;
  uint8_t* m = &multi.mutable_data()->midi_cc_map[track][slot];
  *m ^= 0x80;
  return 1;
}

/* static */
uint8_t ExtCcPage::OnPot(uint8_t index, uint8_t value) {
  if (index >= 8) return 0;
  uint8_t track = ui.state().active_part;
  uint8_t slot = index & 3;
  cursor_ = index;

  if (index < 4) {
    // Top row: CC#. Pot 0..127 → CC# 0..127 directly. Preserve the enable
    // bit so dialling a CC# doesn't accidentally arm the slot.
    uint8_t* m = &multi.mutable_data()->midi_cc_map[track][slot];
    *m = (*m & 0x80) | (value & 0x7f);
    return 1;
  }

  // Bot row: VAL. Find held step; write a per-step lock if held, else the
  // track default. Live-emit to external gear iff the slot is enabled.
  uint8_t held_sr = 0xff;
  for (uint8_t s = 0; s < 8; ++s) {
    if (ui.switch_held(s)) { held_sr = s; break; }
  }
  uint8_t mapped = value & 0x7f;
  if (held_sr != 0xff) {
    uint8_t held_step = 7 - held_sr;
    sequencer.SetStepLock(track, held_step,
                          kCcSlotLockBase + slot, mapped);
    ui.inhibit_switch(1 << held_sr);
  } else {
    multi.mutable_data()->midi_cc_values[track][slot] = mapped;
  }
  if (multi.cc_slot_enabled(track, slot)) {
    uint8_t ch = (multi.track_channel(track) - 1) & 0x0f;
    midi_dispatcher.SendSlotCc(ch, multi.cc_slot_number(track, slot), mapped);
  }
  return 1;
}

/* static */
uint8_t ExtCcPage::OnKey(uint8_t key) {
  if (key > SWITCH_8) return 0;
  uint8_t track = ui.state().active_part;
  uint8_t sr = 7 - key;
  uint16_t hold = ui.last_hold_ms(sr);
  ui.clear_last_hold_ms(sr);
  // Long-press = peek (no toggle on release). Lock edits set
  // inhibit_switch upstream so OnKey never fires for the lock-edit case.
  if (hold >= kStepLongPressMs) {
    last_tap_step_ = 0xff;
    return 1;
  }
  uint16_t now = static_cast<uint16_t>(avrlib::milliseconds());
  SeqStep& s = sequencer.mutable_track(track)->steps[key];
  // Double-tap-clear: two short taps on the same step within the window
  // wipe every per-step lock (intrinsic + pool + per-lock PROB) for that
  // step and undo the first tap's toggle. Matches SeqStepsPage.
  if (last_tap_step_ == key && (now - last_tap_ms_) < kStepDoubleTapMs) {
    sequencer.ClearAllStepLocks(track, key);
    s.step_flags ^= kStepFlagOn;
    last_tap_step_ = 0xff;
    return 1;
  }
  s.step_flags ^= kStepFlagOn;
  last_tap_step_ = key;
  last_tap_ms_ = now;
  return 1;
}

static void WriteU8Right4(char* buf, uint8_t value) {
  buf[0] = ' ';
  UnsafeItoa<uint8_t>(value, 3, &buf[1]);
  AlignRight(&buf[1], 3);
}

/* static */
void ExtCcPage::UpdateScreen() {
  uint8_t track = ui.state().active_part;
  char* line0 = display.line_buffer(0);
  char* line1 = display.line_buffer(1);

  // Held-step resolves bot-row value as (lock or default).
  uint8_t held_step = 0xff;
  for (uint8_t s = 0; s < 8; ++s) {
    if (ui.switch_held(s)) { held_step = 7 - s; break; }
  }

  for (uint8_t slot = 0; slot < 4; ++slot) {
    char* buf0 = line0 + slot * 10;
    char* buf1 = line1 + slot * 10;
    if (slot != 0) {
      buf0[0] = kDelimiter;
      buf1[0] = kDelimiter;
    }
    uint8_t enabled = multi.cc_slot_enabled(track, slot);
    uint8_t cc_cursor  = (cursor_ == slot);
    uint8_t val_cursor = (cursor_ == slot + 4);

    // Label convention: lowercase by default; uppercase when cursor; both
    // rows uppercase when slot enabled (advertises the armed state).
    char cc_a = cc_cursor || enabled ? 'C' : 'c';
    char val_a = val_cursor || enabled ? 'V' : 'v';
    buf0[1] = cc_a;
    buf0[2] = cc_a;
    buf0[3] = '#';
    buf0[4] = '1' + slot;
    buf1[1] = val_a;
    buf1[2] = val_cursor || enabled ? 'A' : 'a';
    buf1[3] = val_cursor || enabled ? 'L' : 'l';
    buf1[4] = '1' + slot;

    // Top row value: CC# number always shown (editable while disabled).
    WriteU8Right4(&buf0[5], multi.cc_slot_number(track, slot));

    // Bot row value: " off" when disabled; otherwise (lock-or-default) value.
    if (!enabled) {
      buf1[5] = ' '; buf1[6] = ' '; buf1[7] = 'o';
      buf1[8] = 'f'; buf1[9] = 'f';
    } else {
      uint8_t v = (held_step != 0xff)
          ? sequencer.lock_pool().Get(track, held_step,
                                      kCcSlotLockBase + slot,
                                      multi.cc_slot_value(track, slot))
          : multi.cc_slot_value(track, slot);
      WriteU8Right4(&buf1[5], v);
    }
  }
}

/* static */
void ExtCcPage::UpdateLeds() {
  UiPage::UpdateLeds();
}

}  // namespace ambika
