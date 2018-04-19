/*
 * Copyright 2015 - 2018 Darren Edale
 *
 * This file is part of the Equit library.
 *
 * The Equit library is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * The Equit library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * the Equit library. If not, see <http://www.gnu.org/licenses/>.
 */

/// \file counterlabel.h
/// \author Darren Edale
/// \version 1.0.0
/// \date March 2018
///
/// \brief Declaration of the CounterLabel class for the Equit library.
///
/// \dep
/// - <QLabel>
/// - <QString>
///
/// \par Changes
/// - (2018-03) First release.

#ifndef EQ_COUNTERLABEL_H
#define EQ_COUNTERLABEL_H

#include <QLabel>
#include <QString>

class QContextMenuEvent;

namespace Equit {

	class CounterLabel : public QLabel {
		Q_OBJECT

	public:
		explicit CounterLabel(const QString & tmplate = QStringLiteral("%1"), int count = 0, QWidget * parent = nullptr);
		explicit CounterLabel(QWidget * parent);

		inline const QString & displayTemplate() const {
			return m_template;
		}

		inline int count() const {
			return m_count;
		}

	public Q_SLOTS:
		void setDisplayTemplate(const QString & tplate);
		void setCount(int count);

		inline void reset() {
			setCount(0);
		}

		inline void increment() {
			add(1);
		}

		inline void add(int amount) {
			setCount(count() + amount);
		}

		inline void decrement() {
			subtract(1);
		}

		inline void subtract(int amount) {
			setCount(count() - amount);
		}

	protected:
		void refresh();
		void contextMenuEvent(QContextMenuEvent * ev) override;

	private:
		QString m_template;
		int m_count;
	};

}  // namespace Equit

#endif  // EQ_COUNTERLABEL_H
