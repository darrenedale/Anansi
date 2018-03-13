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

/// \file notifications.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of functions to handle user notifications for Anansi.
///
/// \dep
/// - <QApplication>
/// - <QMessageBox>
/// - window.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_NOTIFICATIONS_H
#define ANANSI_NOTIFICATIONS_H

#include <QApplication>
#include <QMessageBox>

#include "windowbase.h"


namespace Anansi {


	namespace Notifications {
		static constexpr const int DefaultTimeout = 5000;
	}


	// could use qOverload<>() but given we'll be using this sig multiple times, an
	// alias with a static cast is quicker to work with
	using QMessageFunction = QMessageBox::StandardButton (*)(QWidget *, const QString &, const QString &, QMessageBox::StandardButtons, QMessageBox::StandardButton);


	inline constexpr QMessageFunction qMessageBoxFunction(NotificationType type) {
		switch(type) {
			case NotificationType::Warning:
				return static_cast<QMessageFunction>(&QMessageBox::warning);

			case NotificationType::Error:
				return static_cast<QMessageFunction>(&QMessageBox::critical);

			case NotificationType::Message:
				return static_cast<QMessageFunction>(&QMessageBox::information);

			case NotificationType::Question:
				return static_cast<QMessageFunction>(&QMessageBox::question);
		}

		return static_cast<QMessageFunction>(&QMessageBox::information);
	}


	inline void showNotification(QWidget * widget, const QString & msg, NotificationType type = NotificationType::Message) {
		WindowBase * win = nullptr;

		if(widget) {
			win = qobject_cast<WindowBase *>(widget->window());
		}

		if(win) {
			win->showInlineNotification(msg, type);
		}
		else {
			qMessageBoxFunction(type)(widget, QApplication::applicationDisplayName(), msg, QMessageBox::Close, {});
		}
	};


	inline void showTransientNotification(QWidget * widget, const QString & msg, NotificationType type = NotificationType::Message, int timeout = Notifications::DefaultTimeout) {
		WindowBase * win = nullptr;

		if(widget) {
			win = qobject_cast<WindowBase *>(widget->window());
		}

		if(win) {
			win->showTransientInlineNotification(msg, type, timeout);
		}
		else {
			qMessageBoxFunction(type)(widget, QApplication::applicationDisplayName(), msg, QMessageBox::Close, {});
		}
	}


}  // namespace Anansi

#endif  // ANANSI_NOTIFICATIONS_H
