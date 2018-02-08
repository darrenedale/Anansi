/** \file ConnectionCountLabel.cpp
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Implementation of the ConnectionCountLabel class for EquitWebServer
  *
  * \todo
  * - decide on application license
  *
  * \par Changes
  * - (2012-06-19) file documentation created.
  *
  */

#include "ConnectionCountLabel.h"

#include <QContextMenuEvent>
#include <QMenu>


EquitWebServer::ConnectionCountLabel::ConnectionCountLabel(const QString & tplate, int c, QWidget * parent)
: QLabel(parent),
  m_template(tplate),
  m_count(c) {
	refresh();
}


void EquitWebServer::ConnectionCountLabel::refresh(void) {
	setText(getTemplate().arg(count()));
}


void EquitWebServer::ConnectionCountLabel::contextMenuEvent(QContextMenuEvent * ev) {
	QMenu myMenu;
	myMenu.addAction(QIcon::fromTheme("clear", QIcon(":/icons/menu/connectioncountlabel/reset")), tr("Reset counter"), this, SLOT(reset()));
	myMenu.exec(ev->globalPos());
}


void EquitWebServer::ConnectionCountLabel::setTemplate(const QString & tplate) {
	m_template = tplate;
	refresh();
}


QString EquitWebServer::ConnectionCountLabel::getTemplate(void) const {
	return m_template;
}


void EquitWebServer::ConnectionCountLabel::reset(void) {
	setCount(0);
}


void EquitWebServer::ConnectionCountLabel::increment(int amount) {
	setCount(count() + amount);
}


void EquitWebServer::ConnectionCountLabel::decrement(int amount) {
	setCount(count() - amount);
}


void EquitWebServer::ConnectionCountLabel::setCount(int c) {
	m_count = c;
	refresh();
}


int EquitWebServer::ConnectionCountLabel::count(void) const {
	return m_count;
}
