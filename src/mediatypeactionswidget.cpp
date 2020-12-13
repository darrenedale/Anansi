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

/// \file mediatypeactionswidget.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the MediaTypeActionsWidget class.
///
/// \dep
/// - mediatypeactionswidget.h
/// - mediatypeactionswidget.ui
/// - <array>
/// - <iostream>
/// - <QMenu>
/// - <QSignalBlocker>
/// - <QPushButton>
/// - <QLineEdit>
/// - <QModelIndex>
/// - macros.h
/// - types.h
/// - qtmetatypes.h
/// - server.h
/// - mediatypeactionsmodel.h
/// - mediatypecombo.h
/// - mediatypecombowidgetaction.h
/// - mediatypeactionsdelegate.h
/// - notifications.h
///
/// \par Changes
/// - (2018-03) First release.

#include "mediatypeactionswidget.h"
#include "ui_mediatypeactionswidget.h"

#include <array>
#include <iostream>

#include <QMenu>
#include <QSignalBlocker>
#include <QPushButton>
#include <QLineEdit>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QSortFilterProxyModel>

#include "macros.h"
#include "types.h"
#include "qtmetatypes.h"
#include "server.h"
#include "mediatypeactionsmodel.h"
#include "mediatypecombo.h"
#include "mediatypecombowidgetaction.h"
#include "mediatypeactionsdelegate.h"
#include "notifications.h"


namespace Anansi {


	MediaTypeActionsWidget::MediaTypeActionsWidget(QWidget * parent)
	: QWidget(parent),
	  m_proxyModel(std::make_unique<QSortFilterProxyModel>()),
	  m_model(nullptr),
	  m_ui(std::make_unique<Ui::MediaTypeActionsWidget>()),
	  m_addEntryMenu(std::make_unique<QMenu>()),
	  m_server(nullptr),
	  m_addMediaTypeCombo(nullptr) {
		m_ui->setupUi(this);
		m_ui->actions->setItemDelegate(new MediaTypeActionsDelegate(this));
		m_ui->actions->setModel(m_proxyModel.get());

		m_proxyModel->setFilterKeyColumn(MediaTypeActionsModel::MediaTypeColumnIndex);
		m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

		auto * action = new MediaTypeComboWidgetAction(this);
		m_addMediaTypeCombo = action->mediaTypeCombo();
		m_addEntryMenu->addAction(action);
		m_ui->add->setMenu(m_addEntryMenu.get());

		connect(m_addEntryMenu.get(), &QMenu::aboutToShow, [action] () {
			action->mediaTypeCombo()->lineEdit()->selectAll();
		});

      // can't use qOverload() with MSVC because it doesn't implement SD-6 (feature
      // detection macros)
      connect(m_addEntryMenu.get(), &QMenu::aboutToShow, m_addMediaTypeCombo, QOverload<>::of(&MediaTypeCombo::setFocus));

		connect(action, &MediaTypeComboWidgetAction::addMediaTypeClicked, [this](const QString & mediaType, const WebServerAction & action) {
			const auto idx = m_model->addMediaType(mediaType, action);

			if(!idx.isValid()) {
				std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: failed to add media type \"" << qPrintable(mediaType) << "\" with action = " << enumeratorString(m_ui->defaultAction->webServerAction()) << " to media type actions list. is it already present?\n";
				showNotification(this, tr("<p>A new action for the media type <strong>%1</strong> could not be added.</p><p><small>Perhaps this media type already has an action assigned?</small></p>").arg(mediaType), NotificationType::Error);
				m_addMediaTypeCombo->setFocus();
				m_addMediaTypeCombo->lineEdit()->selectAll();
				return;
			}

			m_addEntryMenu->hide();
			m_ui->actions->scrollTo(idx);
			m_ui->actions->setCurrentIndex(idx);
		});

		connect(m_ui->filter, &QLineEdit::textEdited, [this] (const QString & term) {
			this->m_proxyModel->setFilterRegularExpression(QRegularExpression(QRegularExpression::escape(term), QRegularExpression::CaseInsensitiveOption));
		});

		connect(m_ui->remove, &QPushButton::clicked, [this]() {
			const auto idx = m_ui->actions->currentIndex();

			if(!idx.isValid()) {
				return;
			}

			const auto row = idx.row();
			const auto mediaType = m_model->index(row, MediaTypeActionsModel::MediaTypeColumnIndex).data().value<QString>();
			const auto action = m_model->index(row, MediaTypeActionsModel::ActionColumnIndex).data().value<WebServerAction>();

			if(m_model->removeRows(idx.row(), 1, {})) {
				if(WebServerAction::CGI == action) {
					Q_EMIT mediaTypeActionRemoved(mediaType, action, m_model->index(row, MediaTypeActionsModel::CgiColumnIndex).data().value<QString>());
				}
				else {
					Q_EMIT mediaTypeActionRemoved(mediaType, action);
				}
			}
		});

		connect(m_ui->defaultAction, &WebServerActionCombo::webServerActionChanged, [this](WebServerAction action) {
			// can be null while setting up UI
			if(!m_server) {
				std::cerr << EQ_PRETTY_FUNCTION << " [" << __LINE__ << "]: server not yet set\n"
							 << std::flush;
				return;
			}

			m_server->configuration().setDefaultAction(action);
			Q_EMIT defaultActionChanged(action);
		});
	}


	MediaTypeActionsWidget::MediaTypeActionsWidget(Server * server, QWidget * parent)
	: MediaTypeActionsWidget(parent) {
		setServer(server);
	}


	// required in impl. file due to use of std::unique_ptr with forward-declared class.
	MediaTypeActionsWidget::~MediaTypeActionsWidget() = default;


	void MediaTypeActionsWidget::setServer(Server * server) {
		std::array<QSignalBlocker, 2> blocks = {{QSignalBlocker(m_ui->defaultAction), QSignalBlocker(m_ui->actions)}};
		m_server = server;
		m_addMediaTypeCombo->clear();

		if(!server) {
			m_proxyModel->setSourceModel(nullptr);
			m_model.reset(nullptr);
			m_ui->defaultAction->setWebServerAction(WebServerAction::Ignore);
		}
		else {
			m_model = std::make_unique<MediaTypeActionsModel>(server);
			m_proxyModel->setSourceModel(m_model.get());

			m_ui->defaultAction->setWebServerAction(server->configuration().defaultAction());

			for(const auto & mediaType : server->configuration().allKnownMediaTypes()) {
				m_addMediaTypeCombo->addMediaType(mediaType);
			}
		}

		auto * selectionModel = m_ui->actions->selectionModel();

		if(selectionModel) {
			selectionModel->disconnect(this);
		}

		selectionModel = m_ui->actions->selectionModel();

		if(selectionModel) {
			connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &MediaTypeActionsWidget::onActionsSelectionChanged, Qt::UniqueConnection);
		}

		m_ui->actions->resizeColumnToContents(MediaTypeActionsModel::MediaTypeColumnIndex);
		m_ui->actions->resizeColumnToContents(MediaTypeActionsModel::ActionColumnIndex);
		m_ui->actions->resizeColumnToContents(MediaTypeActionsModel::CgiColumnIndex);

		// edit combo needs a bit more space, usually
		m_ui->actions->setColumnWidth(MediaTypeActionsModel::ActionColumnIndex, m_ui->actions->columnWidth(MediaTypeActionsModel::ActionColumnIndex) + 25);
	}


	WebServerAction MediaTypeActionsWidget::defaultAction() const {
		return m_ui->defaultAction->webServerAction();
	}


	void MediaTypeActionsWidget::setDefaultAction(WebServerAction action) {
		if(action == defaultAction()) {
			return;
		}

		m_ui->defaultAction->setWebServerAction(action);
		Q_EMIT defaultActionChanged(action);
	}


	void MediaTypeActionsWidget::clear() {
		m_model->clear();
	}


	void MediaTypeActionsWidget::onActionsSelectionChanged() {
		auto * selectionModel = m_ui->actions->selectionModel();
		m_ui->remove->setEnabled(selectionModel && !selectionModel->selectedIndexes().isEmpty());
	}


}  // namespace Anansi
