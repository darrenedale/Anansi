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
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_FILEASSOCIATIONSWIDGET_H
#define ANANSI_FILEASSOCIATIONSWIDGET_H

#include <memory>

#include <QWidget>

#include "configuration.h"

class QSortFilterProxyModel;

namespace Anansi {

	class Server;
	class ServerFileAssociationsModel;

	namespace Ui {
		class FileAssociationsWidget;
	}

	class FileAssociationsWidget : public QWidget {
		Q_OBJECT

	public:
		explicit FileAssociationsWidget(QWidget * parent = nullptr);
		explicit FileAssociationsWidget(Server * server, QWidget * parent = nullptr);
		~FileAssociationsWidget();

		void setServer(Server * server);

		bool hasExtension(const QString & ext) const;
		bool extensionHasMimeType(const QString & ext, const QString & mimeType) const;

		std::vector<QString> availableMimeTypes() const;

		QString defaultMimeType() const;

		QString currentExtension() const;
		QString selectedExtension() const;
		std::vector<QString> selectedExtensions() const;

		QString currentMimeType() const;
		QString selectedMimeType() const;
		std::vector<QString> selectedMimeTypes() const;

		void clear();

	Q_SIGNALS:
		void defaultMimeTypeChanged(const QString &);
		void currentExtensionChanged(const QString &);
		void currentExtensionMimeTypeChanged(const QString & ext, const QString & mimeType);

		void extensionAdded(const QString &);
		void extensionRemoved(const QString &);
		void extensionChanged(const QString & oldExt, const QString & newExt);

		void extensionMimeTypeAdded(const QString & ext, const QString & mimeType);
		void extensionMimeTypeRemoved(const QString & ext, const QString & mimeType);
		void extensionMimeTypeChanged(const QString & ext, const QString & oldMimeType, const QString & newMimeType);

	public Q_SLOTS:
		void addAvailableMimeType(const QString & mimeType);

		bool addExtension(const QString & ext);
		bool addExtensionMimeType(const QString & ext, const QString & mimeType);

		void removeExtension(const QString & ext);
		void removeExtensionMimeType(const QString & ext, const QString & mimeType);

		inline void removeCurrentExtension() {
			auto ext = currentExtension();

			if(ext.isEmpty()) {
				return;
			}

			removeExtension(ext);
		}

		inline void removeCurrentMimeType() {
			auto ext = currentExtension();

			if(ext.isEmpty()) {
				return;
			}

			auto mimeType = currentMimeType();

			if(mimeType.isEmpty()) {
				return;
			}

			removeExtensionMimeType(ext, mimeType);
		}

		bool setCurrentExtension(const QString & ext);
		bool setCurrentExtensionMimeType(const QString & ext, const QString & mimeType);
		void setDefaultMimeType(const QString & mimeType);

	private Q_SLOTS:
		void onFileExtensionsSelectionChanged();

	private:
		std::unique_ptr<ServerFileAssociationsModel> m_model;
		std::unique_ptr<Ui::FileAssociationsWidget> m_ui;
		Server * m_server;  // observed only
	};


}  // namespace Anansi
#endif  // ANANSI_FILEASSOCIATIONSWIDGET_H
