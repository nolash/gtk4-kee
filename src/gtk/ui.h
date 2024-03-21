#ifndef _UI_H
#define _UI_H

#include <gtk/gtk.h>
#include "kee-uicontext.h"
#include "kee-import.h"

#define KEE_ACT_SCAN_QR "import_scan"
#define KEE_ACT_SCAN_FILE "import_file"
#define KEE_ACT_SCAN_TEXT "import_text"


void ui_build(GtkApplication *app, KeeUicontext *uctx);
void ui_build_scan(GtkApplication *app, KeeImport *imp);
void ui_handle_unlock(KeeUicontext *uctx, gpointer user_data);

#endif // _UI_H
