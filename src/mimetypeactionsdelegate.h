/// \file mimetypeactionsdelegate.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the MimeTypeActionsDelegate class for EquitWebServer
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQUITWEBSERVER_MIMEACTIONSITEMDELEGATE_H
#define EQUITWEBSERVER_MIMEACTIONSITEMDELEGATE_H

#include <QStyledItemDelegate>

namespace EquitWebServer {

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

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_MIMEACTIONSITEMDELEGATE_H
