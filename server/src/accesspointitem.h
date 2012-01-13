#ifndef __ACCESSPOINTITEM_H__
#define __ACCESSPOINTITEM_H__

#include <glib.h>
#include <nm-access-point.h>
#include <libdbusmenu-glib/menuitem.h>
#include <stdlib.h>
#include <string.h>

G_BEGIN_DECLS


#define DBUSMENU_TYPE_ACCESSPOINTITEM (dbusmenu_accesspointitem_get_type ())
#define DBUSMENU_ACCESSPOINTITEM(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), DBUSMENU_TYPE_ACCESSPOINTITEM, DbusmenuAccesspointitem))
#define DBUSMENU_ACCESSPOINTITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), DBUSMENU_TYPE_ACCESSPOINTITEM, DbusmenuAccesspointitemClass))
#define DBUSMENU_IS_ACCESSPOINTITEM(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DBUSMENU_TYPE_ACCESSPOINTITEM))
#define DBUSMENU_IS_ACCESSPOINTITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DBUSMENU_TYPE_ACCESSPOINTITEM))
#define DBUSMENU_ACCESSPOINTITEM_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), DBUSMENU_TYPE_ACCESSPOINTITEM, DbusmenuAccesspointitemClass))

typedef struct _DbusmenuAccesspointitem DbusmenuAccesspointitem;
typedef struct _DbusmenuAccesspointitemClass DbusmenuAccesspointitemClass;
typedef struct _DbusmenuAccesspointitemPrivate DbusmenuAccesspointitemPrivate;

struct _DbusmenuAccesspointitem {
	DbusmenuMenuitem parent_instance;
	DbusmenuAccesspointitemPrivate * priv;
};

struct _DbusmenuAccesspointitemClass {
	DbusmenuMenuitemClass parent_class;
};

GType dbusmenu_accesspointitem_get_type (void) G_GNUC_CONST;
void dbusmenu_accesspointitem_bind_accesspoint (DbusmenuAccesspointitem *self, NMAccessPoint *ap);
DbusmenuAccesspointitem* dbusmenu_accesspointitem_new (void);
DbusmenuAccesspointitem* dbusmenu_accesspointitem_new_with_id (gint id);

G_END_DECLS

#endif
