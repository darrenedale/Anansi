/** \file ConnectionCountLabel.h
  * \author Darren Edale
  * \version 0.9.9
  * \date 19th June, 2012
  *
  * \brief Definition of the ConnectionCountLabel class for EquitWebServer
  *
  * \todo
  * - class documentation.
  * - decide on application license.
  *
  * \par Changes
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
		explicit ConnectionCountLabel(const QString & tplate, int c = 0, QWidget * parent = 0);

		QString getTemplate(void) const;
		int count(void) const;

	public slots:
		void setTemplate(const QString & tplate);
		void reset(void);
		void setCount(int c);

		inline void increment() {
			increment(1);
		}

		void increment(int amount);

		inline void decrement() {
			decrement(1);
		}

		void decrement(int amount);

	protected slots:
		void refresh(void);

	protected:
		virtual void contextMenuEvent(QContextMenuEvent * ev);

	private:
		QString m_template;
		int m_count;
	};
}  // namespace EquitWebServer

#endif  // CONNECTIONCOUNTLABEL_H
