#ifndef ANANSI_ICONPANEL_H
#define ANANSI_ICONPANEL_H

#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>
#include <QString>

class QShowEvent;

namespace Anansi {

	class SelectorPanel : public QListWidget {
	public:
		SelectorPanel(QWidget * parent = nullptr);

		void addItem(QListWidgetItem * item);

		inline void addItem(const QIcon & icon, const QString & label) {
			addItem(new QListWidgetItem(icon, label));
		}

		void insertItem(int row, QListWidgetItem * item);

		inline void insertItem(int row, const QIcon & icon, const QString & label) {
			insertItem(row, new QListWidgetItem(icon, label));
		}

		void addItems() = delete;
		void insertItems() = delete;

	protected:
		virtual void showEvent(QShowEvent *) override;

	private:
		void recalculateSize();
	};

}  // namespace Anansi

#endif  // ANANSI_ICONPANEL_H
