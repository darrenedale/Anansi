/*
 * Copyright 2015 - 2017 Darren Edale
 *
 * This file is part of EquitWebServer.
 *
 * Qonvince is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Qonvince is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EquitWebServer. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file iplineeditaction.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Implementation of the IpLineEditAction class.
///
/// \dep
/// - "iplineeditaction.h"
/// - <QMenu>
/// - <QLabel>
/// - <QLineEdit>
/// - <QPushButton>
/// - <QHBoxLayout>
///
/// \par Changes
/// - (2018-03) First release.

#include "iplineeditaction.h"

#include <QMenu>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>


namespace EquitWebServer {


	/// \class IpLineEditAction
	///
	/// \brief A QWidgetAction encapsulating a QLineEdit widget.
	///
	/// The primary use case for objects of this class is to embed IP address
	/// widgets in QMenus (e.g. for providing a "pop-up" IP Address widget).


	IpLineEditAction::IpLineEditAction(QObject * parent)
	: QWidgetAction(parent) {
		auto * container = new QWidget;
		m_ipAddress = new QLineEdit();
		auto * add = new QPushButton(QIcon::fromTheme("list-add", QIcon(":/icons/buttons/addtolist")), {});
		QHBoxLayout * layout = new QHBoxLayout;
		layout->addWidget(new QLabel(tr("IP address")));
		layout->addWidget(m_ipAddress);
		layout->addWidget(add);
		add->setDefault(true);
		container->setLayout(layout);

		connect(m_ipAddress, &QLineEdit::returnPressed, add, &QPushButton::click);

		connect(add, &QPushButton::clicked, [this]() {
			Q_EMIT addIpAddressClicked(m_ipAddress->text());
		});

		setDefaultWidget(container);
	}


	IpLineEditAction::~IpLineEditAction() = default;


	QString IpLineEditAction::ipAddress() const {
		return m_ipAddress->text();
	}


	void IpLineEditAction::setIpAddress(const QString & addr) {
		m_ipAddress->setText(addr);
	}


}  // namespace EquitWebServer
