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

/// \file windowbase.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the WindowBase base class for Anansi.
///
/// \dep
/// - <QMainWindow>
/// - inlinenotificationwidget.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_WINDOWBASE_H
#define ANANSI_WINDOWBASE_H

#include <deque>

#include <QMainWindow>

#include "inlinenotificationwidget.h"

class QString;
class QVBoxLayout;

namespace Anansi {

	using NotificationType = Equit::InlineNotificationWidget::NotificationType;

	class WindowBase : public QMainWindow {
		Q_OBJECT

	public:
		enum class NotificationDisplayPolicy {
			Simultaneous = 0,  // display multiple messages at same time
			Queue,				 // display subsequent message only after previous message closed
			Replace,				 // display subsequent message immediately, forcing previous message to close
			Ignore,				 // never display any notifications
		};

		static const int DefaultNotificationTimeout = 5000;

		explicit WindowBase(QWidget * parent = nullptr);
		WindowBase(const WindowBase &) = delete;
		WindowBase(WindowBase &&) = delete;
		void operator=(const WindowBase &) = delete;
		void operator=(WindowBase &&) = delete;
		~WindowBase() override;

		void setNotificationDisplayPolicy(NotificationDisplayPolicy policy);

		NotificationDisplayPolicy notificationDisplayPolicy() const {
			return m_notificationDisplayPolicy;
		}

		void showTransientInlineNotification(const QString & title, const QString & msg, NotificationType type, int timeout = DefaultNotificationTimeout) {
			showNotificationImplementation(title, msg, type, timeout);
		}

		inline void showTransientInlineNotification(const QString & title, const QString & msg, int timeout = DefaultNotificationTimeout) {
			showNotificationImplementation(title, msg, NotificationType::Message, timeout);
		}

		inline void showTransientInlineNotification(const QString & msg, NotificationType type, int timeout = DefaultNotificationTimeout) {
			showNotificationImplementation({}, msg, type, timeout);
		}

		inline void showTransientInlineNotification(const QString & msg, int timeout = DefaultNotificationTimeout) {
			showNotificationImplementation({}, msg, NotificationType::Message, timeout);
		}

		void showInlineNotification(const QString & title, const QString & msg, const NotificationType type = NotificationType::Message) {
			showNotificationImplementation(title, msg, type);
		}

		inline void showInlineNotification(const QString & msg, const NotificationType type = NotificationType::Message) {
			showInlineNotification({}, msg, type);
		}

		bool hasVisibleNotifications() const;

	public Q_SLOTS:
		void closeAllNotifications();

		inline QWidget * centralWidget() const noexcept {
			return m_centralWidget;
		}

		void setCentralWidget(QWidget * widget);

	private:
		struct NotificationDetails {
			const NotificationType type;
			const QString title;
			const QString message;
			const std::optional<int> timeout;
		};

		void disposeCentralWidget();

		Equit::InlineNotificationWidget * createNotificationWidget(const QString & title, const QString & msg, NotificationType type, const std::optional<int> & timeout = {}) const;
		void showNextQueuedNotification();
		void showNotificationImplementation(const QString & title, const QString & msg, const NotificationType type, const std::optional<int> & timeout = {});

		NotificationDisplayPolicy m_notificationDisplayPolicy;
		std::deque<NotificationDetails> m_notificiationQueue;
		QVBoxLayout * m_layout;
		QWidget * m_centralWidget;
	};

}  // namespace Anansi

#endif  // ANANSI_WINDOWBASE_H
