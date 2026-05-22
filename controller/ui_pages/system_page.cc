#include "controller/ui_pages/system_page.h"

#include "avrlib/op.h"
#include "avrlib/string.h"
#include "avrlib/time.h"

#include "controller/display.h"
#include "controller/leds.h"
#include "controller/multi.h"
#include "controller/snapshot.h"
#include "controller/storage.h"

namespace ambika {

/* static */ uint8_t  SystemPage::mode_;
/* static */ uint8_t  SystemPage::cur_slot_ = 0xff;
/* static */ uint8_t  SystemPage::new_slot_;
/* static */ uint8_t  SystemPage::pending_slot_;
/* static */ uint8_t  SystemPage::hold_button_;
/* static */ uint16_t SystemPage::hold_arm_ms_;
/* static */ uint8_t  SystemPage::hold_fired_;
/* static */ uint8_t  SystemPage::feedback_button_;
/* static */ uint8_t  SystemPage::feedback_status_;
/* static */ uint16_t SystemPage::feedback_start_ms_;
/* static */ uint8_t  SystemPage::new_slot_occupied_cached_;
/* static */ uint8_t  SystemPage::new_slot_occupied_for_ = 0xff;

// Hold-to-confirm: 300 ms arm → fast blink, 900 ms fire. Feedback runs
// for 600 ms after fire — two slow blinks (~3 Hz) of green (success) or
// red (fail) on the originating button. Cancels are silent.
static const uint16_t kHoldArmMs       = 300;
static const uint16_t kHoldFireMs      = 900;
static const uint16_t kFeedbackTotalMs = 600;

/* static */
const prog_EventHandlers SystemPage::event_handlers_ PROGMEM = {
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
void SystemPage::OnInit(PageInfo* info) {
  active_control_ = 0;
  mode_ = MODE_MENU;
  hold_button_     = kHoldNone;
  hold_arm_ms_     = 0;
  hold_fired_      = 0;
  feedback_button_ = kHoldNone;
  feedback_status_ = kFbNone;
  if (new_slot_ >= Snapshot::kNumSlots) new_slot_ = 0;
  storage.InitFilesystem();
}

/* static */
uint8_t SystemPage::OnIncrement(int8_t increment) {
  new_slot_ = Clip(int16_t(new_slot_) + increment, 0, Snapshot::kNumSlots - 1);
  // Invalidate the occupancy cache; UpdateScreen will refresh once before
  // next render. Don't query SD here — encoder spins can be rapid.
  new_slot_occupied_for_ = 0xff;
  return 1;
}

/* static */
void SystemPage::RefreshOccupancyCache() {
  if (new_slot_occupied_for_ != new_slot_) {
    new_slot_occupied_cached_ = Snapshot::SlotOccupied(new_slot_);
    new_slot_occupied_for_    = new_slot_;
  }
}

/* static */
uint8_t SystemPage::OnPot(uint8_t index, uint8_t value) {
  if (index == 2) {
    uint8_t bpm = 40 + ((static_cast<uint16_t>(value) * 200) >> 7);
    if (bpm > 240) bpm = 240;
    multi.SetValue(PRM_MULTI_CLOCK_BPM, bpm);
  } else if (index == 3) {
    uint8_t clk = (value >> 5) & 0x03;
    multi.mutable_data()->midi_clock_mode = clk;
  }
  return 1;
}

/* static */
void SystemPage::DoSave() {
  FilesystemStatus s = Snapshot::Save(pending_slot_);
  feedback_button_    = kHoldSave;
  feedback_start_ms_  = static_cast<uint16_t>(avrlib::milliseconds());
  if (s == FS_OK) {
    cur_slot_         = pending_slot_;
    feedback_status_  = kFbSuccess;
    new_slot_occupied_for_ = 0xff;  // saved → cache stale
  } else {
    feedback_status_  = kFbFail;
  }
}

/* static */
void SystemPage::DoLoad() {
  FilesystemStatus s = (Snapshot::SlotOccupied(new_slot_))
      ? Snapshot::Load(new_slot_) : FS_DISK_ERROR;
  feedback_button_    = kHoldLoad;
  feedback_start_ms_  = static_cast<uint16_t>(avrlib::milliseconds());
  if (s == FS_OK) {
    cur_slot_         = new_slot_;
    feedback_status_  = kFbSuccess;
  } else {
    feedback_status_  = kFbFail;
  }
}

/* static */
uint8_t SystemPage::OnKey(uint8_t key) {
  // No immediate action on tap — hold-to-confirm only. The release here
  // either fires nothing (cancel) or cleans up after OnIdle already fired.
  if (key == SWITCH_1 || key == SWITCH_3) {
    hold_button_  = kHoldNone;
    hold_arm_ms_  = 0;
    hold_fired_   = 0;
  } else if (key == SWITCH_5) {
    ui.ShowPage(PAGE_OS_INFO);
  } else if (key == SWITCH_7) {
    ui.ShowPreviousPage();
  }
  return 1;
}

/* static */
void SystemPage::OnDialogClosed(uint8_t /*dialog_id*/, uint8_t /*return_value*/) {
  // No dialogs surfaced from this page in v4.2.
}

/* static */
uint8_t SystemPage::OnIdle() {
  // Drive both save (S1, sr 7) and load (S3, sr 5) through the same
  // hold-to-fire state machine. Only one button can be armed at a time.
  uint8_t s1_held = ui.switch_held(7);
  uint8_t s3_held = ui.switch_held(5);

  uint8_t which = kHoldNone;
  if (s1_held) which = kHoldSave;
  else if (s3_held) which = kHoldLoad;

  if (which == kHoldNone) {
    hold_button_ = kHoldNone;
    hold_arm_ms_ = 0;
    hold_fired_  = 0;
    return 0;
  }

  // If a different button than the one we were tracking is now held,
  // restart the timeline.
  if (hold_button_ != which) {
    hold_button_ = which;
    hold_arm_ms_ = 0;
    hold_fired_  = 0;
  }

  if (hold_fired_) return 0;

  uint16_t now = static_cast<uint16_t>(avrlib::milliseconds());
  if (hold_arm_ms_ == 0) {
    hold_arm_ms_ = now;
    return 0;
  }
  if (static_cast<uint16_t>(now - hold_arm_ms_) >= kHoldFireMs) {
    hold_fired_ = 1;
    if (which == kHoldSave) {
      pending_slot_ = new_slot_;
      DoSave();
    } else {
      DoLoad();
    }
  }
  return 0;
}

/* static */
void SystemPage::RenderSlot(char* buf, uint8_t slot) {
  if (slot >= Snapshot::kNumSlots) {
    buf[0] = '-'; buf[1] = '-';
  } else {
    buf[0] = '0' + (slot / 10);
    buf[1] = '0' + (slot % 10);
  }
}

/* static */
void SystemPage::UpdateScreen() {
  char* full0 = display.line_buffer(0);
  char* full1 = display.line_buffer(1);
  for (uint8_t i = 0; i < kLcdWidth; ++i) {
    full0[i] = ' ';
    full1[i] = ' ';
  }
  char* l0 = full0 + 1;
  char* l1 = full1 + 1;

  // Top row: Cur: | Next: | BPM | CLK.
  memcpy_P(&l0[0], PSTR("Cur:"), 4);
  RenderSlot(&l0[5], cur_slot_);
  l0[9] = kDelimiter;
  memcpy_P(&l0[10], PSTR("Next:"), 5);
  RenderSlot(&l0[15], new_slot_);
  RefreshOccupancyCache();
  l0[18] = new_slot_occupied_cached_ ? '*' : ' ';
  l0[19] = kDelimiter;
  memcpy_P(&l0[20], PSTR("BPM"), 3);
  {
    uint8_t bpm = multi.data().clock_bpm;
    UnsafeItoa<uint8_t>(bpm, 4, &l0[24]);
    AlignRight(&l0[24], 4);
  }
  l0[29] = kDelimiter;
  memcpy_P(&l0[30], PSTR("CLK"), 3);
  {
    uint8_t clk = multi.data().midi_clock_mode & 3;
    static const prog_char kClkLabels[] PROGMEM =
      " INT" " EXT" " OUT" " THR";
    memcpy_P(&l0[34], kClkLabels + clk * 4, 4);
  }

  // Bottom row: save (S1) | load (S3) | info (S5) | exit (S7). All four
  // labels at cell starts (col 0/10/20/30), delimiters at 9/19/29.
  memcpy_P(&l1[0],  PSTR("save"), 4);
  memcpy_P(&l1[10], PSTR("load"), 4);
  memcpy_P(&l1[20], PSTR("info"), 4);
  memcpy_P(&l1[30], PSTR("exit"), 4);
  l1[9]  = kDelimiter;
  l1[19] = kDelimiter;
  l1[29] = kDelimiter;
}

/* static */
void SystemPage::UpdateLeds() {
  // System page owns all 8 button LEDs; no step LEDs here.
  // Per-button defaults: S1=save (amber), S3=load (amber), S7=info (red),
  // S8=exit (amber). Hold overlays a fast blink; feedback overlays a
  // slow blink (green = success, red = fail).
  uint16_t now = static_cast<uint16_t>(avrlib::milliseconds());

  uint8_t s1 = 0xf0;
  uint8_t s3 = 0xf0;

  // Hold-arming blink (~5 Hz) on the actively-held button.
  if (hold_arm_ms_ != 0 && !hold_fired_) {
    uint16_t elapsed = now - hold_arm_ms_;
    if (elapsed >= kHoldArmMs) {
      uint8_t blink_on = ((now & 0xff) < 0x80) ? 0xf0 : 0x00;
      if (hold_button_ == kHoldSave) s1 = blink_on;
      else if (hold_button_ == kHoldLoad) s3 = blink_on;
    }
  }

  // Post-fire feedback (~3 Hz) on the originating button for kFeedbackTotalMs.
  // Color nibbles: empirically on this hardware 0x0f shows as green and
  // 0xf0 as red (opposite of the codebase comment in seq_mixer_page —
  // likely a swap_leds_colors interaction). Adjust here if the rest of
  // the firmware ever gets canonicalized.
  if (feedback_status_ != kFbNone) {
    uint16_t fb_elapsed = now - feedback_start_ms_;
    if (fb_elapsed >= kFeedbackTotalMs) {
      feedback_status_ = kFbNone;
      feedback_button_ = kHoldNone;
    } else {
      uint8_t on_phase = ((fb_elapsed / 150) & 1) == 0;
      uint8_t color = (feedback_status_ == kFbSuccess) ? 0x0f : 0xf0;
      uint8_t lit   = on_phase ? color : 0x00;
      if (feedback_button_ == kHoldSave) s1 = lit;
      else if (feedback_button_ == kHoldLoad) s3 = lit;
    }
  }

  leds.set_pixel(LED_1, s1);
  leds.set_pixel(LED_3, s3);
  leds.set_pixel(LED_5, 0x0f);
  leds.set_pixel(LED_7, 0xf0);
}

}  // namespace ambika
