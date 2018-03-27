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

/// \file mediatypecombowidgetaction.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the MediaTypeComboWidgetAction class.
///
/// \dep
/// - mediatypecombowidgetaction.h
/// - <QHBoxLayout>
/// - <QHBoxLayout>
/// - <QLabel>
/// - <QPushButton>
/// - <QLineEdit>
/// - mediatypecombo.h
///
/// \par Changes
/// - (2018-03) First release.

#include "mediatypecombowidgetaction.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

#include "mediatypecombo.h"


namespace Anansi {


	MediaTypeComboWidgetAction::MediaTypeComboWidgetAction(QObject * parent)
	: QWidgetAction(parent) {
		auto * container = new QWidget;
		m_combo = new MediaTypeCombo(true);
		auto * add = new QPushButton(QIcon::fromTheme("dialog-ok-accept", QIcon(QStringLiteral(":/icons/buttons/add-to-list"))), {});
		add->setDefault(true);
		QVBoxLayout * mainLayout = new QVBoxLayout;
		QHBoxLayout * inputLayout = new QHBoxLayout;
		mainLayout->addWidget(new QLabel(tr("Media type")));
		mainLayout->addLayout(inputLayout);
		inputLayout->addWidget(m_combo);
		inputLayout->addWidget(add);
		container->setLayout(mainLayout);

		connect(m_combo->lineEdit(), &QLineEdit::returnPressed, add, &QPushButton::click);

		connect(add, &QPushButton::clicked, [this]() {
			Q_EMIT addMediaTypeClicked(m_combo->currentMediaType());
		});

		setDefaultWidget(container);
	}


	void MediaTypeComboWidgetAction::setMediaTypes(std::vector<QString> mediaTypes) {
		m_combo->clear();

		for(const auto & mediaType : mediaTypes) {
			m_combo->addMediaType(mediaType);
		}
	}


	void MediaTypeComboWidgetAction::addMediaType(const QString & mediaType) {
		m_combo->addMediaType(mediaType);
	}


}  // namespace Anansi
