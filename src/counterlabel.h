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

	class CounterLabel : public QLabel {
		Q_OBJECT

	public:
		explicit CounterLabel(const QString & tplate = QStringLiteral("%1"), int count = 0, QWidget * parent = nullptr);

		inline const QString & getTemplate() const {
			return m_template;
		}

		inline int count() const {
			return m_count;
		}

	public Q_SLOTS:
		void setTemplate(const QString & tplate);
		void setCount(int c);

		inline void reset() {
			setCount(0);
		}

		inline void increment() {
			increment(1);
		}

		inline void increment(int amount) {
			setCount(count() + amount);
		}

		inline void decrement() {
			decrement(1);
		}

		inline void decrement(int amount) {
			setCount(count() - amount);
		}

	protected:
		void refresh();
		virtual void contextMenuEvent(QContextMenuEvent * ev);

	private:
		QString m_template;
		int m_count;
	};

}  // namespace EquitWebServer

#endif  // CONNECTIONCOUNTLABEL_H
