#include "fileassociationextensionitem.h"

#include "fileassociationswidget.h"

namespace EquitWebServer {


	FileAssociationExtensionItem::FileAssociationExtensionItem(const QString & ext)
	: QTreeWidgetItem(ItemType) {
		// tell the delegate what type of item this is
		setData(0, FileAssociationsWidget::DelegateItemTypeRole, ItemType);
		setData(0, FileAssociationsWidget::DelegateItemDataRole, QStringLiteral());
		setExtension(ext);
	}


	QString FileAssociationExtensionItem::previousExtension() const {
		return data(0, FileAssociationsWidget::DelegateItemOldDataRole).toString();
	}


	QString FileAssociationExtensionItem::extension() const {
		return data(0, FileAssociationsWidget::DelegateItemDataRole).toString();
	}


	void FileAssociationExtensionItem::setExtension(const QString & ext) {
		setData(0, FileAssociationsWidget::DelegateItemDataRole, ext);
		refresh();
	}


	void FileAssociationExtensionItem::refresh() {
		setText(0, extension());
	}


}  // namespace EquitWebServer
