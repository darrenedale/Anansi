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

/// \file iplineeditaction.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the IpLineEditAction class.
///
/// \dep
/// - iplineeditaction.h
/// - <QMenu>
/// - <QLabel>
/// - <QLineEdit>
/// - <QPushButton>
/// - <QVBoxLayout>
/// - <QHBoxLayout>
///
/// NEXTRELEASE create a widget class for the layout and use that
///
/// \par Changes
/// - (2018-03) First release.

#include "iplineeditaction.h"

#include <QMenu>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>


namespace Anansi {


	/// \class IpLineEditAction
	///
	/// \brief A QWidgetAction encapsulating a QLineEdit widget.
	///
	/// The primary use case for objects of this class is to embed IP address
	/// widgets in QMenus (e.g. for providing a "pop-up" IP Address widget).


	IpLineEditAction::IpLineEditAction(QObject * parent)
	: QWidgetAction(parent) {
		auto * container = new QWidget;
		m_ipAddress = new QLineEdit;
		m_ipAddress->setPlaceholderText(tr("Enter an IP address..."));
		auto * add = new QPushButton(QIcon::fromTheme("list-add", QIcon(":/icons/buttons/add-to-list")), {});
		auto * mainLayout = new QVBoxLayout;
		auto * inputLayout = new QHBoxLayout;
		mainLayout->addWidget(new QLabel(tr("IP address")));
		mainLayout->addLayout(inputLayout);
		inputLayout->addWidget(m_ipAddress);
		inputLayout->addWidget(add);
		add->setDefault(true);
		container->setLayout(mainLayout);
		connect(m_ipAddress, &QLineEdit::returnPressed, add, &QPushButton::click);

		connect(add, &QPushButton::clicked, [this]() {
			Q_EMIT addIpAddressClicked(m_ipAddress->text());
		});

		QWidgetAction::setDefaultWidget(container);
	}


	// required in impl. file due to use of std::unique_ptr with forward-declared class.
	IpLineEditAction::~IpLineEditAction() = default;


	QString IpLineEditAction::ipAddress() const {
		return m_ipAddress->text();
	}


	void IpLineEditAction::setIpAddress(const QString & addr) {
		m_ipAddress->setText(addr);
	}


}  // namespace Anansi
