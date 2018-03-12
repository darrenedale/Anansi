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

/// \file accesscontrolwidget.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the AccessControlWidget class for Anansi.
///
/// \dep
/// - <memory>
/// - <QWidget>
/// - <QString>
/// - types.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_ACCESSCONTROLWIDGET_H
#define ANANSI_ACCESSCONTROLWIDGET_H

#include <memory>

#include <QWidget>
#include <QString>

#include "types.h"

namespace Anansi {

	class Server;
	class ServerIpConnectionPolicyModel;
	class IpPolicyDelegate;

	namespace Ui {
		class AccessControlWidget;
	}

	class AccessControlWidget : public QWidget {
		Q_OBJECT

	public:
		explicit AccessControlWidget(QWidget * parent = nullptr);
		explicit AccessControlWidget(Server * server, QWidget * parent = nullptr);
		virtual ~AccessControlWidget() override;

		void setServer(Server * server);

		// IP addresses and policies from the list
		QString selectedIpAddress() const;
		ConnectionPolicy selectedIpAddressConnectionPolicy() const;
		int ipAddressCount() const;

		// IP address and policy in text edit/combo
		QString currentIpAddress() const;
		ConnectionPolicy currentIpAddressConnectionPolicy() const;

		ConnectionPolicy defaultConnectionPolicy() const;

	public Q_SLOTS:
		void clearAllConnectionPolicies();
		void setDefaultConnectionPolicy(ConnectionPolicy policy);
		void setIpAddressConnectionPolicy(const QString & addr, ConnectionPolicy policy);

	Q_SIGNALS:
		void defaultConnectionPolicyChanged(ConnectionPolicy policy);
		void ipAddressRemoved(const QString & addr);
		void ipAddressConnectionPolicySet(const QString & addr, ConnectionPolicy policy);

	private Q_SLOTS:
		void onIpListSelectionChanged();

	private:
		std::unique_ptr<ServerIpConnectionPolicyModel> m_model;
		std::unique_ptr<IpPolicyDelegate> m_delegate;
		std::unique_ptr<Ui::AccessControlWidget> m_ui;
		Server * m_server;  // observed only
	};

}  // namespace Anansi

#endif  // ANANSI_ACCESSCONTROLWIDGET_H
