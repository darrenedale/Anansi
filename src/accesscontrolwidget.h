#ifndef EQUITWEBSERVER_ACCESSCONTROLWIDGET_H
#define EQUITWEBSERVER_ACCESSCONTROLWIDGET_H

#include <memory>

#include <QWidget>

#include "configuration.h"

namespace EquitWebServer {

	class Server;
	class ServerIpConnectionPolicyModel;

	namespace Ui {
		class AccessControlWidget;
	}

	class AccessControlWidget : public QWidget {
		Q_OBJECT

	public:
		explicit AccessControlWidget(QWidget * parent = nullptr);
		explicit AccessControlWidget(Server * server, QWidget * parent = nullptr);
		virtual ~AccessControlWidget();

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
		std::unique_ptr<Ui::AccessControlWidget> m_ui;
		Server * m_server;  // observed only
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_ACCESSCONTROLWIDGET_H
