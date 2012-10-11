/** \file ConnectionCountLabel.h
  * \author darren Hatherley
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the ConnectionCountLabel class for EquitWebServer
  *
  * \todo
  * - class documentation.
  * - decide on application license.
  *
  * \par Current Changes
  * - (2012-06-19) file documentation created.
  *
  */

#ifndef CONNECTIONCOUNTLABEL_H
#define CONNECTIONCOUNTLABEL_H

#include <QLabel>
#include <QString>

class QContextMenuEvent;


namespace EquitWebServer {
	class ConnectionCountLabel
	: public QLabel {

		Q_OBJECT

		public:
			explicit ConnectionCountLabel( const QString & tplate, int c = 0, QWidget * parent = 0 );

			QString getTemplate( void ) const;
			int count( void ) const;

		public slots:
			void setTemplate( const QString & tplate );
			void reset( void );
			void setCount( int c );
			void increment( int d = 1 );
			void decrement( int d = 1 );

		protected slots:
			void refresh( void );

		protected:
			virtual void contextMenuEvent( QContextMenuEvent * ev );

		private:
			QString m_template;
			int m_count;
	};
}	/* EquitWebServer namespace */

#endif // CONNECTIONCOUNTLABEL_H
