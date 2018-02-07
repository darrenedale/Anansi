#include "bpMimeActionDelegate.h"

#include "bpWebServerConfiguration.h"

#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QComboBox>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QDebug>


bpMimeActionDelegate::bpMimeActionDelegate( QObject * parent )
:	QItemDelegate(parent)
{}


QWidget * bpMimeActionDelegate::createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	QComboBox * editor = new QComboBox(parent);
	editor->addItem(tr("Ignore"), bpWebServerConfiguration::Ignore);
	editor->addItem(tr("Serve"), bpWebServerConfiguration::Serve);
	editor->addItem(tr("CGI"), bpWebServerConfiguration::CGI);
	editor->addItem(tr("Forbid"), bpWebServerConfiguration::Forbid);

	return editor;
}


void bpMimeActionDelegate::setEditorData( QWidget * editor, const QModelIndex & index ) const
{
	bpWebServerConfiguration::WebServerAction data = bpWebServerConfiguration::WebServerAction(index.model()->data(index).toInt());
	QComboBox * combo = static_cast<QComboBox *>(editor);
	combo->setCurrentIndex(combo->findData(data));
}


void bpMimeActionDelegate::setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const {
	QComboBox * combo = qobject_cast<QComboBox *>(editor);
//	QTreeWidget * tree = qobject_cast<QTreeWidget *>(model);
//	QTreeWidgetItem * it = tree->item(index);
	bpWebServerConfiguration::WebServerAction action = bpWebServerConfiguration::WebServerAction(combo->itemData(combo->currentIndex()).toInt());
//	it->setData(0, Qt::UserRole, QVariant(int(action)));
	
	switch(action) {
		case bpWebServerConfiguration::Ignore:
			model->setData(index, tr("Ignore"), Qt::DisplayRole);
//			it->setText(index.column(), tr("Ignore"));
			break;
		
		case bpWebServerConfiguration::Serve:
			model->setData(index, tr("Serve"), Qt::DisplayRole);
//			it->setText(index.column(), tr("Serve"));
			break;
		
		case bpWebServerConfiguration::CGI:
			model->setData(index, tr("CGI"), Qt::DisplayRole);
//			it->setText(index.column(), tr("CGI"));
			break;
		
		case bpWebServerConfiguration::Forbid:
			model->setData(index, tr("Forbid"), Qt::DisplayRole);
//			it->setText(index.column(), tr("Forbid"));
			break;
	}
}


void bpMimeActionDelegate::updateEditorGeometry( QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
	editor->setGeometry(option.rect);
}
