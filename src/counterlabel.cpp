/*
 * Copyright 2015 - 2018 Darren Edale
 *
 * This file is part of Anansi web server.
 *
 * Anansi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Anansi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Anansi. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file counterlabel.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the CounterLabel class for the Equit library.
///
/// \dep
/// - counterlabel.h
/// - <QContextMenuEvent>
/// - <QIcon>
/// - <QMenu>
///
/// \par Changes
/// - (2018-03) First release.

#include "counterlabel.h"

#include <QContextMenuEvent>
#include <QIcon>
#include <QMenu>


namespace Equit {


	static constexpr const int DefaultCount = 0;


	CounterLabel::CounterLabel(QWidget * parent)
	: CounterLabel(QStringLiteral("%1"), DefaultCount, parent) {
	}


	CounterLabel::CounterLabel(const QString & tmplate, int count, QWidget * parent)
	: QLabel(parent),
	  m_template(tmplate),
	  m_count(count) {
		refresh();
	}


	void CounterLabel::refresh() {
		setText(displayTemplate().arg(count()));
	}


	void CounterLabel::contextMenuEvent(QContextMenuEvent * ev) {
		QMenu myMenu;
		myMenu.addAction(QIcon::fromTheme("clear"), tr("Reset counter"), this, &CounterLabel::reset);
		myMenu.exec(ev->globalPos());
	}


	void CounterLabel::setDisplayTemplate(const QString & tplate) {
		m_template = tplate;
		refresh();
	}


	void CounterLabel::setCount(int count) {
		m_count = count;
		refresh();
	}


}  // namespace Equit
