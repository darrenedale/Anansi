/*
 * Copyright 2015 - 2017 Darren Edale
 *
 * This file is part of Anansi web server.
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
 * along with Anansi. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file mimecombowidgetaction.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Implementation of the MimeComboWidgetAction class.
///
/// \dep
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


	/// \class MimeComboWidgetAction
	///
	/// \brief A QWidgetAction encapsulating a MimeCombo widget.
	///
	/// The primary use case for objects of this class is to embed MIME combos in
	/// QMenus (e.g. for providing a "pop-up" MimeCombo widget).


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

		for(const auto & mimeType : mimeTypes) {
			m_combo->addMimeType(mimeType);
		}
	}


	void MimeComboWidgetAction::addMimeType(const QString & mimeType) {
		m_combo->addMimeType(mimeType);
	}


	MimeComboWidgetAction::~MimeComboWidgetAction() = default;


}  // namespace Anansi
