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

/// \file accesscontrolwidget.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the AccessControlWidget class.
///
/// \dep
/// - accesscontrolwidget.h
/// - accesscontrolwidget.ui
/// - <iostream>
/// - <QMenu>
/// - <QPushButton>
/// - <QMessageBox>
/// - types.h
/// - server.h
/// - window.h
/// - iplineeditaction.h
/// - serveripconnectionpolicymodel.h
/// - ippolicydelegate.h
/// - connectionpolicycombo.h
/// - notifications.h
/// - qtmetatypes.h
///
/// \par Changes
/// - (2018-03) First release.

#include "accesscontrolwidget.h"
#include "ui_accesscontrolwidget.h"

#include <iostream>

#include <QMenu>
#include <QPushButton>
#include <QMessageBox>

#include "types.h"
#include "server.h"
#include "window.h"
#include "iplineeditaction.h"
#include "serveripconnectionpolicymodel.h"
#include "ippolicydelegate.h"
#include "connectionpolicycombo.h"
#include "notifications.h"
#include "qtmetatypes.h"


namespace Anansi {


	/// \brief Create a new AccessControlWidget
	///
	/// \param parent The parent widget.
	AccessControlWidget::AccessControlWidget(QWidget * parent)
	: QWidget(parent),
	  m_model(nullptr),
	  m_delegate(std::make_unique<IpPolicyDelegate>()),
	  m_ui(std::make_unique<Ui::AccessControlWidget>()),
	  m_server(nullptr) {
		m_ui->setupUi(this);

		auto * addEntryMenu = new QMenu(this);
		auto * action = new IpLineEditAction(this);
		addEntryMenu->addAction(action);
		m_ui->add->setMenu(addEntryMenu);

		connect(addEntryMenu, &QMenu::aboutToShow, [action]() {
			action->lineEdit()->setFocus();
			action->lineEdit()->selectAll();
		});

		connect(m_ui->remove, &QPushButton::clicked, [this]() {
			Q_ASSERT_X(m_model, __PRETTY_FUNCTION__, "model must not be null");
			const auto idx = m_ui->ipPolicyList->currentIndex();

			if(!idx.isValid()) {
				return;
			}

			const auto row = idx.row();
			const auto addr = m_model->index(row, ServerIpConnectionPolicyModel::IpAddressColumnIndex).data().value<QString>();

			if(m_model->removeRows(idx.row(), 1, {})) {
				Q_EMIT ipAddressRemoved(addr);
			}
		});

		connect(action, &IpLineEditAction::addIpAddressClicked, [this, addEntryMenu, action](const QString & addr) {
			Q_ASSERT_X(m_model, __PRETTY_FUNCTION__, "model must not be null");
			const auto idx = m_model->addIpAddress(addr, m_ui->defaultPolicy->connectionPolicy());

			if(!idx.isValid()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to add IP address \"" << qPrintable(addr) << "\" with policy = " << enumeratorString(m_ui->defaultPolicy->connectionPolicy()) << " to IP policy list. is it already present?\n";
				showNotification(this, tr("<p>A new policy for the IP address <strong>%1</strong> could not be added.</p><p><small>Perhaps this IP address already has a policy assigned?</small></p>").arg(addr), NotificationType::Error);
				action->lineEdit()->setFocus();
				action->lineEdit()->selectAll();
				return;
			}

			addEntryMenu->hide();
			action->setIpAddress({});
			m_ui->ipPolicyList->edit(idx);
		});

		connect(m_ui->defaultPolicy, &ConnectionPolicyCombo::connectionPolicyChanged, [this](ConnectionPolicy policy) {
			// can be null while setting up UI
			if(!m_server) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: server not yet set\n";
				return;
			}

			m_server->configuration().setDefaultConnectionPolicy(policy);
			Q_EMIT defaultConnectionPolicyChanged(policy);
		});

		m_ui->ipPolicyList->setItemDelegateForColumn(ServerIpConnectionPolicyModel::PolicyColumnIndex, m_delegate.get());
		onIpListSelectionChanged();
	}


	AccessControlWidget::AccessControlWidget(Server * server, QWidget * parent)
	: AccessControlWidget(parent) {
		setServer(server);
	}


	void AccessControlWidget::setServer(Server * server) {
		std::array<QSignalBlocker, 2> blocks = {{QSignalBlocker(m_ui->defaultPolicy), QSignalBlocker(m_ui->ipPolicyList)}};
		m_server = server;

		if(!server) {
			m_model.reset(nullptr);
			m_ui->defaultPolicy->setConnectionPolicy(ConnectionPolicy::None);
		}
		else {
			m_model = std::make_unique<ServerIpConnectionPolicyModel>(server);
			m_ui->defaultPolicy->setConnectionPolicy(server->configuration().defaultConnectionPolicy());

			connect(m_model.get(), &ServerIpConnectionPolicyModel::policyChanged, this, &AccessControlWidget::ipAddressConnectionPolicySet);
		}

		auto * selectionModel = m_ui->ipPolicyList->selectionModel();

		if(selectionModel) {
			selectionModel->disconnect(this);
		}

		m_ui->ipPolicyList->setModel(m_model.get());
		selectionModel = m_ui->ipPolicyList->selectionModel();

		if(selectionModel) {
			connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &AccessControlWidget::onIpListSelectionChanged, Qt::UniqueConnection);
		}
	}


	/// \brief Destroy the AccessControlWidget.
	AccessControlWidget::~AccessControlWidget() = default;


	/// \brief Fetch the currently selected IP address.
	///
	/// If no IP address is selected, an empty string is returned.
	///
	/// \return The selected IP address.
	QString AccessControlWidget::selectedIpAddress() const {
		Q_ASSERT_X(m_model, __PRETTY_FUNCTION__, "model must not be null");
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
		Q_ASSERT_X(m_model, __PRETTY_FUNCTION__, "model must not be null");
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
		Q_ASSERT_X(m_model, __PRETTY_FUNCTION__, "model must not be null");
		return m_model->index(m_ui->ipPolicyList->currentIndex().row(), ServerIpConnectionPolicyModel::IpAddressColumnIndex).data().value<QString>();
	}


	/// \brief Fetch the connection policy currently selected in the policy combo box.
	///
	/// \return The policy.
	ConnectionPolicy AccessControlWidget::currentIpAddressConnectionPolicy() const {
		Q_ASSERT_X(m_model, __PRETTY_FUNCTION__, "model must not be null");
		return m_model->index(m_ui->ipPolicyList->currentIndex().row(), ServerIpConnectionPolicyModel::PolicyColumnIndex).data().value<ConnectionPolicy>();
	}


	/// \brief Fetch the default connection policy displayed in the widget.
	///
	/// \return The policy.
	ConnectionPolicy AccessControlWidget::defaultConnectionPolicy() const {
		return m_ui->defaultPolicy->connectionPolicy();
	}


	void AccessControlWidget::setDefaultConnectionPolicy(ConnectionPolicy policy) {
		m_ui->defaultPolicy->setConnectionPolicy(policy);
	}


	/// \brief Clear all policies for all IP addresses.
	void AccessControlWidget::clearAllConnectionPolicies() {
		Q_ASSERT_X(m_model, __PRETTY_FUNCTION__, "model must not be null");
		m_model->removeRows(0, m_model->rowCount(), {});
	}


	void AccessControlWidget::setIpAddressConnectionPolicy(const QString & addr, ConnectionPolicy policy) {
		Q_ASSERT_X(m_model, __PRETTY_FUNCTION__, "model must not be null");
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


	void AccessControlWidget::onIpListSelectionChanged() {
		auto * model = m_ui->ipPolicyList->selectionModel();
		m_ui->remove->setEnabled(model && !model->selectedIndexes().isEmpty());
	}


}  // namespace Anansi
