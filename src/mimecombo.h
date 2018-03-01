/// \file mimecombo.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Declaration of the MimeCombo class for EquitWebServer
///
/// \par Changes
/// - (2018-02) First release.

#ifndef EQUITWEBSERVER_MIMECOMBO_H
#define EQUITWEBSERVER_MIMECOMBO_H

#include <vector>

#include <QString>
#include <QComboBox>

#include "configuration.h"

namespace EquitWebServer {

	class MimeCombo : public QComboBox {
		Q_OBJECT

	public:
		explicit MimeCombo(QWidget * parent = nullptr);
		explicit MimeCombo(bool allowCustom, QWidget * parent = nullptr);
		virtual ~MimeCombo() = default;

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

#endif  // EQUITWEBSERVER_MIMECOMBO_H