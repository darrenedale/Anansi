#include "fileassociationmimetypeitem.h"

#include "mimeicons.h"
#include "fileassociationswidget.h"


namespace EquitWebServer {


	FileAssociationMimeTypeItem::FileAssociationMimeTypeItem(const QString & mimeType)
	: QTreeWidgetItem(ItemType) {
		// tell the delegate what type of item this is
		setData(0, FileAssociationsWidget::DelegateItemTypeRole, ItemType);
		setData(0, FileAssociationsWidget::DelegateItemDataRole, QStringLiteral());
		setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);
		setMimeType(mimeType);
	}


	QString FileAssociationMimeTypeItem::previousMimeType() const {
		return data(0, FileAssociationsWidget::DelegateItemOldDataRole).toString();
	}


	QString FileAssociationMimeTypeItem::mimeType() const {
		return data(0, FileAssociationsWidget::DelegateItemDataRole).toString();
	}


	void FileAssociationMimeTypeItem::setMimeType(const QString & mime) {
		setData(0, FileAssociationsWidget::DelegateItemOldDataRole, mimeType());
		setData(0, FileAssociationsWidget::DelegateItemDataRole, mime);
		refresh();
	}


	void FileAssociationMimeTypeItem::refresh() {
		setText(0, mimeType());
		setIcon(0, mimeIcon(mimeType()));
	}


}  // namespace EquitWebServer
