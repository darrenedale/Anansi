/** \file bpIpListWidget.h
  * \author darren Hatherley
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the bpIpListWidget class for EquitWebServer
  *
  * \todo
  * - class documentation.
  * - decide on application license.
  *
  * \par Current Changes
  * - (2012-06-19) file documentation created.
  *
  */

#ifndef BPIPLISTWIDGET_H
#define BPIPLISTWIDGET_H

#include <QTreeWidget>
class QContextMenuEvent;


class bpIpListWidget
:	public QTreeWidget {

	Q_OBJECT

	public:
		bpIpListWidget( QWidget * parent = 0 );
		void setSelectionMode( SelectionMode mode );
		void insertTopLevelItem ( int index, QTreeWidgetItem * item );

	protected:
		void contextMenuEvent( QContextMenuEvent * event );
		
	protected slots:
		void removeIPAddress( int i );
		void removeSelectedIPAddress( void );
		
	signals:
		void ipAddressRemoved( QString );
};

#endif
