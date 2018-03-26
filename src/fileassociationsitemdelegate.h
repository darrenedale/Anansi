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

/// \file fileassociationsitemdelegate.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the FileAssociationsItemDelegate class for Anansi.
///
/// \dep
/// - <QStyledItemDelegate>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_FILEASSOCIATIONSITEMDELEGATE_H
#define ANANSI_FILEASSOCIATIONSITEMDELEGATE_H

#include <QStyledItemDelegate>

class QStyleOptionViewItem;
class QModelIndex;
class QWidget;
class QAbstractItemModel;

namespace Anansi {

	class Configuration;
	class FileAssociationsWidget;

	class FileAssociationsItemDelegate : public QStyledItemDelegate {
	public:
		explicit FileAssociationsItemDelegate(QObject * parent = nullptr);

		void addMediaType(const QString & mediaType);
		void removeMediaType(const QString & mediaType);

		inline std::vector<QString> mediaTypes() const {
			return m_mediaTypes;
		}

		virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & idx) const override;
		virtual void setEditorData(QWidget * editor, const QModelIndex & idx) const override;
		virtual void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & idx) const override;

	private:
		std::vector<QString> m_mediaTypes;
	};

}  // namespace Anansi

#endif  // ANANSI_FILEASSOCIATIONSITEMDELEGATE_H
