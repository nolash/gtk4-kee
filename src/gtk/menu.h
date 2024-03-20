#ifndef _KEE_GTK_MENU_H
#define _KEE_GTK_MENU_H

#include "kee-uicontext.h"

#define KEE_W_HEADER "header"
#define KEE_W_FOOTER "footer"

#define KEE_W_UI_MENU_QUICK_ADD "quick_add"
#define KEE_W_UI_MENU_ACT_IMPORT "act_import"

GtkWidget* header_setup(GtkApplication *gapp, KeeUicontext *uctx);

#endif // _KEE_GTK_MENU_H
