#ifndef EQUITWEBSERVER_NOTIFICATIONS_H
#define EQUITWEBSERVER_NOTIFICATIONS_H

#include <QApplication>
#include <QMessageBox>

#include "window.h"


namespace EquitWebServer {


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
	///
	/// \todo equivalent for transient notifications
	inline void showNotification(QWidget * widget, const QString & msg, NotificationType type = NotificationType::Message) {
		auto * win = qobject_cast<Window *>(widget->window());

		if(win) {
			win->showInlineNotification(msg, NotificationType::Error);
		}
		else {
			qMessageBoxFunction(type)(win, QApplication::applicationDisplayName(), msg, QMessageBox::Close, {});
		}
	};


}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_NOTIFICATIONS_H
