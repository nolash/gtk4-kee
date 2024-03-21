#ifndef _GTK_KEE_IMPORT_H
#define _GTK_KEE_IMPORT_H

#include <glib-object.h>

G_BEGIN_DECLS

#define KEE_TYPE_IMPORT kee_import_get_type()
G_DECLARE_FINAL_TYPE(KeeImport, kee_import, KEE, IMPORT, GtkBox)

enum KEE_IMPORT_SIGS {
	KEE_S_IMPORT_SCAN_CHANGE,
	KEE_N_IMPORT_SIGS,
};

KeeImport* kee_import_new(void);
int kee_import_refresh(KeeImport *im);
GListModel* kee_import_get_camera_list(KeeImport *o);
int kee_import_scanchange(KeeImport *o, const char *device);

G_END_DECLS

#endif //_GTK_KEE_IMPORT_H
