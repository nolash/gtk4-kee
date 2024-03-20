#ifndef _UI_H
#define _UI_H

#include <gtk/gtk.h>

enum KeeActScan {
	KEE_ACT_SCAN_QR = 1,
	KEE_ACT_SCAN_FILE,
	KEE_ACT_SCAN_TEXT
};

#include "kee-uicontext.h"

void ui_build(GtkApplication *app, KeeUicontext *uctx);
void ui_handle_scan(KeeUicontext *uctx);
void ui_handle_unlock(KeeUicontext *uctx, gpointer user_data);

#endif // _UI_H
