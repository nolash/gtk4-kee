#ifndef _GTK_KEE_IMPORT_H
#define _GTK_KEE_IMPORT_H

#include <glib-object.h>

#include "kee-menu.h"

G_BEGIN_DECLS

#define KEE_TYPE_IMPORT kee_import_get_type()
G_DECLARE_FINAL_TYPE(KeeImport, kee_import, KEE, IMPORT, GtkBox)

#define KEE_ACT_SCAN_QR "import_scan"
#define KEE_ACT_SCAN_FILE "import_file"
#define KEE_ACT_SCAN_TEXT "import_text"


enum KEE_IMPORT_PROPS {
	KEE_P_IMPORT_WIN = 1,
	KEE_N_IMPORT_PROPS,
};

enum KEE_IMPORT_SIGS {
	KEE_S_IMPORT_SCAN_CHANGE,
	KEE_S_IMPORT_DATA,
	KEE_N_IMPORT_SIGS,
};

KeeImport* kee_import_new(KeeMenu *win);
int kee_import_refresh(KeeImport *im);
GListModel* kee_import_get_camera_list(KeeImport *o);
int kee_import_scanchange(KeeImport *o, const char *device);
GtkStack* kee_import_get_stack(KeeImport *o);

G_END_DECLS

#endif //_GTK_KEE_IMPORT_H
