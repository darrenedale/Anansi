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

/// \file mediatypeactionsdelegate.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the MediaTypeActionsDelegate class for Anansi.
///
/// \dep
/// - <QStyledItemDelegate>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_MEDIATYPEACTIONSDELEGATE_H
#define ANANSI_MEDIATYPEACTIONSDELEGATE_H

#include <QStyledItemDelegate>

class QWidget;
class QStyleOptionViewItem;
class QModelIndex;
class QAbstractItemModel;

namespace Anansi {

	class MediaTypeActionsWidget;

	class MediaTypeActionsDelegate final : public QStyledItemDelegate {
	public:
		explicit MediaTypeActionsDelegate(MediaTypeActionsWidget * parent = nullptr);

		QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & styleOption, const QModelIndex & idx) const override;
		void setEditorData(QWidget * parent, const QModelIndex & idx) const override;
		void setModelData(QWidget * parent, QAbstractItemModel * model, const QModelIndex & idx) const override;

	private:
		MediaTypeActionsWidget * m_parent;
	};

}  // namespace Anansi

#endif  // ANANSI_MEDIATYPEACTIONSDELEGATE_H
