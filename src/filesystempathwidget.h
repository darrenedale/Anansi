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

/// \file filesystempathwidget.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the FilesystemPathWidget class for Anansi.
///
/// \dep
/// - <memory>
/// - <QWidget>
/// - <QString>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_FILESYSTEMPATHWIDGET_H
#define ANANSI_FILESYSTEMPATHWIDGET_H

#include <memory>

#include <QWidget>
#include <QString>

namespace Anansi {

	namespace Ui {
		class FileNameWidget;
	}

	class FilesystemPathWidget : public QWidget {
		Q_OBJECT

	public:
		enum PathType {
			OpenFile,
			SaveFile,
			ExistingDirectory,
		};

		explicit FilesystemPathWidget(QWidget * parent = nullptr);
		explicit FilesystemPathWidget(const QString & path, QWidget * parent = nullptr);
		explicit FilesystemPathWidget(PathType type, QWidget * parent = nullptr);
		virtual ~FilesystemPathWidget();

		QString placeholderText() const;
		void setPlaceholderText(const QString & placeholder);

		inline void setDialogueCaption(const QString & caption) {
			m_dialogueCaption = caption;
		}

		inline const QString & dialogueCaption() const noexcept {
			return m_dialogueCaption;
		}

		void setPath(const QString & path);
		QString path() const;

		inline void setFilter(const QString & filter) {
			m_dialogueFilter = filter;
		}

		inline const QString & filter() const noexcept {
			return m_dialogueFilter;
		}

		inline PathType pathType() const noexcept {
			return m_pathType;
		}

		inline void setPathType(PathType type) {
			m_pathType = type;
		}

	Q_SIGNALS:
		// the user has changed the path (emitted when a new path is chosen via dialogue
		// or the user finishes directly editing the text
		void pathChanged(const QString & path);

		// pass-through for QLineEdit signals
		void textChanged(const QString & text);
		void textEdited(const QString & text);
		void cursorPositionChanged(int oldPos, int newPos);
		void selectionChanged();
		void returnPressed();
		void editingFinished();

	public Q_SLOTS:
		void choosePath(QString path = {});

	private:
		std::unique_ptr<Ui::FileNameWidget> m_ui;
		PathType m_pathType;
		QString m_dialogueCaption;
		QString m_dialogueFilter;
	};

}  // namespace Anansi

#endif  // ANANSI_FILESYSTEMPATHWIDGET_H
