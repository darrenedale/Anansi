/// \file accesscontrolwidget.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Implementation of the AccessControlWidget class.
///
/// \dep
/// - <QApplication>
/// - accesslogtreeitem.h
///
/// \par Changes
/// - (2018-02) first version of this file.
#include "accesslogtreeitem.h"

#include <QApplication>


namespace EquitWebServer {


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
	AccessLogTreeItem::AccessLogTreeItem(const QString & addr, quint16 port, const QString & resource, WebServerAction action)
	: QTreeWidgetItem(ItemType) {
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
	AccessLogTreeItem::AccessLogTreeItem(const QString & addr, quint16 port, ConnectionPolicy policy)
	: QTreeWidgetItem(ItemType) {
		setIpAddress(addr);
		setPort(port);
		setConnectionPolicy(policy);
	}


	/// \brief Set the item's client IP address.
	///
	/// \param addr The IP address.
	void AccessLogTreeItem::setIpAddress(const QString & addr) {
		setText(0, addr);
	}


	/// \brief Set the item's client port.
	///
	/// \param port The port.
	void AccessLogTreeItem::setPort(quint16 port) {
		setText(1, QString::number(port));
	}


	/// \brief Set the item's requested resource.
	///
	/// \param resource The resource.
	///
	/// The resource is only of relevance to log items representing the action taken as a result
	/// of a request for a resource. Nonetheless, the method can be used on any item and will
	/// display the provided text in the appropriate column.
	void AccessLogTreeItem::setResource(const QString & resource) {
		setText(2, resource);
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
				setText(3, QApplication::QApplication::tr("Ignored"));
				break;

			case WebServerAction::Serve:
				setText(3, QApplication::tr("Served"));
				break;

			case WebServerAction::Forbid:
				setText(3, QApplication::tr("Forbidden, not found, or CGI failed"));
				break;

			case WebServerAction::CGI:
				setText(3, QApplication::tr("Executed through CGI"));
				break;
		}

		setIcon(3, {});
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
				setText(3, QApplication::QApplication::tr("No Connection Policy"));
				setIcon(3, {});
				break;

			case ConnectionPolicy::Reject:
				setText(3, QApplication::tr("Rejected"));
				setIcon(3, QIcon(QStringLiteral(":/icons/connectionpolicies/reject")));
				break;

			case ConnectionPolicy::Accept:
				setText(3, QApplication::tr("Accepted"));
				setIcon(3, QIcon(QStringLiteral(":/icons/connectionpolicies/accept")));
				break;
		}
	}


}  // namespace EquitWebServer
