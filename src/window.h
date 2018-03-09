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

/// \file window.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the Window base class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_WINDOW_H
#define ANANSI_WINDOW_H

#include <QMainWindow>

#include "inlinenotificationwidget.h"

class QVBoxLayout;
class QWidget;
class QString;

namespace Anansi {

	using NotificationType = Equit::InlineNotificationWidget::NotificationType;

	class Window : public QMainWindow {
		Q_OBJECT
	public:
		explicit Window(QWidget * parent = nullptr);
		virtual ~Window() override;

		void showTransientInlineNotification(const QString & title, const QString & msg, NotificationType type, int timeout = 5000);

		void showTransientInlineNotification(const QString & title, const QString & msg, int timeout = 5000) {
			showTransientInlineNotification(title, msg, NotificationType::Message, timeout);
		}

		inline void showTransientInlineNotification(const QString & msg, NotificationType type, int timeout = 5000) {
			showTransientInlineNotification({}, msg, type, timeout);
		}

		inline void showTransientInlineNotification(const QString & msg, int timeout = 5000) {
			showTransientInlineNotification({}, msg, NotificationType::Message, timeout);
		}

		void showInlineNotification(const QString & title, const QString & msg, const NotificationType type = NotificationType::Message);

		inline void showInlineNotification(const QString & msg, const NotificationType type = NotificationType::Message) {
			showInlineNotification({}, msg, type);
		}

		inline QWidget * centralWidget() const {
			return m_centralWidget;
		}

		void setCentralWidget(QWidget *);

	private:
		void disposeCentralWidget();

		QVBoxLayout * m_layout;
		QWidget * m_centralWidget;
	};

}  // namespace Anansi

#endif  // WINDOW_H
