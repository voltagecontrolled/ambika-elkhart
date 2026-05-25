#ifndef CONTROLLER_UI_PAGES_SYSTEM_PAGE_H_
#define CONTROLLER_UI_PAGES_SYSTEM_PAGE_H_

#include "controller/ui_pages/ui_page.h"

namespace ambika {

class SystemPage : public UiPage {
 public:
  SystemPage() { }

  static void OnInit(PageInfo* info);
  static uint8_t OnIncrement(int8_t increment);
  static uint8_t OnPot(uint8_t index, uint8_t value);
  static uint8_t OnKey(uint8_t key);
  static uint8_t OnIdle();
  static void UpdateScreen();
  static void UpdateLeds();
  static void OnDialogClosed(uint8_t dialog_id, uint8_t return_value);

  static const prog_EventHandlers event_handlers_;

 private:
  // v4.2: SAVE/LOAD sub-modes and confirmation dialogs retired (issue #30).
  // Save/load fire on hold-to-confirm; result reported via LED blink
  // (green = success, red = fail) instead of an info/error popup.
  enum Mode { MODE_MENU };

  static void DoSave();
  static void DoLoad();
  static void RenderSlot(char* buf, uint8_t slot);  // writes 2 chars or "--"

  static uint8_t mode_;
  static uint8_t cur_slot_;   // 0xff = none
  static uint8_t new_slot_;
  static uint8_t pending_slot_;

  // Cached SlotOccupied(new_slot_) result. Re-queried only when new_slot_
  // changes (encoder) or after save/load completes — every other render
  // would otherwise spin up an SdCardSession and flash the SD icon.
  static uint8_t new_slot_occupied_cached_;
  static uint8_t new_slot_occupied_for_;
  static void RefreshOccupancyCache();

  // SD card presence cached on page entry (#39). One InitFilesystem mount
  // probe per page entry; absent card → save/load buttons inert, slot
  // indicator renders `?` instead of `*`/(space). Re-checked on every
  // OnInit so re-inserting + re-entering the page picks the card up.
  static uint8_t card_present_;

  // Hold-to-confirm state for save (S1) and load (S3). Both go through the
  // same arm-then-fire timeline: 300 ms → fast-blink armed, 900 ms → fire,
  // then ~600 ms feedback blink (green = success, red = fail).
  enum HoldButton { kHoldNone = 0, kHoldSave = 1, kHoldLoad = 3 };
  enum FeedbackStatus { kFbNone = 0, kFbSuccess = 1, kFbFail = 2 };
  static uint8_t  hold_button_;        // which button is currently held
  static uint16_t hold_arm_ms_;        // 0 = not yet armed; else press ms
  static uint8_t  hold_fired_;         // already fired this press cycle
  static uint8_t  feedback_button_;    // which LED to blink (1=save, 3=load)
  static uint8_t  feedback_status_;    // success / fail
  static uint16_t feedback_start_ms_;  // when feedback animation began

  DISALLOW_COPY_AND_ASSIGN(SystemPage);
};

}  // namespace ambika

#endif  // CONTROLLER_UI_PAGES_SYSTEM_PAGE_H_
