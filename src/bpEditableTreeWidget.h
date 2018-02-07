/** \file bpEditableTreeWidget.h
  * \author darren Hatherley
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the bpEditableTreeWidget class for EquitWebServer
  *
  * \todo
  * - class documentation.
  * - decide on application license.
  *
  * \par Current Changes
  * - (2012-06-19) file documentation created.
  *
  */

#ifndef BPEDITABLETREEWIDGET_H
#define BPEDITABLETREEWIDGET_H

#include <QTreeWidget>
#include <QString>
#include <QIcon>
#include <QList>
#include <QModelIndexList>
#include <QKeySequence>

class QObject;
class QTreeWidgetItem;


class bpEditableTreeWidget
:	public QTreeWidget {

	Q_OBJECT

	private:
		QAction * m_removeAction;

	public:
		bpEditableTreeWidget( QWidget * parent = 0 );

		QAction * addAction( const QIcon & icon, const QString & text, const QObject * receiver, const char * slot, const QKeySequence & shortcut = 0 );
		QAction * addAction( const QIcon & icon, const QString & text );
		QAction * addAction( const QString & text, const QObject * receiver, const char * slot, const QKeySequence & shortcut = 0);
		QAction * addAction( const QString & text );
		void addAction( QAction * action );

	protected:
		void contextMenuEvent( QContextMenuEvent * event );

	public slots:
		void removeItems( const QModelIndexList & items );
	
	protected slots:
		void emitRemoveRequested( void );

	signals:
		void removeRequested( QList<QTreeWidgetItem *> );
		void removeRequested( void );
		void removingItem( QTreeWidgetItem * );
};

#endif
