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
		explicit FileAssociationsItemDelegate(FileAssociationsWidget * = nullptr);

		virtual QWidget * createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const override;
		virtual void setEditorData(QWidget *, const QModelIndex &) const override;
		virtual void setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const override;

	private:
		FileAssociationsWidget * m_parent;
	};

}  // namespace Anansi

#endif  // ANANSI_FILEASSOCIATIONSITEMDELEGATE_H
