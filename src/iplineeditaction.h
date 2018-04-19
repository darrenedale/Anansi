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

/// \file iplineeditaction.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the IpLineEditAction class for Anansi.
///
/// \dep
/// - <QWidgetAction>
/// - <QString>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_IPLINEEDITACTION_H
#define ANANSI_IPLINEEDITACTION_H

#include <QWidgetAction>
#include <QString>

class QLineEdit;

namespace Anansi {

	class IpLineEditAction : public QWidgetAction {
		Q_OBJECT

	public:
		IpLineEditAction(QObject * parent = nullptr);
		~IpLineEditAction() override;

		void setDefaultWidget(QWidget *) = delete;

		QLineEdit * lineEdit() const noexcept {
			return m_ipAddress;
		}

		QString ipAddress() const;
		void setIpAddress(const QString & addr);

	Q_SIGNALS:
		void addIpAddressClicked(const QString & addr);

	private:
		QLineEdit * m_ipAddress;
	};

}  // namespace Anansi

#endif  // ANANSI_IPLINEEDITACTION_H
