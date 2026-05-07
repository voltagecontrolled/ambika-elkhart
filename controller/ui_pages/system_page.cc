#include "controller/ui_pages/system_page.h"

#include "avrlib/op.h"
#include "avrlib/string.h"

#include "controller/display.h"
#include "controller/leds.h"
#include "controller/snapshot.h"
#include "controller/storage.h"

namespace ambika {

/* static */ uint8_t SystemPage::mode_;
/* static */ uint8_t SystemPage::cur_slot_ = 0xff;
/* static */ uint8_t SystemPage::new_slot_;
/* static */ uint8_t SystemPage::pending_slot_;

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
  storage.InitFilesystem();
}

/* static */
void SystemPage::EnterMode(Mode m) {
  mode_ = m;
  if (m == MODE_SAVE || m == MODE_LOAD) {
    new_slot_ = (cur_slot_ < Snapshot::kNumSlots) ? cur_slot_ : 0;
  }
}

/* static */
uint8_t SystemPage::OnIncrement(int8_t increment) {
  if (mode_ == MODE_SAVE || mode_ == MODE_LOAD) {
    new_slot_ = Clip(int16_t(new_slot_) + increment, 0, Snapshot::kNumSlots - 1);
  }
  return 1;
}

/* static */
void SystemPage::ShowInfo(const prog_char* text) {
  Dialog d;
  d.dialog_type = DIALOG_INFO;
  d.text = text;
  d.user_text = NULL;
  ui.ShowDialogBox(DLG_INFO_SAVED, d, 0);
}

/* static */
void SystemPage::ShowError(const prog_char* text) {
  Dialog d;
  d.dialog_type = DIALOG_ERROR;
  d.text = text;
  d.user_text = NULL;
  ui.ShowDialogBox(DLG_ERROR_FAILED, d, 0);
}

/* static */
void SystemPage::DoSave() {
  FilesystemStatus s = Snapshot::Save(pending_slot_);
  if (s == FS_OK) {
    cur_slot_ = pending_slot_;
    mode_ = MODE_MENU;
    ShowInfo(PSTR("saved"));
  } else {
    ShowError(PSTR("save failed"));
  }
}

/* static */
void SystemPage::DoLoad() {
  if (!Snapshot::SlotOccupied(new_slot_)) {
    Dialog d;
    d.dialog_type = DIALOG_INFO;
    d.text = PSTR("empty slot");
    d.user_text = NULL;
    ui.ShowDialogBox(DLG_INFO_EMPTY, d, 0);
    return;
  }
  FilesystemStatus s = Snapshot::Load(new_slot_);
  if (s == FS_OK) {
    cur_slot_ = new_slot_;
    mode_ = MODE_MENU;
    ShowInfo(PSTR("loaded"));
  } else {
    ShowError(PSTR("load failed"));
  }
}

/* static */
uint8_t SystemPage::OnKey(uint8_t key) {
  switch (mode_) {
    case MODE_MENU:
      switch (key) {
        case SWITCH_1: EnterMode(MODE_SAVE); break;
        case SWITCH_4: EnterMode(MODE_LOAD); break;
        case SWITCH_7: ui.ShowPage(PAGE_OS_INFO); break;
        case SWITCH_8: ui.ShowPreviousPage(); break;
      }
      break;

    case MODE_SAVE:
      if (key == SWITCH_7) {
        pending_slot_ = new_slot_;
        if (Snapshot::SlotOccupied(new_slot_)) {
          Dialog d;
          d.dialog_type = DIALOG_CONFIRM;
          d.text = PSTR("overwrite slot?");
          d.user_text = NULL;
          ui.ShowDialogBox(DLG_OVERWRITE, d, 0);
        } else {
          DoSave();
        }
      } else if (key == SWITCH_8) {
        mode_ = MODE_MENU;
      }
      break;

    case MODE_LOAD:
      if (key == SWITCH_7) {
        DoLoad();
      } else if (key == SWITCH_8) {
        mode_ = MODE_MENU;
      }
      break;
  }
  return 1;
}

/* static */
void SystemPage::OnDialogClosed(uint8_t dialog_id, uint8_t return_value) {
  if (dialog_id == DLG_OVERWRITE && return_value == 1) {
    DoSave();
  }
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
  char* l0 = display.line_buffer(0) + 1;
  char* l1 = display.line_buffer(1) + 1;

  if (mode_ == MODE_MENU) {
    memcpy_P(&l0[0], PSTR("system"), 6);
    memcpy_P(&l0[10], PSTR("Cur:"), 4);
    RenderSlot(&l0[15], cur_slot_);

    memcpy_P(&l1[0],  PSTR("save"), 4);
    memcpy_P(&l1[15], PSTR("load"), 4);
    memcpy_P(&l1[30], PSTR("info"), 4);
    memcpy_P(&l1[35], PSTR("exit"), 4);
    l1[14] = kDelimiter;
    l1[29] = kDelimiter;
    l1[34] = kDelimiter;
    return;
  }

  // MODE_SAVE / MODE_LOAD
  memcpy_P(&l0[0], (mode_ == MODE_SAVE) ? PSTR("save") : PSTR("load"), 4);
  memcpy_P(&l0[6], PSTR("Cur:"), 4);
  RenderSlot(&l0[11], cur_slot_);
  l0[14] = kDelimiter;
  memcpy_P(&l0[16], PSTR("New:"), 4);
  RenderSlot(&l0[21], new_slot_);
  l0[24] = Snapshot::SlotOccupied(new_slot_) ? '*' : ' ';

  memcpy_P(&l1[30], PSTR("ok"), 2);
  memcpy_P(&l1[35], PSTR("back"), 4);
  l1[34] = kDelimiter;
}

/* static */
void SystemPage::UpdateLeds() {
  if (mode_ == MODE_MENU) {
    leds.set_pixel(LED_1, 0xf0);
    leds.set_pixel(LED_4, 0xf0);
    leds.set_pixel(LED_7, 0x0f);
    leds.set_pixel(LED_8, 0xf0);
  } else {
    leds.set_pixel(LED_7, 0xf0);
    leds.set_pixel(LED_8, 0x0f);
  }
}

}  // namespace ambika
