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

		// ip addresses and policies from the list
		QString ipAddress(int idx) const;
		Configuration::ConnectionPolicy connectionPolicy(int idx) const;
		int selectedIpAddressIndex() const;
		QString selectedIpAddress() const;
		Configuration::ConnectionPolicy selectedIpAddressConnectionPolicy() const;
		int ipAddressCount() const;

		// IP address and policy in text edit/combo
		QString currentIpAddress() const;
		Configuration::ConnectionPolicy currentIpAddressConnectionPolicy() const;

		Configuration::ConnectionPolicy defaultConnectionPolicy() const;

	public Q_SLOTS:
		// select from list
		void selectIpAddress(const QString & addr);
		void selectIpAddress(int idx);

		// set content of text edit/combo
		void setCurrentIpAddress(const QString & addr);
		void setCurrentIpAddressConnectionPolicy(Configuration::ConnectionPolicy policy);

		void setDefaultConnectionPolicy(Configuration::ConnectionPolicy policy);

		void clearAllConnectionPolicies();
		void setIpAddressConnectionPolicy(const QString & addr, Configuration::ConnectionPolicy policy);

	Q_SIGNALS:
		void defaultConnectionPolicyChanged(Configuration::ConnectionPolicy policy);
		void ipAddressSelected(int idx);
		void ipAddressSelected(const QString & addr);
		void ipAddressRemoved(const QString & addr);
		void currentIpAddressChanged(const QString & addr);
		void currentIpAddressConnectionPolicyChanged(Configuration::ConnectionPolicy policy);
		void ipAddressConnectionPolicySet(const QString & addr, Configuration::ConnectionPolicy policy);

	private:
		IpAddressConnectionPolicyTreeItem * selectedIpAddressItem() const;

		std::unique_ptr<Ui::AccessControlWidget> m_ui;
	};


}  // namespace EquitWebServer
#endif  // EQUITWEBSERVER_ACCESSCONTROLWIDGET_H