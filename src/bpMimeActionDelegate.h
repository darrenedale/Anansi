#ifndef BPMIMEACTIONDELEGATE_H
#define BPMIMEACTIONDELEGATE_H

#include <QItemDelegate>
#include <QStyleOptionViewItem>
#include <QModelIndex>

class QObject;
class QWidget;
class QAbstractItemModel;


class bpMimeActionDelegate
:	public QItemDelegate {

	Q_OBJECT

	public:
		bpMimeActionDelegate( QObject * parent = 0 );

		QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;

		void setEditorData( QWidget * editor, const QModelIndex & index) const;
		void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const;

		void updateEditorGeometry( QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif
