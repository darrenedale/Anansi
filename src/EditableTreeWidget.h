/** \file bpEditableTreeWidget.h
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the bpEditableTreeWidget class for EquitWebServer
  *
  * \todo
  * - class documentation.
  * - decide on application license.
  *
  * \par Changes
  * - (2012-06-19) file documentation created.
  *
  */

#ifndef EDITABLETREEWIDGET_H
#define EDITABLETREEWIDGET_H

#include <QTreeWidget>
#include <QString>
#include <QIcon>
#include <QList>
#include <QModelIndexList>
#include <QKeySequence>

class QObject;
class QTreeWidgetItem;

namespace EquitWebServer {
	class EditableTreeWidget : public QTreeWidget {
		Q_OBJECT

	public:
		EditableTreeWidget(QWidget * parent = nullptr);

	protected:
		void contextMenuEvent(QContextMenuEvent * event);

	public Q_SLOTS:
		void removeItems(const QModelIndexList & items);

	Q_SIGNALS:
		void removeRequested(QList<QTreeWidgetItem *>);
		void removeRequested();
		void removingItem(QTreeWidgetItem *);

	private:
		QAction * m_removeAction;
	};
}  // namespace EquitWebServer

#endif
