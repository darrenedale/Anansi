#ifndef BPMIMETYPEMANAGERWIDGET_H
#define BPMIMETYPEMANAGERWIDGET_H

#include "bpWebServerConfiguration.h"

#include <QWidget>
#include <QString>

class bpEditableTreeWidget;

class QWidget;
class QComboBox;


class bpMimeTypeManagerWidget
:	public QWidget
{
	Q_OBJECT

	private:
		bpEditableTreeWidget * m_mimeTree;
		QComboBox * m_defaultMIMECombo;
		QComboBox * m_defaultActionCombo;

	public:
		explicit bpMimeTypeManagerWidget( const bpWebServerConfiguration * config = 0, QWidget * parent = 0 );
		virtual ~bpMimeTypeManagerWidget( void );

	signals:
		void extensionAddedToMimeType( QString & mimeType, QString & extension );
		void extensionRemovedFromMimeType( QString & mimeType, QString & extension );
		void mimeTypeAdded( QString & mimeType );
		void mimeTypeRemoved( QString & mimeType );

		void mimeTypeActionChanged( QString & mimeType, bpWebServerConfiguration::WebServerAction action, QString cgiExe = QString() );
		void mimeTypeCgiExecutableChanged( QString & mimeType, QString & cgiExe );

		void defaultMimeTypeChanged( QString & mimeType );
		void defaultActionChanged( bpWebServerConfiguration::WebServerAction action );

	private slots:
		/* these are all slots to get connected to button clicked() signals, content (e.g. mime type) to come from
		 * current content of line exit/combo/tree widgets */
		void _addNewMimeType( void );
		bool _removeMimeType( void );
		bool _addExtensionToMimeType( void );
		bool _removeExtensionFromMimeType( void );
		bool _setMimeTypeCgiExecutable( void );
		bool _setMimeTypeAction( void );
		void _emitDefaultMimeTypeChanged( void );
		void _emitDefaultActionChanged( void );

	public slots:
		
};

#endif
