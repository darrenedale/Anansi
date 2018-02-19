#include "fileassociationextensionitem.h"

#include "fileassociationswidget.h"

namespace EquitWebServer {


	FileAssociationExtensionItem::FileAssociationExtensionItem(const QString & ext)
	: QTreeWidgetItem(ItemType) {
		// tell the delegate what type of item this is
		setData(0, FileAssociationsWidget::DelegateItemTypeRole, ItemType);
		setData(0, FileAssociationsWidget::DelegateItemDataRole, QStringLiteral());
		setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
		setExtension(ext);
	}


	QString FileAssociationExtensionItem::previousExtension() const {
		return data(0, FileAssociationsWidget::DelegateItemOldDataRole).value<QString>();
	}


	QString FileAssociationExtensionItem::extension() const {
		return data(0, FileAssociationsWidget::DelegateItemDataRole).value<QString>();
	}


	void FileAssociationExtensionItem::setExtension(const QString & ext) {
		setData(0, FileAssociationsWidget::DelegateItemOldDataRole, extension());
		setData(0, Qt::EditRole, ext);
		setData(0, Qt::DisplayRole, ext);
		setData(0, FileAssociationsWidget::DelegateItemDataRole, ext);
		refresh();
	}


	void FileAssociationExtensionItem::refresh() {
		setText(0, extension());
	}


}  // namespace EquitWebServer
