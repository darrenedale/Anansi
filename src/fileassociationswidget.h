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

namespace Anansi {

	class Server;
	class ServerFileAssociationsModel;

	namespace Ui {
		class FileAssociationsWidget;
	}

	class FileAssociationsWidget : public QWidget {
		Q_OBJECT

	public:
		explicit FileAssociationsWidget(QWidget * = nullptr);
		explicit FileAssociationsWidget(Server *, QWidget * = nullptr);
		~FileAssociationsWidget();

		void setServer(Server *);

		bool hasExtension(const QString &) const;
		bool extensionHasMimeType(const QString &, const QString &) const;

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
		void currentExtensionMimeTypeChanged(const QString &, const QString &);

		void extensionAdded(const QString &);
		void extensionRemoved(const QString &);
		void extensionChanged(const QString &, const QString &);

		void extensionMimeTypeAdded(const QString &, const QString &);
		void extensionMimeTypeRemoved(const QString &, const QString &);
		void extensionMimeTypeChanged(const QString &, const QString &, const QString &);

	public Q_SLOTS:
		void addAvailableMimeType(const QString &);

		bool addExtension(const QString &);
		bool addExtensionMimeType(const QString &, const QString &);

		void removeExtension(const QString &);
		void removeExtensionMimeType(const QString &, const QString &);

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

		bool setCurrentExtension(const QString &);
		bool setCurrentExtensionMimeType(const QString &, const QString &);
		void setDefaultMimeType(const QString &);

	private Q_SLOTS:
		void onFileExtensionsSelectionChanged();

	private:
		std::unique_ptr<ServerFileAssociationsModel> m_model;
		std::unique_ptr<Ui::FileAssociationsWidget> m_ui;
		Server * m_server;  // observed only
	};

}  // namespace Anansi

#endif  // ANANSI_FILEASSOCIATIONSWIDGET_H
