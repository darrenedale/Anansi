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

/// \file filesystempathwidget.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the FilesystemPathWidget class.
///
/// \dep
/// - filenamewidget.h
/// - filenamewidget.ui
/// - <QFileDialog>
/// - <QLineEdit>
/// - <QPushButton>
///
/// NEXTRELEASE accept drop of path on widget
///
/// \par Changes
/// - (2018-03) First release.

#include "filesystempathwidget.h"
#include "ui_filesystempathwidget.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>


namespace Anansi {


	FilesystemPathWidget::FilesystemPathWidget(QWidget * parent)
	: QWidget(parent),
	  m_ui(std::make_unique<Ui::FileNameWidget>()),
	  m_pathType(PathType::OpenFile) {
		m_ui->setupUi(this);
		connect(m_ui->path, &QLineEdit::textEdited, this, &FilesystemPathWidget::textEdited);
		connect(m_ui->path, &QLineEdit::textChanged, this, &FilesystemPathWidget::textChanged);
		connect(m_ui->path, &QLineEdit::returnPressed, this, &FilesystemPathWidget::returnPressed);
		connect(m_ui->path, &QLineEdit::editingFinished, this, &FilesystemPathWidget::editingFinished);
		connect(m_ui->path, &QLineEdit::cursorPositionChanged, this, &FilesystemPathWidget::cursorPositionChanged);
		connect(m_ui->path, &QLineEdit::selectionChanged, this, &FilesystemPathWidget::selectionChanged);

		connect(m_ui->path, &QLineEdit::editingFinished, [this]() {
			Q_EMIT pathChanged(m_ui->path->text());
		});

		connect(m_ui->choose, &QPushButton::clicked, [this]() {
			choosePath();
		});
	}


	FilesystemPathWidget::FilesystemPathWidget(const QString & path, QWidget * parent)
	: FilesystemPathWidget(parent) {
		setPath(path);
	}


	FilesystemPathWidget::FilesystemPathWidget(FilesystemPathWidget::PathType type, QWidget * parent)
	: FilesystemPathWidget(parent) {
		setPathType(type);
	}


	// required in impl. file due to use of std::unique_ptr with forward-declared class.
	FilesystemPathWidget::~FilesystemPathWidget() = default;


	QString FilesystemPathWidget::placeholderText() const {
		return m_ui->path->placeholderText();
	}


	void FilesystemPathWidget::setPlaceholderText(const QString & placeholder) {
		m_ui->path->setPlaceholderText(placeholder);
	}


	QString FilesystemPathWidget::path() const {
		return m_ui->path->text();
	}


	void FilesystemPathWidget::setPath(const QString & path) {
		if(path == m_ui->path->text()) {
			return;
		}

		m_ui->path->setText(path);
		Q_EMIT pathChanged(path);
	}


	void FilesystemPathWidget::choosePath(QString path) {
		if(path.isEmpty()) {
			path = m_ui->path->text();
		}

		const auto & caption = dialogueCaption();

		switch(pathType()) {
			case PathType::OpenFile:
				path = QFileDialog::getOpenFileName(this, (caption.isEmpty() ? tr("Choose file") : caption), path, m_dialogueFilter);
				break;

			case PathType::SaveFile:
				path = QFileDialog::getSaveFileName(this, (caption.isEmpty() ? tr("Choose file") : caption), path, m_dialogueFilter);
				break;

			case PathType::ExistingDirectory:
				path = QFileDialog::getExistingDirectory(this, (caption.isEmpty() ? tr("Choose directory") : caption), path);
				break;
		}

		if(path.isNull()) {
			return;
		}

		m_ui->path->setText(path);
		Q_EMIT pathChanged(path);
	}


}  // namespace Anansi
