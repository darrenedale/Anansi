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


namespace EquitWebServer {


	ConnectionCountLabel::ConnectionCountLabel(const QString & tplate, int c, QWidget * parent)
	: QLabel(parent),
	  m_template(tplate),
	  m_count(c) {
		refresh();
	}


	void ConnectionCountLabel::refresh() {
		setText(getTemplate().arg(count()));
	}


	void ConnectionCountLabel::contextMenuEvent(QContextMenuEvent * ev) {
		QMenu myMenu;
		myMenu.addAction(QIcon::fromTheme("clear", QIcon(":/icons/menu/connectioncountlabel/reset")), tr("Reset counter"), this, &ConnectionCountLabel::reset);
		myMenu.exec(ev->globalPos());
	}


	void ConnectionCountLabel::setTemplate(const QString & tplate) {
		m_template = tplate;
		refresh();
	}


	QString ConnectionCountLabel::getTemplate() const {
		return m_template;
	}


	void ConnectionCountLabel::reset() {
		setCount(0);
	}


	void ConnectionCountLabel::increment(int amount) {
		setCount(count() + amount);
	}


	void ConnectionCountLabel::decrement(int amount) {
		setCount(count() - amount);
	}


	void ConnectionCountLabel::setCount(int c) {
		m_count = c;
		refresh();
	}


	int ConnectionCountLabel::count() const {
		return m_count;
	}


}  // namespace EquitWebServer
