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

/// \file accesslogtreeitem.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the AccessLogTreeItem class.
///
/// \dep
/// - accesslogtreeitem.h
/// - <QApplication>
/// - <QString>
/// - <QDateTime>
/// - <QIcon>
///
/// \par Changes
/// - (2018-03) First release.

#include "accesslogtreeitem.h"

#include <QApplication>
#include <QString>
#include <QDateTime>
#include <QIcon>


namespace Anansi {


	static constexpr const int AccessLogTreeItemType = QTreeWidgetItem::UserType + 9003;

	static constexpr const int TimestampColumnIndex = 0;
	static constexpr const int IpAddressColumnIndex = 1;
	static constexpr const int IpPortColumnIndex = 2;
	static constexpr const int ResourceColumnIndex = 3;
	static constexpr const int ActionColumnIndex = 4;


	AccessLogTreeItem::AccessLogTreeItem(const QDateTime & timestamp, const QString & addr, uint16_t port, const QString & resource, WebServerAction action)
	: QTreeWidgetItem(AccessLogTreeItemType) {
		setText(TimestampColumnIndex, timestamp.toString(Qt::RFC2822Date));
		setIpAddress(addr);
		setPort(port);
		setResource(resource);
		setAction(action);
	}


	AccessLogTreeItem::AccessLogTreeItem(const QDateTime & timestamp, const QString & addr, uint16_t port, ConnectionPolicy policy)
	: QTreeWidgetItem(AccessLogTreeItemType) {
		setText(TimestampColumnIndex, timestamp.toString(Qt::RFC2822Date));
		setIpAddress(addr);
		setPort(port);
		setResource(QApplication::tr("[http connection]"));
		setConnectionPolicy(policy);
	}


	void AccessLogTreeItem::setIpAddress(const QString & addr) {
		setText(IpAddressColumnIndex, addr);
	}


	void AccessLogTreeItem::setPort(uint16_t port) {
		setText(IpPortColumnIndex, QString::number(port));
	}


	void AccessLogTreeItem::setResource(const QString & resource) {
		setText(ResourceColumnIndex, resource);
	}


	void AccessLogTreeItem::setAction(WebServerAction action) {
		switch(action) {
			case WebServerAction::Ignore:
				setText(ActionColumnIndex, QApplication::QApplication::tr("Ignored"));
				break;

			case WebServerAction::Serve:
				setText(ActionColumnIndex, QApplication::tr("Served"));
				break;

			case WebServerAction::Forbid:
				setText(ActionColumnIndex, QApplication::tr("Forbidden, not found, or CGI failed"));
				break;

			case WebServerAction::CGI:
				setText(ActionColumnIndex, QApplication::tr("Executed through CGI"));
				break;
		}

		setIcon(ActionColumnIndex, {});
	}


	void AccessLogTreeItem::setConnectionPolicy(ConnectionPolicy policy) {
		switch(policy) {
			case ConnectionPolicy::None:
				setText(ActionColumnIndex, QApplication::QApplication::tr("No Connection Policy"));
				setIcon(ActionColumnIndex, QIcon(QStringLiteral(":/icons/connectionpolicies/nopolicy")));
				break;

			case ConnectionPolicy::Reject:
				setText(ActionColumnIndex, QApplication::tr("Rejected"));
				setIcon(ActionColumnIndex, QIcon::fromTheme(QStringLiteral("cards-block"), QIcon(QStringLiteral(":/icons/connectionpolicies/reject"))));
				break;

			case ConnectionPolicy::Accept:
				setText(ActionColumnIndex, QApplication::tr("Accepted"));
				setIcon(ActionColumnIndex, QIcon::fromTheme(QStringLiteral("dialog-ok-accept"), QIcon(QStringLiteral(":/icons/connectionpolicies/accept"))));
				break;
		}
	}


}  // namespace Anansi
