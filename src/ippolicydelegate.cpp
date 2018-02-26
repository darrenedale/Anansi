#include "ippolicydelegate.h"

#include "types.h"
#include "serveripconnectionpolicymodel.h"
#include "accesscontrolwidget.h"
#include "connectionpolicycombo.h"


Q_DECLARE_METATYPE(EquitWebServer::ConnectionPolicy)


namespace EquitWebServer {


	IpPolicyDelegate::IpPolicyDelegate(AccessControlWidget * parent)
	: QStyledItemDelegate(parent),
	  m_parent(parent) {
	}

	QWidget * IpPolicyDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem &, const QModelIndex & index) const {
		if(!index.isValid()) {
			return nullptr;
		}

		switch(index.column()) {
			case ServerIpConnectionPolicyModel::IpAddressColumnIndex:
				return nullptr;

			case ServerIpConnectionPolicyModel::PolicyColumnIndex:
				return new ConnectionPolicyCombo(parent);
		}

		return nullptr;
	}


	void IpPolicyDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const {
		if(!index.isValid()) {
			return;
		}

		if(index.column() == ServerIpConnectionPolicyModel::PolicyColumnIndex) {
			auto * combo = qobject_cast<ConnectionPolicyCombo *>(editor);
			Q_ASSERT_X(combo, __PRETTY_FUNCTION__, "expected editor to be a WebServerActionCombo");
			combo->setConnectionPolicy(index.data(Qt::EditRole).value<ConnectionPolicy>());
			return;
		}

		QStyledItemDelegate::setEditorData(editor, index);
	}


	void IpPolicyDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const {
		if(!index.isValid()) {
			return;
		}

		if(index.column() == ServerIpConnectionPolicyModel::PolicyColumnIndex) {
			auto * combo = qobject_cast<ConnectionPolicyCombo *>(editor);
			Q_ASSERT_X(combo, __PRETTY_FUNCTION__, "expected editor to be a WebServerActionCombo");
			model->setData(index, QVariant::fromValue(combo->connectionPolicy()));
		}
	}


	IpPolicyDelegate::~IpPolicyDelegate() = default;


}  // namespace EquitWebServer
