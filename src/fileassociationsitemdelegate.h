/// \file fileassociationsitemdelegate.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the FileAssociationsItemDelegate class for EquitWebServer
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQUITWEBSERVER_FILEASSOCIATIONSITEMDELEGATE_H
#define EQUITWEBSERVER_FILEASSOCIATIONSITEMDELEGATE_H

#include <QStyledItemDelegate>

namespace EquitWebServer {

	class Configuration;
	class FileAssociationsWidget;

	class FileAssociationsItemDelegate : public QStyledItemDelegate {
	public:
		explicit FileAssociationsItemDelegate(FileAssociationsWidget * parent = nullptr);
		virtual ~FileAssociationsItemDelegate() override;

		virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
		virtual void setEditorData(QWidget * editor, const QModelIndex & index) const override;
		virtual void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;

	private:
		FileAssociationsWidget * m_parent;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_FILEASSOCIATIONSITEMDELEGATE_H
