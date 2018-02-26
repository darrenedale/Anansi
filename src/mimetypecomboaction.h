#ifndef EQUITWEBSERVER_MIMECOMBOACTION_H
#define EQUITWEBSERVER_MIMECOMBOACTION_H

#include <QWidgetAction>

namespace EquitWebServer {

	class MimeTypeCombo;

	// TODO find a better name for this - its current name makes it sound like it could
	// be a combo box for MIME type actions
	class MimeTypeComboAction : public QWidgetAction {
		Q_OBJECT

	public:
		MimeTypeComboAction(QObject * parent = nullptr);
		virtual ~MimeTypeComboAction();

		MimeTypeCombo * mimeCombo() const {
			return m_combo;
		}

		void setMimeTypes(std::vector<QString> mimeTypes);
		void addMimeType(const QString & mimeType);

	Q_SIGNALS:
		void addMimeTypeClicked(const QString & mimeType);

	private:
		MimeTypeCombo * m_combo;
	};

}  // namespace EquitWebServer

#endif  // EQUITWEBSERVER_MIMECOMBOACTION_H
