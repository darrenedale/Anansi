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

/// \file accesslogwidget.cpp
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Implementation of the AccessLogWidget class.
///
/// \dep
/// - <QFileDialog>
/// - <QLineEdit>
/// - <QPushButton>
///
/// \par Changes
/// - (2018-03) First release.

#include "filenamewidget.h"
#include "ui_filenamewidget.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QPushButton>


namespace Anansi {


	FileNameWidget::FileNameWidget(const QString & path, QWidget * parent)
	: FileNameWidget(parent) {
		setFileName(path);
	}


	FileNameWidget::FileNameWidget(QWidget * parent)
	: QWidget(parent),
	  m_ui(std::make_unique<Ui::FileNameWidget>()) {
		m_ui->setupUi(this);
		connect(m_ui->path, &QLineEdit::textEdited, this, &FileNameWidget::fileNameChanged);

		connect(m_ui->choose, &QPushButton::clicked, [this]() {
			chooseFile();
		});
	}


	QString FileNameWidget::placeholderText() const {
		return m_ui->path->placeholderText();
	}


	FileNameWidget::~FileNameWidget() = default;


	void FileNameWidget::setPlaceholderText(const QString & str) {
		m_ui->path->setPlaceholderText(str);
	}


	void FileNameWidget::setFileName(const QString & path) {
		if(path == m_ui->path->text()) {
			return;
		}

		m_ui->path->setText(path);
		Q_EMIT fileNameChanged(path);
	}


	QString FileNameWidget::fileName() const {
		return m_ui->path->text();
	}


	void FileNameWidget::chooseFile(QString path) {
		if(path.isEmpty()) {
			path = m_ui->path->text();
		}

		path = QFileDialog::getOpenFileName(this, (m_dialogueCaption.isEmpty() ? tr("Choose file") : m_dialogueCaption), path, m_dialogueFilter);

		if(path.isNull()) {
			return;
		}

		m_ui->path->setText(path);
		Q_EMIT fileNameChanged(path);
	}

}  // namespace Anansi
