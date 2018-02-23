#ifndef EQUITWEBSERVER_MIMETYPECOMBO_H
#define EQUITWEBSERVER_MIMETYPECOMBO_H

#include <QComboBox>

#include "configuration.h"

namespace EquitWebServer {

	class MimeTypeCombo : public QComboBox {
		Q_OBJECT

	public:
		explicit MimeTypeCombo(QWidget * parent = nullptr);
		explicit MimeTypeCombo(bool allowCustom, QWidget * parent = nullptr);
		virtual ~MimeTypeCombo() = default;

		void insertItem() = delete;
		void addItem() = delete;
		void removeItem() = delete;

		std::vector<QString> availableMimeTypes() const;
		QString currentMimeType() const;

		inline bool customMimeTypesAllowed() const {
			return isEditable();
		}

		bool hasMimeType(const QString & mime) const;

	public Q_SLOTS:
		void setCustomMimeTypesAllowed(bool allowed) {
			setEditable(allowed);
		}

		bool addMimeType(const QString & mime);
		void removeMimeType(const QString & mime);
		void setCurrentMimeType(const QString & mime);

	Q_SIGNALS:
		void mimeTypeAdded(const QString &);
		void mimeTypeRemoved(const QString &);
		void currentMimeTypeChanged(const QString &);
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_MIMETYPECOMBO_H
