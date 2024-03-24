#ifndef _UI_H
#define _UI_H

#include <gtk/gtk.h>
#include "context.h"


//void ui_build(GtkApplication *app, KeeUicontext *uctx);
void ui_build(GtkApplication *app, struct kee_context *ctx);

#endif // _UI_H
