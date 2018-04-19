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

#include <QMainWindow>

#include "inlinenotificationwidget.h"

class QString;
class QVBoxLayout;

namespace Anansi {

	using NotificationType = Equit::InlineNotificationWidget::NotificationType;

	class WindowBase : public QMainWindow {
		Q_OBJECT

	public:
		static const int DefaultNotificationTimeout = 5000;

		explicit WindowBase(QWidget * parent = nullptr);
		WindowBase(const WindowBase &) = delete;
		WindowBase(WindowBase &&) = delete;
		void operator=(const WindowBase &) = delete;
		void operator=(WindowBase &&) = delete;
		~WindowBase() override;

		void showTransientInlineNotification(const QString & title, const QString & msg, NotificationType type, int = DefaultNotificationTimeout);

		inline void showTransientInlineNotification(const QString & title, const QString & msg, int timeout = DefaultNotificationTimeout) {
			showTransientInlineNotification(title, msg, NotificationType::Message, timeout);
		}

		inline void showTransientInlineNotification(const QString & msg, NotificationType type, int timeout = DefaultNotificationTimeout) {
			showTransientInlineNotification({}, msg, type, timeout);
		}

		inline void showTransientInlineNotification(const QString & msg, int timeout = DefaultNotificationTimeout) {
			showTransientInlineNotification({}, msg, NotificationType::Message, timeout);
		}

		void showInlineNotification(const QString & title, const QString & msg, const NotificationType type = NotificationType::Message);

		inline void showInlineNotification(const QString & msg, const NotificationType type = NotificationType::Message) {
			showInlineNotification({}, msg, type);
		}

		inline QWidget * centralWidget() const noexcept {
			return m_centralWidget;
		}

		void setCentralWidget(QWidget * widget);

	private:
		void disposeCentralWidget();

		QVBoxLayout * m_layout;
		QWidget * m_centralWidget;
	};

}  // namespace Anansi

#endif  // ANANSI_WINDOWBASE_H
