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

/// \file webserveractioncombo.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Implementation of the WebServerActionCombo class.
///
/// \dep
/// - <QVariant>
/// - <QIcon>
/// - types.h
/// - qtmetatypes.h
///
/// \par Changes
/// - (2018-03) First release.

#include "webserveractioncombo.h"

#include <QVariant>
#include <QIcon>

#include "types.h"
#include "qtmetatypes.h"
#include "display_strings.h"


namespace Anansi {


	WebServerActionCombo::WebServerActionCombo(QWidget * parent)
	: QComboBox(parent) {
		QComboBox::addItem(QIcon::fromTheme("trash-empty", QIcon(":/icons/webserveractions/ignore")), displayString(WebServerAction::Ignore), QVariant::fromValue(WebServerAction::Ignore));
		QComboBox::addItem(QIcon::fromTheme("dialog-ok", QIcon(":/icons/webserveractions/serve")), displayString(WebServerAction::Serve), QVariant::fromValue(WebServerAction::Serve));
		QComboBox::addItem(QIcon::fromTheme("system-run", QIcon(":/icons/webserveractions/cgi")), displayString(WebServerAction::CGI), QVariant::fromValue(WebServerAction::CGI));
		QComboBox::addItem(QIcon::fromTheme("error", QIcon(":/icons/webserveractions/forbid")), displayString(WebServerAction::Forbid), QVariant::fromValue(WebServerAction::Forbid));
		setToolTip(tr("<p>Choose what to do with requests of this type.</p>"));

		connect(this, qOverload<int>(&QComboBox::currentIndexChanged), [this](int) {
			Q_EMIT webServerActionChanged(webServerAction());
		});
	}


	WebServerAction WebServerActionCombo::webServerAction() {
		return currentData().value<WebServerAction>();
	}


	void WebServerActionCombo::setWebServerAction(WebServerAction action) {
		setCurrentIndex(findData(QVariant::fromValue(action)));
	}


}  // namespace Anansi
