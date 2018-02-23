#ifndef EQUITWEBSERVER_FILENAMEWIDGET_H
#define EQUITWEBSERVER_FILENAMEWIDGET_H

#include <memory>

#include <QWidget>

namespace EquitWebServer {

	namespace Ui {
		class FileNameWidget;
	}

	class FileNameWidget : public QWidget {
		Q_OBJECT

	public:
		explicit FileNameWidget(const QString & path, QWidget * parent = nullptr);
		explicit FileNameWidget(QWidget * parent = nullptr);
		~FileNameWidget();

		QString placeholderText() const;
		void setPlaceholderText(const QString &);

		inline void setDialogueCaption(const QString & caption) {
			m_dialogueCaption = caption;
		}

		inline const QString & dialogueCaption() const {
			return m_dialogueCaption;
		}

		void setFileName(const QString & path);
		QString fileName() const;

		// TODO set filter for dialogue

	Q_SIGNALS:
		void fileNameChanged(const QString & path);

	public Q_SLOTS:
		void chooseFile(QString path = {});

	private:
		std::unique_ptr<Ui::FileNameWidget> m_ui;
		QString m_dialogueCaption;
	};


}  // namespace EquitWebServer
#endif  // EQUITWEBSERVER_FILENAMEWIDGET_H
