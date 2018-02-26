#ifndef EQUITWEBSERVER_IPPOLICYDELEGATE_H
#define EQUITWEBSERVER_IPPOLICYDELEGATE_H

#include <QStyledItemDelegate>

namespace EquitWebServer {

	class Configuration;
	class AccessControlWidget;

	class IpPolicyDelegate : public QStyledItemDelegate {
	public:
		explicit IpPolicyDelegate(AccessControlWidget * parent = nullptr);
		virtual ~IpPolicyDelegate() override;

		virtual QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
		virtual void setEditorData(QWidget * editor, const QModelIndex & index) const override;
		virtual void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const override;

	private:
		AccessControlWidget * m_parent;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_IPPOLICYDELEGATE_H
