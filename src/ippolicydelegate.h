/// \file ippolicydelegate.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the IpPolicyDelegate class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_IPPOLICYDELEGATE_H
#define ANANSI_IPPOLICYDELEGATE_H

#include <QStyledItemDelegate>

namespace Anansi {

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

}  // namespace Anansi

#endif  // ANANSI_IPPOLICYDELEGATE_H
