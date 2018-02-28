/// \file mimecombowidgetaction.h
/// \author Darren Edale
/// \version 0.9.9
/// \date February, 2018
///
/// \brief Declaration of the Configuration class for EquitWebServer
///
/// \par Changes
/// - (2018-02) First release.

#ifndef EQUITWEBSERVER_MIMECOMBOWIDGETACTION_H
#define EQUITWEBSERVER_MIMECOMBOWIDGETACTION_H

#include <QWidgetAction>

namespace EquitWebServer {

	class MimeCombo;

	class MimeComboWidgetAction : public QWidgetAction {
		Q_OBJECT

	public:
		MimeComboWidgetAction(QObject * parent = nullptr);
		virtual ~MimeComboWidgetAction();

		MimeCombo * mimeCombo() const {
			return m_combo;
		}

		void setMimeTypes(std::vector<QString> mimeTypes);
		void addMimeType(const QString & mimeType);

	Q_SIGNALS:
		void addMimeTypeClicked(const QString & mimeType);

	private:
		MimeCombo * m_combo;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_MIMECOMBOWIDGETACTION_H
