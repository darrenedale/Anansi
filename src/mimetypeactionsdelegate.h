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

/// \file mimetypeactionsdelegate.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the MimeTypeActionsDelegate class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_MIMEACTIONSITEMDELEGATE_H
#define ANANSI_MIMEACTIONSITEMDELEGATE_H

#include <QStyledItemDelegate>

namespace Anansi {

	class Configuration;
	class MimeActionsWidget;

	class MimeTypeActionsDelegate : public QStyledItemDelegate {
	public:
		explicit MimeTypeActionsDelegate(MimeActionsWidget * parent = nullptr);
		virtual ~MimeTypeActionsDelegate() override;

		virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
		virtual void setEditorData(QWidget * editor, const QModelIndex & index) const override;
		virtual void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;

	private:
		MimeActionsWidget * m_parent;
	};

}  // namespace Anansi

#endif  // ANANSI_MIMEACTIONSITEMDELEGATE_H
