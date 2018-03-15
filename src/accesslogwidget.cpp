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

/// \file accesslogwidget.cpp
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Implementation of the AccessLogWidget class.
///
/// \dep
/// - accesslogwidget.h
/// - accesslogwidget.ui
/// - <iostream>
/// - <QString>
/// - <QTreeWidgetItem>
/// - <QFileDialog>
/// - <QMessageBox>
/// - <QFile>
/// - <QTextStream>
/// - accesslogwidgetitem.h
/// - windowbase.h
/// - notifications.h
///
/// \par Changes
/// - (2018-03) First release.

#include "accesslogwidget.h"
#include "ui_accesslogwidget.h"

#include <iostream>
#include <QString>
#include <QTreeWidgetItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>

#include "accesslogtreeitem.h"
#include "windowbase.h"
#include "notifications.h"


namespace Anansi {


	AccessLogWidget::AccessLogWidget(QWidget * parent)
	: QWidget(parent),
	  m_ui(std::make_unique<Ui::AccessLogWidget>()) {
		m_ui->setupUi(this);
		QTreeWidgetItem * accessLogHeader = new QTreeWidgetItem;
		accessLogHeader->setText(0, tr("Time"));
		accessLogHeader->setText(1, tr("Remote IP"));
		accessLogHeader->setText(2, tr("Remote Port"));
		accessLogHeader->setText(3, tr("Resource Requested"));
		accessLogHeader->setText(4, tr("Response/Action"));
		// QTreeWidget takes ownership
		m_ui->log->setHeaderItem(accessLogHeader);

		connect(m_ui->save, &QPushButton::clicked, this, &AccessLogWidget::save);
		connect(m_ui->clear, &QPushButton::clicked, this, &AccessLogWidget::clear);
	}


	// required in impl. file due to use of std::unique_ptr with forward-declared class.
	AccessLogWidget::~AccessLogWidget() = default;


	void AccessLogWidget::save() {
		static QString lastSavePath = QDir::homePath();
		auto fileName = QFileDialog::getSaveFileName(this, tr("Save access log"), lastSavePath);

		if(fileName.isEmpty()) {
			return;
		}

		lastSavePath = fileName;
		saveAs(fileName);
	}


	void AccessLogWidget::saveAs(const QString & fileName) {
		if(fileName.isEmpty()) {
			return;
		}

		QFile outFile(fileName);

		if(!outFile.open(QIODevice::WriteOnly)) {
			showNotification(this, tr("The file <strong>%2</strong> could not be opened for writing.").arg(fileName), NotificationType::Error);
			return;
		}

		QTextStream outStream(&outFile);
		auto itemCount = m_ui->log->topLevelItemCount();

		for(int idx = 0; idx < itemCount; ++idx) {
			auto * logEntry = m_ui->log->topLevelItem(idx);
			outStream << logEntry->text(0) << QByteArrayLiteral(" - ") << logEntry->text(1) << ':' << logEntry->text(2) << ' ' << logEntry->text(3) << ' ' << logEntry->text(4) << '\n';
		}

		outStream.flush();
		outFile.close();
	}


	void AccessLogWidget::clear() {
		m_ui->log->clear();
	}


	void AccessLogWidget::addPolicyEntry(const QDateTime & timestamp, const QString & addr, uint16_t port, ConnectionPolicy policy) {
		m_ui->log->addTopLevelItem(new AccessLogTreeItem(timestamp, addr, port, policy));
	}


	void AccessLogWidget::addActionEntry(const QDateTime & timestamp, const QString & addr, uint16_t port, const QString & resource, WebServerAction action) {
		m_ui->log->addTopLevelItem(new AccessLogTreeItem(timestamp, addr, port, resource, action));
	}


}  // namespace Anansi
