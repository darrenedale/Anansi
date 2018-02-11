#include "accesscontrolwidget.h"
#include "ui_accesscontrolwidget.h"

#include <iostream>

#include "ipaddressconnectionpolicytreeitem.h"


namespace EquitWebServer {


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
					Q_EMIT ipAddressSelected(idx);
					m_ui->ipAddress->setText(myItem->ipAddress());
					m_ui->ipPolicy->setConnectionPolicy(myItem->connectionPolicy());
					return;
				}
			}

			m_ui->ipAddress->setText({});
			m_ui->ipPolicy->setConnectionPolicy(Configuration::NoConnectionPolicy);
		});

		connect(m_ui->apply, &QToolButton::clicked, [this]() {
			std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: apply button clicked\n";
			setIpAddressConnectionPolicy(m_ui->ipAddress->text(), m_ui->ipPolicy->connectionPolicy());
		});
	}


	QString AccessControlWidget::ipAddress(int idx) const {
		auto * item = m_ui->ipPolicyList->topLevelItem(idx);

		if(!item) {
			return {};
		}

		if(IpAddressConnectionPolicyTreeItem::ItemType != item->type()) {
			return {};
		}

		return static_cast<IpAddressConnectionPolicyTreeItem *>(item)->ipAddress();
	}


	Configuration::ConnectionPolicy AccessControlWidget::connectionPolicy(int idx) const {
		auto * item = m_ui->ipPolicyList->topLevelItem(idx);

		if(!item) {
			return {};
		}

		if(IpAddressConnectionPolicyTreeItem::ItemType != item->type()) {
			return {};
		}

		return static_cast<IpAddressConnectionPolicyTreeItem *>(item)->connectionPolicy();
	}


	int AccessControlWidget::selectedIpAddressIndex() const {
		auto * item = selectedIpAddressItem();

		if(!item) {
			return -1;
		}

		return m_ui->ipPolicyList->indexOfTopLevelItem(item);
	}


	QString AccessControlWidget::selectedIpAddress() const {
		auto * item = selectedIpAddressItem();

		if(!item) {
			return {};
		}

		return item->ipAddress();
	}


	Configuration::ConnectionPolicy AccessControlWidget::selectedIpAddressConnectionPolicy() const {
		auto * item = selectedIpAddressItem();

		if(!item) {
			return Configuration::NoConnectionPolicy;
		}

		return item->connectionPolicy();
	}


	QString AccessControlWidget::currentIpAddress() const {
		return m_ui->ipAddress->text();
	}


	Configuration::ConnectionPolicy AccessControlWidget::currentIpAddressConnectionPolicy() const {
		return m_ui->ipPolicy->connectionPolicy();
	}


	Configuration::ConnectionPolicy AccessControlWidget::defaultConnectionPolicy() const {
		return m_ui->defaultPolicy->connectionPolicy();
	}


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


	void AccessControlWidget::selectIpAddress(int idx) {
		m_ui->ipPolicyList->clearSelection();
		auto * item = m_ui->ipPolicyList->topLevelItem(idx);

		if(item) {
			item->setSelected(true);
		}
	}


	void AccessControlWidget::setCurrentIpAddress(const QString & addr) {
		m_ui->ipAddress->setText(addr);
	}


	void AccessControlWidget::setCurrentIpAddressConnectionPolicy(Configuration::ConnectionPolicy policy) {
		m_ui->ipPolicy->setConnectionPolicy(policy);
	}


	void AccessControlWidget::setDefaultConnectionPolicy(Configuration::ConnectionPolicy policy) {
		m_ui->defaultPolicy->setConnectionPolicy(policy);
	}


	void AccessControlWidget::clearAllConnectionPolicies() {
		m_ui->ipPolicyList->clear();
	}


	void AccessControlWidget::setIpAddressConnectionPolicy(const QString & addr, Configuration::ConnectionPolicy policy) {
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


	AccessControlWidget::~AccessControlWidget() = default;

}  // namespace EquitWebServer
