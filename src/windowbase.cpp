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

#include "eqassert.h"


namespace Anansi {


	using Equit::InlineNotificationWidget;


	WindowBase::WindowBase(QWidget * parent)
	: QMainWindow(parent),
	  m_notificationDisplayPolicy(NotificationDisplayPolicy::Simultaneous),
	  m_layout(new QVBoxLayout),
	  m_centralWidget(nullptr) {
		auto * container = new QWidget;
		container->setLayout(m_layout);
		QMainWindow::setCentralWidget(container);
	}


	WindowBase::~WindowBase() {
		disposeCentralWidget();
	}


	void WindowBase::setNotificationDisplayPolicy(WindowBase::NotificationDisplayPolicy policy) {
		if(policy == m_notificationDisplayPolicy) {
			return;
		}

		switch(policy) {
			case NotificationDisplayPolicy::Simultaneous:
				// if there is a queue, flush it
				break;

			case NotificationDisplayPolicy::Queue:
				// existing displayed notifications are left in place
				break;

			case NotificationDisplayPolicy::Replace:
				// existing displayed notifications are left in place
				break;

			case NotificationDisplayPolicy::Ignore:
				// hide all notifications and empty queue
				m_notificiationQueue.clear();
				closeAllNotifications();
				break;
		}

		m_notificationDisplayPolicy = policy;
	}


	void WindowBase::closeAllNotifications() {
		int count = m_layout->count();

		while(0 < count) {
			auto * layoutWidget = m_layout->itemAt(0)->widget();

			// in theory, central widget could be an InlineNotificationWidget instance so
			// we need this check so we don't remove it in error
			if(layoutWidget == m_centralWidget) {
				continue;
			}

			auto * notificationWidget = qobject_cast<InlineNotificationWidget *>(layoutWidget);
			eqAssert(notificationWidget, "expected InlineNotificationWidget in layout, found " << (layoutWidget ? layoutWidget->staticMetaObject.className() : "null object"));
			delete notificationWidget;
			--count;
		}
	};


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


	bool WindowBase::hasVisibleNotifications() const {
		int count = m_layout->count();

		for(auto idx = 0; idx < count; ++idx) {
			auto * layoutWidget = m_layout->itemAt(idx)->widget();

			// in theory, central widget could be an InlineNotificationWidget instance so
			// we need this check so we don't count it as a notification
			if(layoutWidget == m_centralWidget) {
				continue;
			}

			if(qobject_cast<InlineNotificationWidget *>(layoutWidget)) {
				return true;
			}
		}

		return false;
	}


	InlineNotificationWidget * WindowBase::createNotificationWidget(const QString & title, const QString & msg, NotificationType type, const std::optional<int> & timeout) const {
		auto * notificationWidget = new InlineNotificationWidget(type, msg);
		notificationWidget->setTitle(title);
		connect(notificationWidget, &InlineNotificationWidget::closed, this, &WindowBase::showNextQueuedNotification);
		m_layout->insertWidget(0, notificationWidget, 0);
		connect(notificationWidget, &InlineNotificationWidget::closed, notificationWidget, &QObject::deleteLater);

		if(timeout) {
			auto ms = timeout.value();

			if(100 > ms) {
				ms = 100;
			}

			QTimer::singleShot(ms, notificationWidget, &QWidget::hide);
		}

		return notificationWidget;
	}


	void WindowBase::showNextQueuedNotification() {
		if(m_notificiationQueue.empty()) {
			return;
		}

		const auto & notification = m_notificiationQueue.front();
		createNotificationWidget(notification.title, notification.message, notification.type, notification.timeout);
		m_notificiationQueue.pop_front();
	}


	void WindowBase::showNotificationImplementation(const QString & title, const QString & msg, const NotificationType type, const std::optional<int> & timeout) {
		switch(notificationDisplayPolicy()) {
			case NotificationDisplayPolicy::Ignore:
				return;

			case NotificationDisplayPolicy::Simultaneous:
				// nothing to do - just add the notification alongside any others
				break;

			case NotificationDisplayPolicy::Replace:
				closeAllNotifications();
				break;

			case NotificationDisplayPolicy::Queue:
				if(hasVisibleNotifications()) {
					m_notificiationQueue.push_back({type, title, msg, timeout});
					return;
				}
				break;
		}

		createNotificationWidget(title, msg, type, timeout);
	}

}  // namespace Anansi
