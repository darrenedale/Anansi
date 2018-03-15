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

/// \file mimecombowidgetaction.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the MimeComboWidgetAction class.
///
/// \dep
/// - mimecombowidgetaction.h
/// - <QHBoxLayout>
/// - <QLabel>
/// - <QPushButton>
/// - <QLineEdit>
/// - mimecombo.h
///
/// \par Changes
/// - (2018-03) First release.

#include "mimecombowidgetaction.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

#include "mimecombo.h"


namespace Anansi {


	MimeComboWidgetAction::MimeComboWidgetAction(QObject * parent)
	: QWidgetAction(parent) {
		auto * container = new QWidget;
		m_combo = new MimeCombo(true);
		auto * add = new QPushButton(QIcon::fromTheme("dialog-ok-accept"), {});
		add->setDefault(true);
		QHBoxLayout * layout = new QHBoxLayout;
		layout->addWidget(new QLabel(tr("Mime type")));
		layout->addWidget(m_combo);
		layout->addWidget(add);
		container->setLayout(layout);

		connect(m_combo->lineEdit(), &QLineEdit::returnPressed, add, &QPushButton::click);

		connect(add, &QPushButton::clicked, [this]() {
			Q_EMIT addMimeTypeClicked(m_combo->currentMimeType());
		});

		setDefaultWidget(container);
	}


	void MimeComboWidgetAction::setMimeTypes(std::vector<QString> mimeTypes) {
		m_combo->clear();

		for(const auto & mime : mimeTypes) {
			m_combo->addMimeType(mime);
		}
	}


	void MimeComboWidgetAction::addMimeType(const QString & mime) {
		m_combo->addMimeType(mime);
	}


}  // namespace Anansi
