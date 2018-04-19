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

/// \file windowbase.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the WindowBase base class for Anansi.
///
/// \dep
/// - windowbase.h
/// - <iostream>
/// - <QString>
/// - <QVBoxLayout>
/// - <QWidget>
/// - <QTimer>
///
/// \par Changes
/// - (2018-03) First release.

#include "windowbase.h"

#include <iostream>

#include <QString>
#include <QVBoxLayout>
#include <QWidget>
#include <QTimer>


namespace Anansi {


	using Equit::InlineNotificationWidget;


	WindowBase::WindowBase(QWidget * parent)
	: QMainWindow(parent),
	  m_layout(new QVBoxLayout),
	  m_centralWidget(nullptr) {
		auto * container = new QWidget;
		container->setLayout(m_layout);
		QMainWindow::setCentralWidget(container);
	}


	WindowBase::~WindowBase() {
		disposeCentralWidget();
	};


	void WindowBase::showTransientInlineNotification(const QString & title, const QString & msg, NotificationType type, int timeout) {
		if(100 > timeout) {
			timeout = 100;
		}

		auto * notificationWidget = new InlineNotificationWidget(type, msg, this);
		notificationWidget->setTitle(title);
		m_layout->insertWidget(0, notificationWidget, 0);
		connect(notificationWidget, &InlineNotificationWidget::closed, notificationWidget, &QObject::deleteLater);
		QTimer::singleShot(timeout, notificationWidget, &QWidget::hide);
	}


	void WindowBase::showInlineNotification(const QString & title, const QString & msg, const NotificationType type) {
		auto * notification = new InlineNotificationWidget(type, msg, this);
		notification->setTitle(title);
		m_layout->insertWidget(0, notification, 0);
		connect(notification, &InlineNotificationWidget::closed, notification, &QObject::deleteLater);
	}


	void WindowBase::setCentralWidget(QWidget * widget) {
		disposeCentralWidget();
		m_centralWidget = widget;

		if(widget) {
			m_layout->addWidget(m_centralWidget);
		}
	}


	void WindowBase::disposeCentralWidget() {
		if(m_centralWidget && m_centralWidget->parent() == m_layout) {
			m_layout->removeWidget(m_centralWidget);
			delete m_centralWidget;
		}

		m_centralWidget = nullptr;
	}

}  // namespace Anansi
