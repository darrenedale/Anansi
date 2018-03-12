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

/// \file mimetypeactionswidget.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the MimeTypeActionsWidget class for Anansi.
///
/// \dep
/// - <memory>
/// - <QWidget>
/// - types.h
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_MIMETYPEACTIONSWIDGET_H
#define ANANSI_MIMETYPEACTIONSWIDGET_H

#include <memory>

#include <QWidget>

#include "types.h"

class QString;

namespace Anansi {

	class Server;
	class ServerMimeActionsModel;
	class MimeCombo;

	namespace Ui {
		class MimeActionsWidget;
	}

	class MimeActionsWidget : public QWidget {
		Q_OBJECT

	public:
		explicit MimeActionsWidget(QWidget * = nullptr);
		explicit MimeActionsWidget(Server *, QWidget * = nullptr);
		~MimeActionsWidget() override;

		void setServer(Server *);

		WebServerAction defaultAction() const;
		void setDefaultAction(WebServerAction);

		void clear();

	Q_SIGNALS:
		void defaultActionChanged(WebServerAction);
		void mimeTypeActionRemoved(const QString &, WebServerAction, const QString & = {});

	private Q_SLOTS:
		void onActionsSelectionChanged();

	private:
		std::unique_ptr<ServerMimeActionsModel> m_model;
		std::unique_ptr<Ui::MimeActionsWidget> m_ui;
		Server * m_server;  // observed only
		MimeCombo * m_addMimeCombo;
	};

}  // namespace Anansi

#endif  // ANANSI_MIMETYPEACTIONSWIDGET_H
