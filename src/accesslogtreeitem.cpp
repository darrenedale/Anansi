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
/// \version 0.9.9
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


	/// \class AccessLogTreeItem
	/// \brief A custom tree item to represent an entry in the access log.
	///
	/// Notionally, an item can be one of two types:
	/// - one representing an action taken in response to a request;
	/// - one representing a decision taken on whether or not to accept a connection.
	///
	/// A constructor is provided for each notional type for easy creation of new items
	/// with the appropriate content. For the first type, the client IP address, client
	/// port, resource requested and action taken can be set; for the second, the address
	/// and port can be set, along with the policy implemented for the connection attempt.
	/// There are no getters for any of these properties - objects of this class are not
	/// intended to be read, only to be displayed.
	///
	/// There is no internal distinction between the available types, and the methods
	/// related to each notional type can be used regardless of which constructor was used.


	/// \brief Create a new AccessLogTreeItem.
	///
	/// The item created by this constructor shows the action taken as a result of a request for a resource.
	///
	/// \param addr The client IP address.
	/// \param port The client port.
	/// \param resource The requested resource.
	/// \param action The action to show in the log item.
	AccessLogTreeItem::AccessLogTreeItem(const QDateTime & timestamp, const QString & addr, uint16_t port, const QString & resource, WebServerAction action)
	: QTreeWidgetItem(AccessLogTreeItemType) {
		setText(TimestampColumnIndex, timestamp.toString(Qt::RFC2822Date));
		setIpAddress(addr);
		setPort(port);
		setResource(resource);
		setAction(action);
	}


	/// \brief Create a new AccessLogTreeItem.
	///
	/// The item created by this constructor shows the policy determined for a connection attempt.
	///
	/// \param addr The client IP address.
	/// \param port The client port.
	/// \param policy The policy to show in the log item.
	AccessLogTreeItem::AccessLogTreeItem(const QDateTime & timestamp, const QString & addr, uint16_t port, ConnectionPolicy policy)
	: QTreeWidgetItem(AccessLogTreeItemType) {
		setText(TimestampColumnIndex, timestamp.toString(Qt::RFC2822Date));
		setIpAddress(addr);
		setPort(port);
		setResource(QApplication::tr("[http connection]"));
		setConnectionPolicy(policy);
	}


	/// \brief Set the item's client IP address.
	///
	/// \param addr The IP address.
	void AccessLogTreeItem::setIpAddress(const QString & addr) {
		setText(IpAddressColumnIndex, addr);
	}


	/// \brief Set the item's client port.
	///
	/// \param port The port.
	void AccessLogTreeItem::setPort(uint16_t port) {
		setText(IpPortColumnIndex, QString::number(port));
	}


	/// \brief Set the item's requested resource.
	///
	/// \param resource The resource.
	///
	/// The resource is only of relevance to log items representing the action taken as a result
	/// of a request for a resource. Nonetheless, the method can be used on any item and will
	/// display the provided text in the appropriate column.
	void AccessLogTreeItem::setResource(const QString & resource) {
		setText(ResourceColumnIndex, resource);
	}


	/// \brief Set the item's action taken.
	///
	/// \param action The action.
	///
	/// The action is only of relevance to log items representing the action taken as a result
	/// of a request for a resource. Nonetheless, the method can be used on any item and will
	/// display the provided action in the appropriate column. This will override any content
	/// displayed based on a connection policy.
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


	/// \brief Set the item's connection policy.
	///
	/// \param policy The policy.
	///
	/// The policy is only of relevance to log items representing the policy determined for
	/// a connection attempt. Nonetheless, the method can be used on any item and will display
	/// the provided policy in the appropriate column. This will override any content
	/// displayed based on an action taken.
	void AccessLogTreeItem::setConnectionPolicy(ConnectionPolicy policy) {
		switch(policy) {
			case ConnectionPolicy::None:
				setText(ActionColumnIndex, QApplication::QApplication::tr("No Connection Policy"));
				setIcon(ActionColumnIndex, {});
				break;

			case ConnectionPolicy::Reject:
				setText(ActionColumnIndex, QApplication::tr("Rejected"));
				setIcon(ActionColumnIndex, QIcon(QStringLiteral(":/icons/connectionpolicies/reject")));
				break;

			case ConnectionPolicy::Accept:
				setText(ActionColumnIndex, QApplication::tr("Accepted"));
				setIcon(ActionColumnIndex, QIcon(QStringLiteral(":/icons/connectionpolicies/accept")));
				break;
		}
	}


}  // namespace Anansi
