/*
 * PasswordMenu.h
 *
 *  Created on: 16 Sep 2013
 *      Author: pete
 */

#ifndef PASSWORDMENU_H_
#define PASSWORDMENU_H_

#include <QString>
#include <QScopedPointer>

class PasswordMenuPriv;

class PasswordMenu {
public:
	PasswordMenu(unsigned int requestId);

	virtual ~PasswordMenu();

	const QString & busName() const;

	const QString & password() const;

	const QString & actionPath() const;

	const QString & menuPath() const;

protected:
	QScopedPointer<PasswordMenuPriv> p;
};

#endif /* PASSWORDMENU_H_ */
