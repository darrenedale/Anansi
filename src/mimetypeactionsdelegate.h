#ifndef EQUITWEBSERVER_MIMEACTIONSITEMDELEGATE_H
#define EQUITWEBSERVER_MIMEACTIONSITEMDELEGATE_H

#include <QStyledItemDelegate>

namespace EquitWebServer {

	class Configuration;
	class MimeTypeActionsWidget;

	class MimeTypeActionsDelegate : public QStyledItemDelegate {
	public:
		explicit MimeTypeActionsDelegate(MimeTypeActionsWidget * parent = nullptr);
		virtual ~MimeTypeActionsDelegate() override;

		virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
		virtual void setEditorData(QWidget * editor, const QModelIndex & index) const override;
		virtual void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;

	private:
		MimeTypeActionsWidget * m_parent;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_MIMEACTIONSITEMDELEGATE_H
