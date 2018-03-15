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

/// \file filenamewidget.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the FileNameWidget class for Anansi.
///
/// \dep
/// - <memory>
/// - <QWidget>
/// - <QString>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_FILENAMEWIDGET_H
#define ANANSI_FILENAMEWIDGET_H

#include <memory>

#include <QWidget>
#include <QString>

namespace Anansi {

	namespace Ui {
		class FileNameWidget;
	}

	class FileNameWidget : public QWidget {
		Q_OBJECT

	public:
		explicit FileNameWidget(QWidget * parent = nullptr);
		explicit FileNameWidget(const QString & path, QWidget * parent = nullptr);
		virtual ~FileNameWidget();

		QString placeholderText() const;
		void setPlaceholderText(const QString & placeholder);

		inline void setDialogueCaption(const QString & caption) {
			m_dialogueCaption = caption;
		}

		inline const QString & dialogueCaption() const noexcept {
			return m_dialogueCaption;
		}

		void setFileName(const QString & path);
		QString fileName() const;

		inline void setFilter(const QString & filter) {
			m_dialogueFilter = filter;
		}

		inline const QString & filter() const noexcept {
			return m_dialogueFilter;
		}

	Q_SIGNALS:
		void fileNameChanged(const QString & path);

	public Q_SLOTS:
		void chooseFile(QString path = {});

	private:
		std::unique_ptr<Ui::FileNameWidget> m_ui;
		QString m_dialogueCaption;
		QString m_dialogueFilter;
	};

}  // namespace Anansi

#endif  // ANANSI_FILENAMEWIDGET_H
