#ifndef EQUITWEBSERVER_ACCESSCONTROLWIDGET_H
#define EQUITWEBSERVER_ACCESSCONTROLWIDGET_H

#include <memory>

#include <QWidget>

#include "configuration.h"

namespace EquitWebServer {

	class IpAddressConnectionPolicyTreeItem;

	namespace Ui {
		class AccessControlWidget;
	}

	class AccessControlWidget : public QWidget {
		Q_OBJECT

	public:
		explicit AccessControlWidget(QWidget * parent = nullptr);
		virtual ~AccessControlWidget();

		// IP addresses and policies from the list
		QString selectedIpAddress() const;
		ConnectionPolicy selectedIpAddressConnectionPolicy() const;
		int ipAddressCount() const;

		// IP address and policy in text edit/combo
		QString currentIpAddress() const;
		ConnectionPolicy currentIpAddressConnectionPolicy() const;

		ConnectionPolicy defaultConnectionPolicy() const;

	public Q_SLOTS:
		// select from list
		void selectIpAddress(const QString & addr);

		// set content of text edit/combo
		void setCurrentIpAddress(const QString & addr);
		void setCurrentIpAddressConnectionPolicy(ConnectionPolicy policy);

		void setDefaultConnectionPolicy(ConnectionPolicy policy);

		void clearAllConnectionPolicies();
		void setIpAddressConnectionPolicy(const QString & addr, ConnectionPolicy policy);

	Q_SIGNALS:
		void defaultConnectionPolicyChanged(ConnectionPolicy policy);
		void ipAddressSelected(const QString & addr);
		void ipAddressRemoved(const QString & addr);
		void currentIpAddressChanged(const QString & addr);
		void currentIpAddressConnectionPolicyChanged(ConnectionPolicy policy);
		void ipAddressConnectionPolicySet(const QString & addr, ConnectionPolicy policy);

	private:
		IpAddressConnectionPolicyTreeItem * selectedIpAddressItem() const;
		std::unique_ptr<Ui::AccessControlWidget> m_ui;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_ACCESSCONTROLWIDGET_H
