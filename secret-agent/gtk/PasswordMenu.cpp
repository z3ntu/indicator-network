/*
 * PasswordMenu.cpp
 *
 *  Created on: 16 Sep 2013
 *      Author: pete
 */

#include <gtk/PasswordMenu.h>
#include <gio/gio.h>

#include <QString>

static const QString PASSWORD_ACTION_PATH("/action/%1");
static const QString PASSWORD_MENU_PATH("/menu/%1");

class PasswordMenuPriv {
public:
	PasswordMenuPriv() :
			m_connection(g_bus_get_sync(G_BUS_TYPE_SESSION, NULL,
			NULL)), m_exportedActionGroupId(0), m_exportedMenuModelId(0) {
	}

	~PasswordMenuPriv() {
		g_object_unref(m_connection);
	}

	static void passwordChangedCallback(GAction *passwordAction,
			GVariant *variant, gpointer userData) {
		PasswordMenuPriv *self(reinterpret_cast<PasswordMenuPriv*>(userData));
		self->passwordChanged(variant);
	}

	void passwordChanged(GVariant *variant) {
		m_password = QString::fromUtf8(g_variant_get_string(variant, 0));
	}

	GDBusConnection *m_connection;

	QString m_busName;

	QString m_actionPath;

	QString m_menuPath;

	unsigned int m_exportedActionGroupId;

	unsigned int m_exportedMenuModelId;

	QString m_password;

};

PasswordMenu::PasswordMenu(unsigned int requestId) :
		p(new PasswordMenuPriv()) {
	p->m_busName = QString::fromUtf8(
			g_dbus_connection_get_unique_name(p->m_connection));

	p->m_actionPath = PASSWORD_ACTION_PATH.arg(requestId);
	p->m_menuPath = PASSWORD_MENU_PATH.arg(requestId);

	// menu
	GMenu *menu(g_menu_new());

	GMenuItem *passwordItem(
			g_menu_item_new("", "notifications.password"));
	g_menu_item_set_attribute_value(passwordItem, "x-canonical-type",
			g_variant_new_string("com.canonical.snapdecision.textfield"));
	g_menu_item_set_attribute_value(passwordItem, "x-echo-mode-password",
			g_variant_new_boolean(true));

	g_menu_append_item(menu, passwordItem);

	// actions
	GActionGroup *actions(G_ACTION_GROUP(g_simple_action_group_new()));
	GAction *passwordAction(G_ACTION(
			g_simple_action_new_stateful("password", G_VARIANT_TYPE_STRING,
					g_variant_new_string(""))));

	g_signal_connect(G_OBJECT(passwordAction), "change-state",
			G_CALLBACK(PasswordMenuPriv::passwordChangedCallback),
			reinterpret_cast<gpointer>(p.data()));

	g_action_map_add_action(G_ACTION_MAP(actions), passwordAction);

	p->m_exportedActionGroupId = g_dbus_connection_export_action_group(
			p->m_connection, p->m_actionPath.toUtf8().data(), actions, NULL);

	p->m_exportedMenuModelId = g_dbus_connection_export_menu_model(
			p->m_connection, p->m_menuPath.toUtf8().data(),
			G_MENU_MODEL(menu), NULL);

	g_object_unref(menu);
	g_object_unref(passwordItem);

	g_object_unref(actions);
	g_object_unref(passwordAction);
}

PasswordMenu::~PasswordMenu() {
	g_dbus_connection_unexport_action_group(p->m_connection,
			p->m_exportedActionGroupId);
	g_dbus_connection_unexport_menu_model(p->m_connection,
			p->m_exportedMenuModelId);
}

const QString & PasswordMenu::busName() const {
	return p->m_busName;
}

const QString & PasswordMenu::password() const {
	return p->m_password;
}

const QString & PasswordMenu::actionPath() const {
	return p->m_actionPath;
}

const QString & PasswordMenu::menuPath() const {
	return p->m_menuPath;
}
