/// \file mimecombowidgetaction.h
/// \author Darren Edale
/// \version 0.9.9
/// \date March 2018
///
/// \brief Declaration of the Configuration class for Anansi.
///
/// \par Changes
/// - (2018-03) First release.

#ifndef ANANSI_MIMECOMBOWIDGETACTION_H
#define ANANSI_MIMECOMBOWIDGETACTION_H

#include <QWidgetAction>

namespace Anansi {

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

}  // namespace Anansi

#endif  // ANANSI_MIMECOMBOWIDGETACTION_H
