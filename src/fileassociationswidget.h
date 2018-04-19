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

/// \file fileassociationswidget.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the FileAssociationsWidget class for Anansi.
///
/// \dep
/// - <memory>
/// - <QWidget>
/// - <QString>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_FILEASSOCIATIONSWIDGET_H
#define ANANSI_FILEASSOCIATIONSWIDGET_H

#include <memory>

#include <QWidget>
#include <QString>

class QMenu;

namespace Anansi {

	class Server;
	class FileAssociationsModel;
	class FileAssociationsItemDelegate;

	namespace Ui {
		class FileAssociationsWidget;
	}

	class FileAssociationsWidget : public QWidget {
		Q_OBJECT

	public:
		explicit FileAssociationsWidget(QWidget * parent = nullptr);
		explicit FileAssociationsWidget(Server * server, QWidget * parent = nullptr);
		~FileAssociationsWidget() override;

		void setServer(Server * server);

		bool hasExtension(const QString & ext) const;
		bool extensionHasMediaType(const QString & ext, const QString & mediaType) const;

		std::vector<QString> availableMediaTypes() const;

		QString defaultMediaType() const;

		QString currentExtension() const;
		QString selectedExtension() const;
		std::vector<QString> selectedExtensions() const;

		QString currentMediaType() const;
		QString selectedMediaType() const;
		std::vector<QString> selectedMediaTypes() const;

		void clear();

	Q_SIGNALS:
		void defaultMediaTypeChanged(const QString & mediaType);
		void currentExtensionChanged(const QString & ext);
		void currentExtensionMediaTypeChanged(const QString & ext, const QString & mediaType);

		void extensionAdded(const QString & ext);
		void extensionRemoved(const QString & ext);
		void extensionChanged(const QString & oldExt, const QString & newExt);

		void extensionMediaTypeAdded(const QString & ext, const QString & mediaType);
		void extensionMediaTypeRemoved(const QString & ext, const QString & mediaType);
		void extensionMediaTypeChanged(const QString & ext, const QString & oldMediaType, const QString & newMediaType);

	public Q_SLOTS:
		void addAvailableMediaType(const QString & mediaType);

		bool addExtension(const QString & ext);
		bool addExtensionMediaType(const QString & ext, const QString & mediaType);

		void removeExtension(const QString & ext);
		void removeExtensionMediaType(const QString & ext, const QString & mediaType);

		inline void removeCurrentExtension() {
			auto ext = currentExtension();

			if(ext.isEmpty()) {
				return;
			}

			removeExtension(ext);
		}

		inline void removeCurrentMediaType() {
			auto ext = currentExtension();

			if(ext.isEmpty()) {
				return;
			}

			auto mediaType = currentMediaType();

			if(mediaType.isEmpty()) {
				return;
			}

			removeExtensionMediaType(ext, mediaType);
		}

		bool setCurrentExtension(const QString & ext);
		bool setCurrentExtensionMediaType(const QString & ext, const QString & mediaType);
		void setDefaultMediaType(const QString & mediaType);

	private Q_SLOTS:
		void onFileExtensionsSelectionChanged();

	private:
		std::unique_ptr<FileAssociationsModel> m_model;
		std::unique_ptr<FileAssociationsItemDelegate> m_delegate;
		std::unique_ptr<Ui::FileAssociationsWidget> m_ui;
		std::unique_ptr<QMenu> m_addEntryMenu;
		Server * m_server;  // observed only
	};

}  // namespace Anansi

#endif  // ANANSI_FILEASSOCIATIONSWIDGET_H
