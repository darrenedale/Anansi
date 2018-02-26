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

#include <QMenu>

#include "types.h"
#include "server.h"
#include "iplineeditaction.h"
#include "serveripconnectionpolicymodel.h"
#include "ippolicydelegate.h"


Q_DECLARE_METATYPE(EquitWebServer::ConnectionPolicy);


namespace EquitWebServer {


	/// \brief Create a new AccessControlWidget
	///
	/// \param parent The parent widget.
	AccessControlWidget::AccessControlWidget(QWidget * parent)
	: QWidget(parent),
	  m_ui(std::make_unique<Ui::AccessControlWidget>()) {
		m_ui->setupUi(this);

		auto * addEntryMenu = new QMenu(this);
		auto * action = new IpLineEditAction(this);
		addEntryMenu->addAction(action);
		m_ui->add->setMenu(addEntryMenu);

		connect(m_ui->remove, &QPushButton::clicked, [this]() {
			const auto idx = m_ui->ipPolicyList->currentIndex();

			if(!idx.isValid()) {
				return;
			}

			const auto row = idx.row();
			const auto addr = m_model->index(row, ServerIpConnectionPolicyModel::IpAddressColumnIndex).data().value<QString>();
			//			const auto policy = m_model->index(row, ServerIpConnectionPolicyModel::PolicyColumnIndex).data().value<ConnectionPolicy>();

			if(m_model->removeRows(idx.row(), 1, {})) {
				Q_EMIT ipAddressRemoved(addr);
			}
		});

		connect(action, &IpLineEditAction::addIpAddressClicked, [this](const QString & addr) {
			const auto idx = m_model->addIpAddress(addr, m_ui->defaultPolicy->connectionPolicy());

			if(!idx.isValid()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to add IP address \"" << qPrintable(addr) << "\" with policy = " << enumeratorString(m_ui->defaultPolicy->connectionPolicy()) << " to IP policy list. is it already present?\n";
				return;
			}

			m_ui->ipPolicyList->edit(idx);
		});

		connect(m_ui->defaultPolicy, &ConnectionPolicyCombo::connectionPolicyChanged, this, &AccessControlWidget::defaultConnectionPolicyChanged);

		m_ui->ipPolicyList->setItemDelegateForColumn(ServerIpConnectionPolicyModel::PolicyColumnIndex, new IpPolicyDelegate(this));
	}


	AccessControlWidget::AccessControlWidget(Server * server, QWidget * parent)
	: AccessControlWidget(parent) {
		setServer(server);
	}


	void AccessControlWidget::setServer(Server * server) {
		QSignalBlocker block(this);

		if(!server) {
			m_model.reset(nullptr);
			m_ui->defaultPolicy->setConnectionPolicy(ConnectionPolicy::None);
		}
		else {
			m_model = std::make_unique<ServerIpConnectionPolicyModel>(server);
			m_ui->defaultPolicy->setConnectionPolicy(server->configuration().defaultConnectionPolicy());

			connect(m_model.get(), &ServerIpConnectionPolicyModel::policyChanged, this, &AccessControlWidget::ipAddressConnectionPolicySet);
		}

		m_ui->ipPolicyList->setModel(m_model.get());
	}


	/// \brief Destroy the AccessControlWidget.
	AccessControlWidget::~AccessControlWidget() = default;


	/// \brief Fetch the currently selected IP address.
	///
	/// If no IP address is selected, an empty string is returned.
	///
	/// \return The selected IP address.
	QString AccessControlWidget::selectedIpAddress() const {
		const auto indices = m_ui->ipPolicyList->selectionModel()->selectedIndexes();

		if(0 == indices.size()) {
			return {};
		}

		return m_model->index(indices[0].row(), ServerIpConnectionPolicyModel::IpAddressColumnIndex).data().value<QString>();
	}


	/// \brief Fetch the connection policy for the currently selected IP address.
	///
	/// If no IP address is selected, the policy [None](\ref ConnectionPolicy::None)
	/// is returned.
	///
	/// \return The policy.
	ConnectionPolicy AccessControlWidget::selectedIpAddressConnectionPolicy() const {
		const auto indices = m_ui->ipPolicyList->selectionModel()->selectedIndexes();

		if(0 == indices.size()) {
			return {};
		}

		return m_model->index(indices[0].row(), ServerIpConnectionPolicyModel::PolicyColumnIndex).data().value<ConnectionPolicy>();
	}


	/// \brief Fetch the IP address currently displayed in the IP address edit widget.
	///
	/// The IP address widget does not currently validate its contents. This may change in
	/// future so this behaviour must not be relied upon.
	///
	/// \return The IP address.
	QString AccessControlWidget::currentIpAddress() const {
		return m_model->index(m_ui->ipPolicyList->currentIndex().row(), ServerIpConnectionPolicyModel::IpAddressColumnIndex).data().value<QString>();
	}


	/// \brief Fetch the connection policy currently selected in the policy combo box.
	///
	/// \return The policy.
	ConnectionPolicy AccessControlWidget::currentIpAddressConnectionPolicy() const {
		return m_model->index(m_ui->ipPolicyList->currentIndex().row(), ServerIpConnectionPolicyModel::PolicyColumnIndex).data().value<ConnectionPolicy>();
	}


	/// \brief Fetch the default connection policy displayed in the widget.
	///
	/// \return The policy.
	ConnectionPolicy AccessControlWidget::defaultConnectionPolicy() const {
		return m_ui->defaultPolicy->connectionPolicy();
	}


	/// \brief Clear all policies for all IP addresses.
	void AccessControlWidget::clearAllConnectionPolicies() {
		m_model->removeRows(0, m_model->rowCount(), {});
	}


	void AccessControlWidget::setDefaultConnectionPolicy(ConnectionPolicy policy) {
		m_ui->defaultPolicy->setConnectionPolicy(policy);
	}


	void AccessControlWidget::setIpAddressConnectionPolicy(const QString & addr, ConnectionPolicy policy) {
		auto idx = m_model->findIpAddressPolicy(addr);

		if(idx.isValid()) {
			m_model->setData(idx, QVariant::fromValue(policy), Qt::EditRole);
		}
		else {
			idx = m_model->addIpAddress(addr, policy);

			if(!idx.isValid()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to set connection policy for \"" << qPrintable(addr) << "\" to " << enumeratorString(policy) << "\n";
			}
			else {
				m_ui->ipPolicyList->edit(idx);
			}
		}
	}


}  // namespace EquitWebServer
