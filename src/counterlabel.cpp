/// \file counterlabel.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Implementation of the CounterLabel class for EquitWebServer
///
/// \par Changes
/// - (2012-06-19) file documentation created.

#include "counterlabel.h"

#include <QContextMenuEvent>
#include <QMenu>


namespace EquitWebServer {


	CounterLabel::CounterLabel(QWidget * parent)
	: CounterLabel(QStringLiteral("%1"), 0, parent) {}


	CounterLabel::CounterLabel(const QString & tplate, int count, QWidget * parent)
	: QLabel(parent),
	  m_template(tplate),
	  m_count(count) {
		refresh();
	}


	void CounterLabel::refresh() {
		setText(getTemplate().arg(count()));
	}


	void CounterLabel::contextMenuEvent(QContextMenuEvent * ev) {
		QMenu myMenu;
		myMenu.addAction(QIcon::fromTheme("clear", QIcon(":/icons/menu/connectioncountlabel/reset")), tr("Reset counter"), this, &CounterLabel::reset);
		myMenu.exec(ev->globalPos());
	}


	void CounterLabel::setTemplate(const QString & tplate) {
		m_template = tplate;
		refresh();
	}


	void CounterLabel::setCount(int count) {
		m_count = count;
		refresh();
	}


}  // namespace EquitWebServer
