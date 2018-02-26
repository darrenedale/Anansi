/// \file accesscontrolwidget.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Implementation of the AccessControlWidget class.
///
/// \dep
/// - <iostream>
/// - accesscontrolwidget.h
/// - accesscontrolwidget.ui
/// - ipaddressconnectionpolicytreeitem.h
///
/// \par Changes
/// - (2018-02) first version of this file.
#include "accesscontrolwidget.h"
#include "ui_accesscontrolwidget.h"

#include <iostream>

#include "types.h"
#include "ipaddressconnectionpolicytreeitem.h"


Q_DECLARE_METATYPE(EquitWebServer::ConnectionPolicy);
Q_DECLARE_METATYPE(EquitWebServer::WebServerAction);


namespace EquitWebServer {


	/// \brief Create a new AccessControlWidget
	///
	/// \param parent The parent widget.
	AccessControlWidget::AccessControlWidget(QWidget * parent)
	: QWidget(parent),
	  m_ui(std::make_unique<Ui::AccessControlWidget>()) {
		m_ui->setupUi(this);

		connect(m_ui->defaultPolicy, &ConnectionPolicyCombo::connectionPolicyChanged, this, &AccessControlWidget::defaultConnectionPolicyChanged);
		connect(m_ui->ipPolicy, &ConnectionPolicyCombo::connectionPolicyChanged, this, &AccessControlWidget::currentIpAddressConnectionPolicyChanged);
		connect(m_ui->ipAddress, &QLineEdit::textChanged, this, &AccessControlWidget::currentIpAddressChanged);
		connect(m_ui->ipPolicyList, &IpListWidget::ipAddressRemoved, this, &AccessControlWidget::ipAddressRemoved);

		connect(m_ui->ipPolicyList, &IpListWidget::itemSelectionChanged, [this]() {
			for(int idx = m_ui->ipPolicyList->topLevelItemCount() - 1; 0 <= idx; --idx) {
				auto * item = m_ui->ipPolicyList->topLevelItem(idx);

				if(IpAddressConnectionPolicyTreeItem::ItemType == item->type() && item->isSelected()) {
					auto * myItem = static_cast<const IpAddressConnectionPolicyTreeItem *>(item);
					Q_EMIT ipAddressSelected(myItem->ipAddress());
					m_ui->ipAddress->setText(myItem->ipAddress());
					m_ui->ipPolicy->setConnectionPolicy(myItem->connectionPolicy());
					return;
				}
			}

			m_ui->ipAddress->setText({});
			m_ui->ipPolicy->setConnectionPolicy(ConnectionPolicy::None);
		});

		connect(m_ui->apply, &QToolButton::clicked, [this]() {
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: apply button clicked\n";
			setIpAddressConnectionPolicy(m_ui->ipAddress->text(), m_ui->ipPolicy->connectionPolicy());
		});
	}


	/// \brief Destroy the AccessControlWidget.
	AccessControlWidget::~AccessControlWidget() = default;


	/// \brief Fetch the currently selected IP address.
	///
	/// If no IP address is selected, an empty string is returned.
	///
	/// \return The selected IP address.
	QString AccessControlWidget::selectedIpAddress() const {
		auto * item = selectedIpAddressItem();

		if(!item) {
			return {};
		}

		return item->ipAddress();
	}


	/// \brief Fetch the connection policy for the currently selected IP address.
	///
	/// If no IP address is selected, the policy [None](\ref ConnectionPolicy::None)
	/// is returned.
	///
	/// \return The policy.
	ConnectionPolicy AccessControlWidget::selectedIpAddressConnectionPolicy() const {
		auto * item = selectedIpAddressItem();

		if(!item) {
			return ConnectionPolicy::None;
		}

		return item->connectionPolicy();
	}


	/// \brief Fetch the IP address currently displayed in the IP address edit widget.
	///
	/// The IP address widget does not currently validate its contents. This may change in
	/// future so this behaviour must not be relied upon.
	///
	/// \return The IP address.
	QString AccessControlWidget::currentIpAddress() const {
		return m_ui->ipAddress->text();
	}


	/// \brief Fetch the connection policy currently selected in the policy combo box.
	///
	/// \return The policy.
	ConnectionPolicy AccessControlWidget::currentIpAddressConnectionPolicy() const {
		return m_ui->ipPolicy->connectionPolicy();
	}


	/// \brief Fetch the default connection policy displayed in the widget.
	///
	/// \return The policy.
	ConnectionPolicy AccessControlWidget::defaultConnectionPolicy() const {
		return m_ui->defaultPolicy->connectionPolicy();
	}


	/// \brief Select a specified IP address.
	///
	/// \param addr The IP address to select.
	///
	/// If the IP address provided does not exist, the selection state of the list remains
	/// unmodified. This behaviour is very likely to change in the near future to ensure
	/// that the current selection is removed if the provided IP address is not present.
	///
	/// Successfully changing the IP address selection using this method also changes the
	/// IP address displayed in the edit widget.
	void AccessControlWidget::selectIpAddress(const QString & addr) {
		m_ui->ipPolicyList->clearSelection();

		for(int idx = m_ui->ipPolicyList->topLevelItemCount() - 1; 0 <= idx; ++idx) {
			auto * item = m_ui->ipPolicyList->topLevelItem(idx);

			if(IpAddressConnectionPolicyTreeItem::ItemType != item->type()) {
				continue;
			}

			if(static_cast<IpAddressConnectionPolicyTreeItem *>(item)->ipAddress() == addr) {
				item->setSelected(true);
			}
		}
	}


	/// \brief Set the content of the IP address edit widget.
	///
	/// \param addr The IP address.
	///
	/// Setting the IP address here has no impact on the selection state or content of
	/// the IP address list.
	void AccessControlWidget::setCurrentIpAddress(const QString & addr) {
		m_ui->ipAddress->setText(addr);
	}


	/// \brief Set the policy displayed in the IP address policy combo box.
	///
	/// \param policy The policy to select.
	///
	/// Setting the policy here has no impact on the content of the IP address list.
	void AccessControlWidget::setCurrentIpAddressConnectionPolicy(ConnectionPolicy policy) {
		m_ui->ipPolicy->setConnectionPolicy(policy);
	}


	/// \brief Set the default connection policy.
	///
	/// \param policy The policy to set.
	///
	/// The provided policy is displayed in the default connection policy combo box.
	void AccessControlWidget::setDefaultConnectionPolicy(ConnectionPolicy policy) {
		m_ui->defaultPolicy->setConnectionPolicy(policy);
	}


	/// \brief Clear all policies for all IP addresses.
	void AccessControlWidget::clearAllConnectionPolicies() {
		m_ui->ipPolicyList->clear();
	}


	/// \brief Set the connection policy for a specified IP address.
	///
	/// \param addr The IP address.
	/// \param policy The connection policy.
	///
	/// The connection policy displayed in the list for the provided IP address is changed to
	/// the provided policy. If the address is not already in the list it is added.
	void AccessControlWidget::setIpAddressConnectionPolicy(const QString & addr, ConnectionPolicy policy) {
		bool found = false;

		for(int idx = m_ui->ipPolicyList->topLevelItemCount() - 1; 0 <= idx; --idx) {
			auto * item = m_ui->ipPolicyList->topLevelItem(idx);

			if(IpAddressConnectionPolicyTreeItem::ItemType != item->type()) {
				continue;
			}

			auto * ipItem = static_cast<IpAddressConnectionPolicyTreeItem *>(item);

			if(ipItem->ipAddress() == addr) {
				found = true;
				ipItem->setConnectionPolicy(policy);
				Q_EMIT ipAddressConnectionPolicySet(addr, policy);
			}
		}

		if(!found) {
			m_ui->ipPolicyList->addTopLevelItem(new IpAddressConnectionPolicyTreeItem(addr, policy));
			Q_EMIT ipAddressConnectionPolicySet(addr, policy);
		}
	}


	/// \brief Fetch the currently selected item from the IP address list.
	///
	/// The class of the policy list is one that internally sets its selection style to single selection.
	/// Therefore, there should always be at most one selected item, and the first selected item encountered
	/// in the widget is returned. If there are no selected items, `nullptr` is returned.
	///
	/// \return A pointer to the selected item, or `nullptr` if no item is currently selected.
	IpAddressConnectionPolicyTreeItem * AccessControlWidget::selectedIpAddressItem() const {
		auto items = m_ui->ipPolicyList->selectedItems();

		if(0 < items.size() && IpAddressConnectionPolicyTreeItem::ItemType == items[0]->type()) {
			return static_cast<IpAddressConnectionPolicyTreeItem *>(items[0]);
		}

		auto * item = m_ui->ipPolicyList->currentItem();

		if(item && IpAddressConnectionPolicyTreeItem::ItemType == item->type()) {
			return static_cast<IpAddressConnectionPolicyTreeItem *>(item);
		}

		return nullptr;
	}


}  // namespace EquitWebServer
