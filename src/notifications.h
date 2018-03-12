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

#ifndef ANANSI_NOTIFICATIONS_H
#define ANANSI_NOTIFICATIONS_H

#include <QApplication>
#include <QMessageBox>

#include "window.h"


namespace Anansi {


	/// type alias for a static QMessageBox::* funciton for showing an error/warning/information
	/// dialogue. (could use qOverload<>() but given we'll be using this sig multiple times, an
	/// alias with a static cast is quicker to work with)
	using QMessageFunction = QMessageBox::StandardButton (*)(QWidget *, const QString &, const QString &, QMessageBox::StandardButtons, QMessageBox::StandardButton);


	/// select the appropriate QMessageBox dialogue function for a notification type
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


	/// \brief Show a notification to the user.
	///
	/// \param widget The widget whose window should show the notification.
	/// \param msg The message to show.
	/// \param type The type of notification.
	///
	/// If the QWidget provided is not `nullptr`, and its `window()` is a
	/// `[Window](\ref Window)` subclass, an inline notification is shown in that
	/// window; otherwise a standard QMessageBox is used. `qMessageBoxFunction()` is
	/// used to ensure that the appropriate type of dialogue is created for the
	/// NotificationType.
	///
	/// The default type of notification is a message.
	inline void showNotification(QWidget * widget, const QString & msg, NotificationType type = NotificationType::Message) {
		auto * win = qobject_cast<Window *>(widget->window());

		if(win) {
			win->showInlineNotification(msg, type);
		}
		else {
			qMessageBoxFunction(type)(win, QApplication::applicationDisplayName(), msg, QMessageBox::Close, {});
		}
	};


	/// \brief Show a transient notification to the user.
	///
	/// \param widget The widget whose window should show the notification.
	/// \param msg The message to show.
	/// \param type The type of notification.
	/// \param timeout The duration for the notification, in ms.
	///
	/// If the QWidget provided is not `nullptr`, and its `window()` is a
	/// `[Window](\ref Window)` subclass, an inline notification is shown in that
	/// window; otherwise a standard QMessageBox is used. `qMessageBoxFunction()` is
	/// used to ensure that the appropriate type of dialogue is created for the
	/// NotificationType. If a dialogue is created, it does not time out
	/// automatically and must be dismissed manually.
	///
	/// The default type of notification is a message.
	inline void showTransientNotification(QWidget * widget, const QString & msg, NotificationType type = NotificationType::Message, int timeout = 5000) {
		auto * win = qobject_cast<Window *>(widget->window());

		if(win) {
			win->showTransientInlineNotification(msg, type, timeout);
		}
		else {
			qMessageBoxFunction(type)(win, QApplication::applicationDisplayName(), msg, QMessageBox::Close, {});
		}
	}


}  // namespace Anansi

#endif  // ANANSI_NOTIFICATIONS_H
