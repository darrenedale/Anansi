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

/// \file mimeactionswidget.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the MimeActionsWidget class.
///
/// \dep
/// - mimeactionswidget.h
/// - mimeactionswidget.ui
/// - <array>
/// - <iostream>
/// - <QMenu>
/// - <QSignalBlocker>
/// - <QPushButton>
/// - <QLineEdit>
/// - <QModelIndex>
/// - types.h
/// - qtmetatypes.h
/// - server.h
/// - servermimeactionsmodel.h
/// - mimecombo.h
/// - mimecombowidgetaction.h
/// - mimetypeactionsdelegate.h
/// - notifications.h
///
/// \par Changes
/// - (2018-03) First release.

#include "mimeactionswidget.h"
#include "ui_mimeactionswidget.h"

#include <array>
#include <iostream>

#include <QMenu>
#include <QSignalBlocker>
#include <QPushButton>
#include <QLineEdit>
#include <QItemSelectionModel>
#include <QModelIndex>

#include "types.h"
#include "qtmetatypes.h"
#include "server.h"
#include "servermimeactionsmodel.h"
#include "mimecombo.h"
#include "mimecombowidgetaction.h"
#include "mimetypeactionsdelegate.h"
#include "notifications.h"


namespace Anansi {


	MimeActionsWidget::MimeActionsWidget(QWidget * parent)
	: QWidget(parent),
	  m_model(nullptr),
	  m_ui(std::make_unique<Ui::MimeActionsWidget>()),
	  m_addEntryMenu(std::make_unique<QMenu>()),
	  m_server(nullptr),
	  m_addMimeCombo(nullptr) {
		m_ui->setupUi(this);
		m_ui->actions->setItemDelegate(new MimeTypeActionsDelegate(this));

		auto * action = new MimeComboWidgetAction(this);
		m_addMimeCombo = action->mimeCombo();
		m_addEntryMenu->addAction(action);
		m_ui->add->setMenu(m_addEntryMenu.get());

		connect(m_addEntryMenu.get(), &QMenu::aboutToShow, m_addMimeCombo, qOverload<>(&MimeCombo::setFocus));

		connect(action, &MimeComboWidgetAction::addMimeTypeClicked, [this](const QString & mime) {
			const auto idx = m_model->addMimeType(mime, m_ui->defaultAction->webServerAction());

			if(!idx.isValid()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to add media type \"" << qPrintable(mime) << "\" with action = " << enumeratorString(m_ui->defaultAction->webServerAction()) << " to media type actions list. is it already present?\n";
				showNotification(this, tr("<p>A new action for the media type <strong>%1</strong> could not be added.</p><p><small>Perhaps this media type already has an action assigned?</small></p>").arg(mime), NotificationType::Error);
				m_addMimeCombo->setFocus();
				m_addMimeCombo->lineEdit()->selectAll();
				return;
			}

			m_addEntryMenu->hide();
			m_ui->actions->edit(idx);
		});

		connect(m_ui->remove, &QPushButton::clicked, [this]() {
			const auto idx = m_ui->actions->currentIndex();

			if(!idx.isValid()) {
				return;
			}

			const auto row = idx.row();
			const auto mime = m_model->index(row, ServerMimeActionsModel::MimeTypeColumnIndex).data().value<QString>();
			const auto action = m_model->index(row, ServerMimeActionsModel::ActionColumnIndex).data().value<WebServerAction>();

			if(m_model->removeRows(idx.row(), 1, {})) {
				if(WebServerAction::CGI == action) {
					Q_EMIT mimeTypeActionRemoved(mime, action, m_model->index(row, ServerMimeActionsModel::CgiColumnIndex).data().value<QString>());
				}
				else {
					Q_EMIT mimeTypeActionRemoved(mime, action);
				}
			}
		});

		connect(m_ui->defaultAction, &WebServerActionCombo::webServerActionChanged, [this](WebServerAction action) {
			// can be null while setting up UI
			if(!m_server) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: server not yet set\n"
							 << std::flush;
				return;
			}

			m_server->configuration().setDefaultAction(action);
			Q_EMIT defaultActionChanged(action);
		});
	}


	MimeActionsWidget::MimeActionsWidget(Server * server, QWidget * parent)
	: MimeActionsWidget(parent) {
		setServer(server);
	}


	// required in impl. file due to use of std::unique_ptr with forward-declared class.
	MimeActionsWidget::~MimeActionsWidget() = default;


	void MimeActionsWidget::setServer(Server * server) {
		std::array<QSignalBlocker, 2> blocks = {{QSignalBlocker(m_ui->defaultAction), QSignalBlocker(m_ui->actions)}};
		m_server = server;
		m_addMimeCombo->clear();

		if(!server) {
			m_model.reset(nullptr);
			m_ui->defaultAction->setWebServerAction(WebServerAction::Ignore);
		}
		else {
			m_model = std::make_unique<ServerMimeActionsModel>(server);
			m_ui->defaultAction->setWebServerAction(server->configuration().defaultAction());

			for(const auto & mimeType : server->configuration().allKnownMimeTypes()) {
				m_addMimeCombo->addMimeType(mimeType);
			}
		}

		auto * selectionModel = m_ui->actions->selectionModel();

		if(selectionModel) {
			selectionModel->disconnect(this);
		}

		m_ui->actions->setModel(m_model.get());
		selectionModel = m_ui->actions->selectionModel();

		if(selectionModel) {
			connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &MimeActionsWidget::onActionsSelectionChanged, Qt::UniqueConnection);
		}

		m_ui->actions->resizeColumnToContents(ServerMimeActionsModel::MimeTypeColumnIndex);
		m_ui->actions->resizeColumnToContents(ServerMimeActionsModel::ActionColumnIndex);
		m_ui->actions->resizeColumnToContents(ServerMimeActionsModel::CgiColumnIndex);

		// edit combo needs a bit more space, usually
		m_ui->actions->setColumnWidth(ServerMimeActionsModel::ActionColumnIndex, m_ui->actions->columnWidth(ServerMimeActionsModel::ActionColumnIndex) + 25);
	}


	WebServerAction MimeActionsWidget::defaultAction() const {
		return m_ui->defaultAction->webServerAction();
	}


	void MimeActionsWidget::setDefaultAction(WebServerAction action) {
		if(action == defaultAction()) {
			return;
		}

		m_ui->defaultAction->setWebServerAction(action);
		Q_EMIT defaultActionChanged(action);
	}


	void MimeActionsWidget::clear() {
		m_model->clear();
	}


	void MimeActionsWidget::onActionsSelectionChanged() {
		auto * selectionModel = m_ui->actions->selectionModel();
		m_ui->remove->setEnabled(selectionModel && !selectionModel->selectedIndexes().isEmpty());
	}


}  // namespace Anansi
