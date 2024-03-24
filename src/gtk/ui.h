#ifndef _UI_H
#define _UI_H

#include <gtk/gtk.h>
//#include "context.h"
#include "kee-entry-store.h"


//void ui_build(GtkApplication *app, KeeUicontext *uctx);
//void ui_build(GtkApplication *app, struct kee_context *ctx);
void ui_build(GtkApplication *app, KeeEntryStore *store);

#endif // _UI_H
