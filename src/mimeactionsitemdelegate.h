#ifndef EQUITWEBSERVER_MIMEACTIONSITEMDELEGATE_H
#define EQUITWEBSERVER_MIMEACTIONSITEMDELEGATE_H

#include <QStyledItemDelegate>

namespace EquitWebServer {

	class Configuration;
	class MimeActionsWidget;

	class MimeActionsItemDelegate : public QStyledItemDelegate {
	public:
		explicit MimeActionsItemDelegate(MimeActionsWidget * parent = nullptr);
		virtual ~MimeActionsItemDelegate() override;

		virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
		virtual void setEditorData(QWidget * editor, const QModelIndex & index) const override;
		virtual void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;

	private:
		MimeActionsWidget * m_parent;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_MIMEACTIONSITEMDELEGATE_H
