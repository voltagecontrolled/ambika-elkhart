#ifndef CONTROLLER_UI_PAGES_SYSTEM_PAGE_H_
#define CONTROLLER_UI_PAGES_SYSTEM_PAGE_H_

#include "controller/ui_pages/ui_page.h"

namespace ambika {

class SystemPage : public UiPage {
 public:
  SystemPage() { }

  static void OnInit(PageInfo* info);
  static uint8_t OnIncrement(int8_t increment);
  static uint8_t OnKey(uint8_t key);
  static void UpdateScreen();
  static void UpdateLeds();
  static void OnDialogClosed(uint8_t dialog_id, uint8_t return_value);

  static const prog_EventHandlers event_handlers_;

 private:
  enum Mode {
    MODE_MENU,
    MODE_SAVE,
    MODE_LOAD,
  };

  enum DialogId {
    DLG_OVERWRITE = 1,
    DLG_INFO_SAVED,
    DLG_INFO_LOADED,
    DLG_INFO_EMPTY,
    DLG_ERROR_FAILED,
  };

  static void EnterMode(Mode m);
  static void DoSave();
  static void DoLoad();
  static void ShowInfo(const prog_char* text);
  static void ShowError(const prog_char* text);
  static void RenderSlot(char* buf, uint8_t slot);  // writes 2 chars or "--"

  static uint8_t mode_;
  static uint8_t cur_slot_;   // 0xff = none
  static uint8_t new_slot_;
  static uint8_t pending_slot_;

  DISALLOW_COPY_AND_ASSIGN(SystemPage);
};

}  // namespace ambika

#endif  // CONTROLLER_UI_PAGES_SYSTEM_PAGE_H_
