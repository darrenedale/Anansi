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

/// \file accesslogwidget.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the AccessLogWidget class for Anansi.
///
/// \dep
/// - <cstdint>
/// - <memory>
/// - <QWidget>
/// - <QDateTime>
/// - types.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_ACCESSLOGWIDGET_H
#define ANANSI_ACCESSLOGWIDGET_H

#include <cstdint>
#include <memory>

#include <QWidget>
#include <QDateTime>

#include "types.h"

class QString;

namespace Anansi {

	namespace Ui {
		class AccessLogWidget;
	}

	class AccessLogWidget : public QWidget {
		Q_OBJECT

	public:
		explicit AccessLogWidget(QWidget * parent = nullptr);
		virtual ~AccessLogWidget() override;

	public Q_SLOTS:
		void save();
		void saveAs(const QString & fileName);

		void clear();

		void addPolicyEntry(const QDateTime & timestamp, const QString & addr, uint16_t port, ConnectionPolicy policy);
		void addActionEntry(const QDateTime & timestamp, const QString & addr, uint16_t port, const QString & resource, WebServerAction action);

		inline void addPolicyEntry(const QString & addr, uint16_t port, ConnectionPolicy policy) {
			addPolicyEntry(QDateTime::currentDateTime(), addr, port, policy);
		}

		inline void addActionEntry(const QString & addr, uint16_t port, const QString & resource, WebServerAction action) {
			addActionEntry(QDateTime::currentDateTime(), addr, port, resource, action);
		}

	private:
		std::unique_ptr<Ui::AccessLogWidget> m_ui;
	};

}  // namespace Anansi

#endif  // ANANSI_ACCESSLOGWIDGET_H
